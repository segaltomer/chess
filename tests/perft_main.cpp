// Perft: the standard correctness test for a chess move generator.
//
// perft(depth) plays out every legal move to that depth and counts the leaf
// positions. The counts for well-known positions are published and agreed on,
// so if our number matches for a handful of positions and depths, our move
// generation, blocking, check handling, castling, en passant and promotion all
// agree with the rules of chess. If a number is wrong, it is wrong by exactly
// the number of positions the bug creates, which makes the bug findable: run
// the same test one level down on each move ("divide") until the disagreement
// narrows to a single position.
//
// This is a test harness, not part of the Client or the Server. It is not in
// common/, client/ or server/, so it is never swept into their build commands,
// and it is excluded from the submission zip.
//
// Build and run from the project root:
//   g++ -std=c++17 common/*.cpp tests/perft_main.cpp -Icommon -o perft.exe
//   ./perft.exe            run the suite
//   ./perft.exe divide "<fen>" <depth>    break one position down by move

#include "Board.h"
#include "ChessGame.h"
#include "ChessTypes.h"
#include "Move.h"
#include "Rules.h"

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

const std::string STARTING_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct PerftCase {
    std::string name;
    std::string fen;
    int depth = 0;
    long long expectedNodes = 0;
};

long long perft(const Board& board, Color colour, int depth) {
    if (depth == 0) {
        return 1;
    }

    const std::vector<Move> moves = Rules::legalMoves(board, colour);

    // At the last level the move count is the answer, so there is no reason to
    // copy a board for each of them.
    if (depth == 1) {
        return static_cast<long long>(moves.size());
    }

    long long nodes = 0;

    for (const Move& move : moves) {
        Board afterMove = board;
        afterMove.applyMove(move);
        nodes += perft(afterMove, opposite(colour), depth - 1);
    }

    return nodes;
}

bool runCase(const PerftCase& testCase) {
    const ChessGame game = ChessGame::fromFen(testCase.fen);

    const auto startedAt = std::chrono::steady_clock::now();
    const long long nodes = perft(game.board(), game.sideToMove(), testCase.depth);
    const auto finishedAt = std::chrono::steady_clock::now();

    const auto milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(finishedAt - startedAt).count();

    const bool passed = (nodes == testCase.expectedNodes);

    std::cout << (passed ? "  ok   " : "  FAIL ")
              << std::left << std::setw(34) << testCase.name
              << " depth " << testCase.depth
              << std::right << std::setw(12) << nodes;

    if (!passed) {
        std::cout << "   expected " << testCase.expectedNodes
                  << "   (off by " << (nodes - testCase.expectedNodes) << ")";
    }

    std::cout << "   " << milliseconds << " ms\n";

    return passed;
}

// Prints the node count under each of the position's legal moves. When a perft
// total is wrong, comparing this against a known-good engine points straight at
// the move whose subtree is miscounted.
void divide(const std::string& fen, int depth) {
    const ChessGame game = ChessGame::fromFen(fen);
    const std::vector<Move> moves = Rules::legalMoves(game.board(), game.sideToMove());

    long long total = 0;

    for (const Move& move : moves) {
        Board afterMove = game.board();
        afterMove.applyMove(move);

        const long long nodes = perft(afterMove, opposite(game.sideToMove()), depth - 1);
        total += nodes;

        std::cout << move.toString() << ": " << nodes << "\n";
    }

    std::cout << "\nmoves: " << moves.size() << "\nnodes: " << total << "\n";
}

// The published positions this engine is currently checked against.
//
// From the starting position, no castling is reachable within four plies and
// no en passant capture is available before the fifth, so these four depths
// test movement, blocking, captures, check and the ban on moving into check
// without depending on either special move. The rest of the standard suite —
// Kiwipete and the four positions built to stress castling, en passant and
// promotion — is added as those moves are implemented.
std::vector<PerftCase> suite() {
    return {
        {"start position", STARTING_FEN, 1,      20},
        {"start position", STARTING_FEN, 2,     400},
        {"start position", STARTING_FEN, 3,    8902},
        {"start position", STARTING_FEN, 4,  197281},
    };
}

}   // namespace


int main(int argc, char** argv) {
    if (argc >= 4 && std::string(argv[1]) == "divide") {
        divide(argv[2], std::atoi(argv[3]));
        return 0;
    }

    int failures = 0;

    for (const PerftCase& testCase : suite()) {
        if (!runCase(testCase)) {
            ++failures;
        }
    }

    std::cout << "\n";

    if (failures == 0) {
        std::cout << "ALL PERFT CASES PASSED\n";
        return 0;
    }

    std::cout << failures << " PERFT CASE(S) FAILED\n";
    return 1;
}
