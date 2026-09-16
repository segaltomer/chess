#pragma once

#include <string>

// Runs Stockfish as a separate process and speaks UCI to it (§45-46).
//
// UCI is the plain-text protocol every chess engine understands: we write
// commands to the engine's standard input and read its replies from its
// standard output, one line at a time. Nothing is shared but those two pipes,
// which is what makes it a genuinely separate program rather than a library.
//
// This is the only file in the project containing Windows-specific code. The
// process and pipe handles are kept as void* so that <windows.h> stays out of
// this header and out of every file that includes it.
//
// RAII (§76): the engine is started by the constructor and shut down by the
// destructor, so it cannot outlive the object that owns it. Non-copyable,
// because two owners would each try to shut the same process down.
//
// Note what this class does NOT do: it never decides whether a move is legal
// and never touches the game state. §47 keeps both of those in our own code.
// All it returns is a string of Stockfish's suggestion.
class StockfishEngine {
public:
    // Finds `stockfish/stockfish.exe` relative to the running executable and
    // starts it. Check isRunning() afterwards — a missing or unstartable
    // engine is reported, not thrown, because the Client can still offer every
    // other mode without it.
    StockfishEngine();
    ~StockfishEngine();

    StockfishEngine(const StockfishEngine&) = delete;
    StockfishEngine& operator=(const StockfishEngine&) = delete;

    bool isRunning() const;

    // Why the engine is unavailable, in words a player can act on. Empty while
    // everything is working.
    std::string lastError() const;

    // §43: how strong the Bot plays. `skillLevel` is Stockfish's own 0-20
    // scale, `thinkingTimeMs` how long it may search for each move.
    void applyDifficulty(int skillLevel, int thinkingTimeMs);

    // Tells the engine a fresh game is starting, so nothing it learned from
    // the previous one carries over.
    void startNewGame();

    // Asks for a move in the given position, as a FEN string. Returns the
    // engine's choice in UCI coordinate notation — "e2e4", or "e7e8q" for a
    // promotion — or an empty string if the engine failed to answer.
    std::string chooseMove(const std::string& positionFen);

private:
    // HANDLEs. Declared void* so this header needs no Windows headers.
    void* processHandle_ = nullptr;
    void* threadHandle_ = nullptr;
    void* engineInput_ = nullptr;    // we write commands here
    void* engineOutput_ = nullptr;   // we read replies here

    bool isRunning_ = false;
    std::string lastError_;
    int thinkingTimeMs_ = 0;

    // Output already read from the pipe but not yet split into whole lines.
    std::string pendingOutput_;

    bool startEngineProcess();
    bool completeUciHandshake();

    bool sendCommand(const std::string& command);
    bool readLine(std::string& line, int timeoutMs);

    // Reads and discards lines until one begins with `prefix`. The engine
    // chatters constantly while searching, so the reply we care about always
    // arrives behind a pile of progress reports.
    bool waitForLineStartingWith(const std::string& prefix, std::string& line,
                                 int timeoutMs);

    void shutDown();
};
