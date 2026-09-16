// Bot match: the evidence for §19-20, that the three difficulty levels are
// significantly and demonstrably different.
//
// "Demonstrably" is the hard word. A human impression of strength is not
// evidence, and a player who does not play chess well cannot form one at all.
// So the levels are made to play each other. Two Stockfish processes are
// started at different settings, each is asked for moves in turn, and every
// move is played through our own ChessGame — so this doubles as a check that
// the rules engine accepts everything a real engine produces.
//
// The stronger level is expected to win. Because the weaker levels choose
// moves partly at random (that is what Stockfish's Skill Level does), a single
// game proves little; the suite plays each pairing from both colours.
//
// This is a test harness, not part of the Client or the Server. It is not in
// common/, client/ or server/, so it is never swept into their build commands,
// and it is excluded from the submission zip.
//
// Build and run from the project root:
//   g++ -std=c++17 -O2 -Icommon -Iclient common/*.cpp client/StockfishEngine.cpp client/BotDifficulty.cpp client/ConsoleInput.cpp tests/bot_match.cpp -o bot_match.exe -lsfml-network -lsfml-system
//   ./bot_match.exe
//
// It deliberately calls the shipped settingsFor(), so it tests the difficulty
// values the Bot actually uses rather than a copy of them that could drift.

#include "BotDifficulty.h"
#include "ChessGame.h"
#include "ChessTypes.h"
#include "Move.h"
#include "StockfishEngine.h"

#include <iostream>
#include <string>

namespace {

// No draw-by-repetition or fifty-move rule is implemented (the requirements do
// not ask for either), so two engines shuffling pieces would never stop on
// their own. A game that reaches this many half-moves is judged on material.
constexpr int MAX_HALF_MOVES = 300;

int valueOf(PieceType type) {
    switch (type) {
        case PieceType::Pawn:   return 1;
        case PieceType::Knight: return 3;
        case PieceType::Bishop: return 3;
        case PieceType::Rook:   return 5;
        case PieceType::Queen:  return 9;
        default:                return 0;
    }
}

int materialFor(const ChessGame& game, Color colour) {
    int total = 0;
    for (int square = 0; square < SQUARE_COUNT; ++square) {
        const Piece piece = game.board().pieceAt(square);
        if (!piece.isEmpty() && piece.colour == colour) {
            total += valueOf(piece.type);
        }
    }
    return total;
}

struct MatchResult {
    std::string winnerName;
    std::string reason;
    int whiteMaterial = 0;
    int blackMaterial = 0;
};

// Plays one complete game. `white` and `black` are already configured to the
// difficulty being tested.
MatchResult playOneGame(StockfishEngine& white, const std::string& whiteName,
                        StockfishEngine& black, const std::string& blackName) {
    ChessGame game;
    white.startNewGame();
    black.startNewGame();

    int halfMoves = 0;

    while (!game.isOver() && halfMoves < MAX_HALF_MOVES) {
        const bool whiteToMove = (game.sideToMove() == Color::White);
        StockfishEngine& engineToMove = whiteToMove ? white : black;

        const std::string suggestion = engineToMove.chooseMove(game.toFen());
        const Move move = Move::fromString(suggestion);

        // §47 in test form: the engine does not get to decide legality. If our
        // rules refuse its move, that is a finding, not something to work around.
        if (!game.makeMove(move)) {
            MatchResult failure;
            failure.winnerName = "NOBODY";
            failure.reason = "our rules rejected the engine move '" + suggestion + "'";
            failure.whiteMaterial = materialFor(game, Color::White);
            failure.blackMaterial = materialFor(game, Color::Black);
            return failure;
        }

        halfMoves++;
    }

    MatchResult result;
    result.whiteMaterial = materialFor(game, Color::White);
    result.blackMaterial = materialFor(game, Color::Black);

    if (game.result() == GameResult::WhiteWins) {
        result.winnerName = whiteName;
        result.reason = "checkmate";
    } else if (game.result() == GameResult::BlackWins) {
        result.winnerName = blackName;
        result.reason = "checkmate";
    } else if (game.result() == GameResult::Draw) {
        result.winnerName = "draw";
        result.reason = "stalemate";
    } else {
        result.winnerName = "draw";
        result.reason = "move limit reached";
    }

    return result;
}

void playPairing(BotDifficulty whiteLevel, BotDifficulty blackLevel) {
    const BotSettings whiteSettings = settingsFor(whiteLevel);
    const BotSettings blackSettings = settingsFor(blackLevel);

    StockfishEngine whiteEngine;
    StockfishEngine blackEngine;

    if (!whiteEngine.isRunning() || !blackEngine.isRunning()) {
        std::cout << "  could not start Stockfish: "
                  << whiteEngine.lastError() << blackEngine.lastError() << "\n";
        return;
    }

    whiteEngine.applyDifficulty(whiteSettings.skillLevel, whiteSettings.thinkingTimeMs);
    blackEngine.applyDifficulty(blackSettings.skillLevel, blackSettings.thinkingTimeMs);

    std::cout << "  " << whiteSettings.name << " (White) vs "
              << blackSettings.name << " (Black) ... " << std::flush;

    const MatchResult result = playOneGame(whiteEngine, whiteSettings.name,
                                           blackEngine, blackSettings.name);

    std::cout << "winner: " << result.winnerName
              << " (" << result.reason << "),  material "
              << result.whiteMaterial << " vs " << result.blackMaterial << "\n";
}

}   // namespace


int main() {
    std::cout << "=== Bot difficulty match (requirements 19-20) ===\n";
    std::cout << "Each pairing is played from both colours, so a win cannot be\n"
                 "explained by having moved first.\n\n";

    playPairing(BotDifficulty::Hard, BotDifficulty::Easy);
    playPairing(BotDifficulty::Easy, BotDifficulty::Hard);

    playPairing(BotDifficulty::Hard, BotDifficulty::Medium);
    playPairing(BotDifficulty::Medium, BotDifficulty::Hard);

    playPairing(BotDifficulty::Medium, BotDifficulty::Easy);
    playPairing(BotDifficulty::Easy, BotDifficulty::Medium);

    std::cout << "\nExpected: the stronger level wins every pairing, from either colour.\n";
    return 0;
}
