#include "StockfishEngine.h"

// Both guarded: the MinGW standard headers already define NOMINMAX, and
// redefining it is a warning.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <vector>

namespace {

// Starting up means loading a neural network of a hundred megabytes or so, so
// the first handshake is allowed far longer than any later command.
constexpr int HANDSHAKE_TIMEOUT_MS = 20000;
constexpr int READY_TIMEOUT_MS = 10000;

// However long the engine was told to think, it may take a little longer to
// answer. This margin is added to the wait so a slow reply is not mistaken for
// a dead engine.
constexpr int REPLY_MARGIN_MS = 5000;

constexpr int PIPE_POLL_INTERVAL_MS = 2;

std::filesystem::path executableDirectory() {
    std::vector<wchar_t> buffer(MAX_PATH);

    DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                      static_cast<DWORD>(buffer.size()));

    // A path longer than the buffer fills it completely rather than failing,
    // so grow and ask again until it fits.
    while (length == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        length = GetModuleFileNameW(nullptr, buffer.data(),
                                    static_cast<DWORD>(buffer.size()));
    }

    if (length == 0) {
        return {};
    }

    return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
}

// Hard rule: no absolute path may appear in the source, because the project is
// marked on another machine. The engine is found relative to the running
// executable first, then relative to the working directory.
std::filesystem::path findStockfishExecutable() {
    const std::filesystem::path relativePath =
        std::filesystem::path("stockfish") / "stockfish.exe";

    const std::filesystem::path exeDirectory = executableDirectory();

    const std::vector<std::filesystem::path> candidates = {
        exeDirectory / relativePath,
        relativePath,
        std::filesystem::path("..") / relativePath,
        exeDirectory / ".." / relativePath
    };

    for (const std::filesystem::path& candidate : candidates) {
        std::error_code ignoredError;
        if (std::filesystem::exists(candidate, ignoredError)) {
            return candidate;
        }
    }

    return {};
}

}   // namespace


StockfishEngine::StockfishEngine() {
    if (!startEngineProcess()) {
        return;
    }

    if (!completeUciHandshake()) {
        shutDown();
        return;
    }

    isRunning_ = true;
}


StockfishEngine::~StockfishEngine() {
    shutDown();
}


