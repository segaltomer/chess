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
//   g++ -std=c++17 common/*.cpp tests/perft_main.cpp -Icommon -o perft.exe -lsfml-network -lsfml-system
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

// The standard perft suite. These six positions and their counts are the ones
// chess engine authors have checked against each other for decades, and they
// were chosen to be nasty: between them they cover pins along every line,
// discovered check, castling out of and through check, castling rights lost by
// a rook being captured on its corner, promotion including under-promotion,
// and the en passant capture that is illegal because taking removes two pawns
// from one rank and exposes the king to a rook along it.
//
// Matching all of them is the evidence that §26-36 are actually implemented,
// as opposed to implemented in the cases we happened to think of.
std::vector<PerftCase> suite() {
    const std::string kiwipete =
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    const std::string endgame = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    const std::string promotions =
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    const std::string crowded =
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 1";
    const std::string steadyState =
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";

    return {
        {"start position",      STARTING_FEN, 1,        20},
        {"start position",      STARTING_FEN, 2,       400},
        {"start position",      STARTING_FEN, 3,      8902},
        {"start position",      STARTING_FEN, 4,    197281},
        {"start position",      STARTING_FEN, 5,   4865609},

        {"kiwipete",            kiwipete,     1,        48},
        {"kiwipete",            kiwipete,     2,      2039},
        {"kiwipete",            kiwipete,     3,     97862},
        {"kiwipete",            kiwipete,     4,   4085603},

        {"endgame, en passant", endgame,      1,        14},
        {"endgame, en passant", endgame,      2,       191},
        {"endgame, en passant", endgame,      3,      2812},
        {"endgame, en passant", endgame,      4,     43238},
        {"endgame, en passant", endgame,      5,    674624},

        {"promotions",          promotions,   1,         6},
        {"promotions",          promotions,   2,       264},
        {"promotions",          promotions,   3,      9467},
        {"promotions",          promotions,   4,    422333},

        {"crowded middlegame",  crowded,      1,        44},
        {"crowded middlegame",  crowded,      2,      1486},
        {"crowded middlegame",  crowded,      3,     62379},
        {"crowded middlegame",  crowded,      4,   2103487},

        {"steady state",        steadyState,  1,        46},
        {"steady state",        steadyState,  2,      2079},
        {"steady state",        steadyState,  3,     89890},
        {"steady state",        steadyState,  4,   3894594},
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
