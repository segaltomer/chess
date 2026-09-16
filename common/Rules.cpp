#include "Rules.h"

#include "PieceRules.h"

#include <algorithm>

namespace {

// A crowded position has a little over forty legal moves, so reserving this
// much keeps the vector from reallocating on the common path.
constexpr std::size_t TYPICAL_MOVE_COUNT = 48;

// §34. Castling is generated here rather than in KingRules with the other king
// moves, because two of its conditions are about attacked squares — you may
// not castle out of check, nor across a square the enemy attacks — and
// answering those needs isSquareAttacked. Having PieceRules call back into
// Rules would make the two mutually dependent; generating castling one level
// up keeps the dependency pointing one way. It is also the honest place for
// it: castling is a fact about a king, a rook and the check state together,
// not about how a king moves.
void appendCastlingForSide(const Board& board, Color colour, CastlingSide side,
                           std::vector<Move>& moves) {
    // Conditions 1 and 2: neither the king nor that rook has moved, and that
    // rook has not been captured where it stood. Board tracks all of it.
    if (!board.canCastle(colour, side)) {
        return;
    }

    const int backRank = backRankOf(colour);
    const int kingFrom = squareIndex(backRank, KING_START_COL);

    const bool isKingSide = (side == CastlingSide::KingSide);
    const int rookCol = isKingSide ? KING_SIDE_ROOK_COL : QUEEN_SIDE_ROOK_COL;

    // A position loaded from FEN can claim a right whose pieces are not there,
    // so the right is trusted only once both pieces are confirmed in place.
    const Piece king = board.pieceAt(kingFrom);
    const Piece rook = board.pieceAt(squareIndex(backRank, rookCol));

    const bool kingIsHome = (king.type == PieceType::King && king.colour == colour);
    const bool rookIsHome = (rook.type == PieceType::Rook && rook.colour == colour);
    if (!kingIsHome || !rookIsHome) {
        return;
    }

    // Condition 3: every square between the king and the rook is empty. On the
    // queen's side that includes b1/b8, which the king never crosses but the
    // rook does.
    const int step = isKingSide ? 1 : -1;
    for (int col = KING_START_COL + step; col != rookCol; col += step) {
        if (!board.pieceAt(squareIndex(backRank, col)).isEmpty()) {
            return;
        }
    }

    // Condition 5: the king may not cross an attacked square. Condition 6, the
    // square it lands on, is left to the legality filter in legalMoves, which
    // plays the whole move — king and rook — and checks the result.
    const int crossedSquare = squareIndex(backRank, KING_START_COL + step);
    if (Rules::isSquareAttacked(board, crossedSquare, opposite(colour))) {
        return;
    }

    const int kingTo = squareIndex(backRank, KING_START_COL + 2 * step);
    moves.push_back(Move{kingFrom, kingTo, PieceType::None});
}

void appendCastlingMoves(const Board& board, Color colour, std::vector<Move>& moves) {
    // Condition 4: you may not castle out of check. Tested once, because it
    // rules out both sides at the same time.
    if (Rules::isInCheck(board, colour)) {
        return;
    }

    appendCastlingForSide(board, colour, CastlingSide::KingSide, moves);
    appendCastlingForSide(board, colour, CastlingSide::QueenSide, moves);
}

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

    appendCastlingMoves(board, colour, moves);

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


bool Rules::isCheckmate(const Board& board, Color colour) {
    if (!isInCheck(board, colour)) {
        return false;
    }

    return legalMoves(board, colour).empty();
}


bool Rules::isStalemate(const Board& board, Color colour) {
    if (isInCheck(board, colour)) {
        return false;
    }

    return legalMoves(board, colour).empty();
}