bool StockfishEngine::startEngineProcess() {
    const std::filesystem::path enginePath = findStockfishExecutable();

    if (enginePath.empty()) {
        lastError_ = "Stockfish was not found. It should be at "
                     "stockfish\\stockfish.exe next to the client.";
        return false;
    }

    SECURITY_ATTRIBUTES inheritable{};
    inheritable.nLength = sizeof(inheritable);
    inheritable.bInheritHandle = TRUE;
    inheritable.lpSecurityDescriptor = nullptr;

    HANDLE engineReadsFrom = nullptr;
    HANDLE weWriteTo = nullptr;
    HANDLE engineWritesTo = nullptr;
    HANDLE weReadFrom = nullptr;

    if (!CreatePipe(&engineReadsFrom, &weWriteTo, &inheritable, 0)) {
        lastError_ = "Could not create a pipe to send commands to Stockfish.";
        return false;
    }

    if (!CreatePipe(&weReadFrom, &engineWritesTo, &inheritable, 0)) {
        CloseHandle(engineReadsFrom);
        CloseHandle(weWriteTo);
        lastError_ = "Could not create a pipe to read Stockfish's replies.";
        return false;
    }

    // Our own ends of the pipes must not be inherited by the child. If they
    // were, the child would hold its own copy of them open, and closing ours
    // would never produce the end-of-file that tells the engine to stop.
    SetHandleInformation(weWriteTo, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(weReadFrom, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = engineReadsFrom;
    startupInfo.hStdOutput = engineWritesTo;
    startupInfo.hStdError = engineWritesTo;

    PROCESS_INFORMATION processInfo{};

    std::wstring commandLine = L"\"" + enginePath.wstring() + L"\"";

    const BOOL started = CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr,
                                        TRUE, CREATE_NO_WINDOW, nullptr, nullptr,
                                        &startupInfo, &processInfo);

    // The child has its own copies now; ours would otherwise keep the pipes
    // alive after the engine exits.
    CloseHandle(engineReadsFrom);
    CloseHandle(engineWritesTo);

    if (!started) {
        CloseHandle(weWriteTo);
        CloseHandle(weReadFrom);
        lastError_ = "Stockfish was found but could not be started.";
        return false;
    }

    processHandle_ = processInfo.hProcess;
    threadHandle_ = processInfo.hThread;
    engineInput_ = weWriteTo;
    engineOutput_ = weReadFrom;

    return true;
}


bool StockfishEngine::sendCommand(const std::string& command) {
    if (engineInput_ == nullptr) {
        return false;
    }

    const std::string line = command + "\n";

    DWORD written = 0;
    const BOOL ok = WriteFile(static_cast<HANDLE>(engineInput_), line.c_str(),
                              static_cast<DWORD>(line.size()), &written, nullptr);

    return ok && written == line.size();
}


bool StockfishEngine::readLine(std::string& line, int timeoutMs) {
    if (engineOutput_ == nullptr) {
        return false;
    }

    // GetTickCount64 rather than GetTickCount: the 32-bit one wraps back to
    // zero after about 49 days of uptime, and a deadline computed across that
    // wrap either expires at once or never.
    const ULONGLONG deadline = GetTickCount64() + static_cast<ULONGLONG>(timeoutMs);

    while (true) {
        const std::size_t newlineAt = pendingOutput_.find('\n');
        if (newlineAt != std::string::npos) {
            line = pendingOutput_.substr(0, newlineAt);
            pendingOutput_.erase(0, newlineAt + 1);

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            return true;
        }

        if (GetTickCount64() >= deadline) {
            return false;
        }

        // Peeking before reading is what keeps this from blocking forever. A
        // plain ReadFile on a pipe waits until something arrives, so an engine
        // that never answered would freeze the whole window with it.
        DWORD availableBytes = 0;
        if (!PeekNamedPipe(static_cast<HANDLE>(engineOutput_), nullptr, 0, nullptr,
                           &availableBytes, nullptr)) {
            return false;   // the engine has gone away
        }

        if (availableBytes == 0) {
            Sleep(PIPE_POLL_INTERVAL_MS);
            continue;
        }

        char buffer[4096];
        const DWORD wanted =
            std::min<DWORD>(availableBytes, static_cast<DWORD>(sizeof(buffer)));

        DWORD readCount = 0;
        if (!ReadFile(static_cast<HANDLE>(engineOutput_), buffer, wanted, &readCount,
                      nullptr)
            || readCount == 0) {
            return false;
        }

        pendingOutput_.append(buffer, readCount);
    }
}


bool StockfishEngine::waitForLineStartingWith(const std::string& prefix, std::string& line,
                                              int timeoutMs) {
    const ULONGLONG deadline = GetTickCount64() + static_cast<ULONGLONG>(timeoutMs);

    while (GetTickCount64() < deadline) {
        const int remaining = static_cast<int>(deadline - GetTickCount64());

        std::string candidate;
        if (!readLine(candidate, remaining)) {
            return false;
        }

        if (candidate.rfind(prefix, 0) == 0) {
            line = candidate;
            return true;
        }
    }

    return false;
}


bool StockfishEngine::completeUciHandshake() {
    if (!sendCommand("uci")) {
        lastError_ = "Stockfish started but would not accept commands.";
        return false;
    }

    std::string reply;
    if (!waitForLineStartingWith("uciok", reply, HANDSHAKE_TIMEOUT_MS)) {
        lastError_ = "Stockfish did not answer the UCI handshake.";
        return false;
    }

    if (!sendCommand("isready")
        || !waitForLineStartingWith("readyok", reply, READY_TIMEOUT_MS)) {
        lastError_ = "Stockfish did not report itself ready.";
        return false;
    }

    return true;
}


bool StockfishEngine::isRunning() const {
    return isRunning_;
}


std::string StockfishEngine::lastError() const {
    return lastError_;
}


void StockfishEngine::applyDifficulty(int skillLevel, int thinkingTimeMs) {
    thinkingTimeMs_ = thinkingTimeMs;

    if (!isRunning_) {
        return;
    }

    sendCommand("setoption name Skill Level value " + std::to_string(skillLevel));

    std::string reply;
    sendCommand("isready");
    waitForLineStartingWith("readyok", reply, READY_TIMEOUT_MS);
}


void StockfishEngine::startNewGame() {
    if (!isRunning_) {
        return;
    }

    sendCommand("ucinewgame");

    std::string reply;
    sendCommand("isready");
    waitForLineStartingWith("readyok", reply, READY_TIMEOUT_MS);
}


std::string StockfishEngine::chooseMove(const std::string& positionFen) {
    if (!isRunning_) {
        return "";
    }

    if (!sendCommand("position fen " + positionFen)) {
        isRunning_ = false;
        lastError_ = "Lost contact with Stockfish.";
        return "";
    }

    if (!sendCommand("go movetime " + std::to_string(thinkingTimeMs_))) {
        isRunning_ = false;
        lastError_ = "Lost contact with Stockfish.";
        return "";
    }

    std::string bestMoveLine;
    if (!waitForLineStartingWith("bestmove", bestMoveLine,
                                 thinkingTimeMs_ + REPLY_MARGIN_MS)) {
        isRunning_ = false;
        lastError_ = "Stockfish stopped answering.";
        return "";
    }

    // The reply reads "bestmove e2e4", sometimes with " ponder e7e5" after it.
    const std::size_t moveStart = bestMoveLine.find(' ');
    if (moveStart == std::string::npos) {
        return "";
    }

    const std::size_t moveEnd = bestMoveLine.find(' ', moveStart + 1);
    const std::size_t moveLength =
        (moveEnd == std::string::npos) ? std::string::npos : moveEnd - moveStart - 1;

    const std::string move = bestMoveLine.substr(moveStart + 1, moveLength);

    // Stockfish says "(none)" when the side to move has no legal move at all.
    // Our own code has already noticed that, but a stray value must not be
    // passed on as if it were a move.
    if (move == "(none)" || move == "0000") {
        return "";
    }

    return move;
}


void StockfishEngine::shutDown() {
    if (engineInput_ != nullptr) {
        // Asking politely first lets the engine exit on its own and release
        // its memory; the pipe closing below would do it too, less tidily.
        sendCommand("quit");
    }

    if (processHandle_ != nullptr) {
        const DWORD SHUTDOWN_GRACE_MS = 2000;

        if (WaitForSingleObject(static_cast<HANDLE>(processHandle_), SHUTDOWN_GRACE_MS)
            == WAIT_TIMEOUT) {
            TerminateProcess(static_cast<HANDLE>(processHandle_), 0);
        }
    }

    if (engineInput_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(engineInput_));
        engineInput_ = nullptr;
    }

    if (engineOutput_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(engineOutput_));
        engineOutput_ = nullptr;
    }

    if (threadHandle_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(threadHandle_));
        threadHandle_ = nullptr;
    }

    if (processHandle_ != nullptr) {
        CloseHandle(static_cast<HANDLE>(processHandle_));
        processHandle_ = nullptr;
    }

    isRunning_ = false;
}
