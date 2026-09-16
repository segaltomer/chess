#include "Rules.h"

#include "PieceRules.h"

#include <algorithm>

namespace {

// A crowded position has a little over forty legal moves, so reserving this
// much keeps the vector from reallocating on the common path.
constexpr std::size_t TYPICAL_MOVE_COUNT = 48;

}   // namespace


std::vector<Move> Rules::pseudoLegalMoves(const Board& board, Color colour) {
    std::vector<Move> moves;
    moves.reserve(TYPICAL_MOVE_COUNT);

    for (int fromSquare = 0; fromSquare < SQUARE_COUNT; ++fromSquare) {
        const Piece piece = board.pieceAt(fromSquare);

        const bool isOwnPiece = !piece.isEmpty() && (piece.colour == colour);
        if (!isOwnPiece) {
            continue;
        }

        rulesFor(piece.type).appendPseudoLegalMoves(board, fromSquare, colour, moves);
    }

    return moves;
}


bool Rules::isSquareAttacked(const Board& board, int square, Color attackingColour) {
    for (int fromSquare = 0; fromSquare < SQUARE_COUNT; ++fromSquare) {
        const Piece piece = board.pieceAt(fromSquare);

        const bool isAttacker = !piece.isEmpty() && (piece.colour == attackingColour);
        if (!isAttacker) {
            continue;
        }

        const bool attacks = rulesFor(piece.type)
            .attacksSquare(board, fromSquare, attackingColour, square);
        if (attacks) {
            return true;
        }
    }

    return false;
}


bool Rules::isInCheck(const Board& board, Color colour) {
    const int king = board.kingSquare(colour);

    // Only reachable in a position built by hand; a real game always has both
    // kings, and a king with nowhere to stand is not in check.
    if (king == NO_SQUARE) {
        return false;
    }

    return isSquareAttacked(board, king, opposite(colour));
}


std::vector<Move> Rules::legalMoves(const Board& board, Color colour) {
    const std::vector<Move> candidates = pseudoLegalMoves(board, colour);

    std::vector<Move> legal;
    legal.reserve(candidates.size());

    for (const Move& move : candidates) {
        // Play it on a throwaway copy and look at the result. This is why the
        // board is 64 plain values: the copy is the whole cost of the test,
        // and the test runs once per candidate move.
        Board afterMove = board;
        afterMove.applyMove(move);

        if (!isInCheck(afterMove, colour)) {
            legal.push_back(move);
        }
    }

    return legal;
}


bool Rules::isLegalMove(const Board& board, Color colour, const Move& move) {
    const std::vector<Move> legal = legalMoves(board, colour);

    return std::find(legal.begin(), legal.end(), move) != legal.end();
}
