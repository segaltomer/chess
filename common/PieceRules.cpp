#include "PieceRules.h"

#include <stdexcept>
#include <utility>

namespace {

const std::vector<Direction> DIAGONAL_DIRECTIONS = {
    {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
};

const std::vector<Direction> STRAIGHT_DIRECTIONS = {
    {-1, 0}, {1, 0}, {0, -1}, {0, 1}
};

const std::vector<Direction> ALL_DIRECTIONS = {
    {-1, -1}, {-1, 0}, {-1, 1},
    { 0, -1},          { 0, 1},
    { 1, -1}, { 1, 0}, { 1, 1}
};

const std::vector<Direction> KNIGHT_STEPS = {
    {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
    { 1, -2}, { 1, 2}, { 2, -1}, { 2, 1}
};

const int WHITE_PAWN_START_ROW = 6;
const int BLACK_PAWN_START_ROW = 1;

const int WHITE_PROMOTION_ROW = 0;
const int BLACK_PROMOTION_ROW = 7;

}   // namespace


int pawnForwardStep(Color colour) {
    // White advances up the board, which is toward row 0 in this indexing.
    return (colour == Color::White) ? -1 : 1;
}


int pawnStartRow(Color colour) {
    return (colour == Color::White) ? WHITE_PAWN_START_ROW : BLACK_PAWN_START_ROW;
}


int pawnPromotionRow(Color colour) {
    return (colour == Color::White) ? WHITE_PROMOTION_ROW : BLACK_PROMOTION_ROW;
}


SlidingPieceRules::SlidingPieceRules(std::vector<Direction> directions)
    : directions_(std::move(directions)) {
}


void SlidingPieceRules::appendPseudoLegalMoves(const Board& board, int fromSquare,
                                               Color colour,
                                               std::vector<Move>& moves) const {
    for (const Direction& direction : directions_) {
        int row = rowOf(fromSquare) + direction.rowStep;
        int col = colOf(fromSquare) + direction.colStep;

        while (isOnBoard(row, col)) {
            const int targetSquare = squareIndex(row, col);
            const Piece target = board.pieceAt(targetSquare);

            if (!target.isEmpty()) {
                // §27: an enemy piece can be taken, one of your own cannot.
                // §28: either way the ray stops at this square.
                const bool isEnemyPiece = (target.colour != colour);
                if (isEnemyPiece) {
                    moves.push_back(Move{fromSquare, targetSquare, PieceType::None});
                }
                break;
            }

            moves.push_back(Move{fromSquare, targetSquare, PieceType::None});

            row += direction.rowStep;
            col += direction.colStep;
        }
    }
}


bool SlidingPieceRules::attacksSquare(const Board& board, int fromSquare,
                                      Color /*colour*/, int targetSquare) const {
    for (const Direction& direction : directions_) {
        int row = rowOf(fromSquare) + direction.rowStep;
        int col = colOf(fromSquare) + direction.colStep;

        while (isOnBoard(row, col)) {
            const int square = squareIndex(row, col);
            if (square == targetSquare) {
                return true;
            }

            const bool isBlocked = !board.pieceAt(square).isEmpty();
            if (isBlocked) {
                break;
            }

            row += direction.rowStep;
            col += direction.colStep;
        }
    }

    return false;
}


SteppingPieceRules::SteppingPieceRules(std::vector<Direction> steps)
    : steps_(std::move(steps)) {
}


void SteppingPieceRules::appendPseudoLegalMoves(const Board& board, int fromSquare,
                                                Color colour,
                                                std::vector<Move>& moves) const {
    for (const Direction& step : steps_) {
        const int row = rowOf(fromSquare) + step.rowStep;
        const int col = colOf(fromSquare) + step.colStep;

        if (!isOnBoard(row, col)) {
            continue;
        }

        const int targetSquare = squareIndex(row, col);
        const Piece target = board.pieceAt(targetSquare);

        const bool isOwnPiece = !target.isEmpty() && (target.colour == colour);
        if (isOwnPiece) {
            continue;   // §27
        }

        moves.push_back(Move{fromSquare, targetSquare, PieceType::None});
    }
}


bool SteppingPieceRules::attacksSquare(const Board& /*board*/, int fromSquare,
                                       Color /*colour*/, int targetSquare) const {
    for (const Direction& step : steps_) {
        const int row = rowOf(fromSquare) + step.rowStep;
        const int col = colOf(fromSquare) + step.colStep;

        if (!isOnBoard(row, col)) {
            continue;
        }

        if (squareIndex(row, col) == targetSquare) {
            return true;
        }
    }

    return false;
}


void PawnRules::appendMoveOrPromotions(int fromSquare, int toSquare, Color colour,
                                       std::vector<Move>& moves) {
    const bool isReachingFarRank = (rowOf(toSquare) == pawnPromotionRow(colour));

    if (!isReachingFarRank) {
        moves.push_back(Move{fromSquare, toSquare, PieceType::None});
        return;
    }

    // §36: a pawn arriving on the far rank must become one of these four, so
    // the same from/to square yields four distinct moves and the player picks.
    const PieceType promotionChoices[] = {
        PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight
    };

    for (PieceType promotion : promotionChoices) {
        moves.push_back(Move{fromSquare, toSquare, promotion});
    }
}


void PawnRules::appendForwardMoves(const Board& board, int fromSquare, Color colour,
                                   std::vector<Move>& moves) const {
    const int forwardStep = pawnForwardStep(colour);
    const int col = colOf(fromSquare);
    const int oneAheadRow = rowOf(fromSquare) + forwardStep;

    if (!isOnBoard(oneAheadRow, col)) {
        return;
    }

    // A pawn is the only piece that cannot capture the way it moves, so the
    // square ahead has to be empty whatever is standing on it (§28).
    const int oneAhead = squareIndex(oneAheadRow, col);
    if (!board.pieceAt(oneAhead).isEmpty()) {
        return;
    }

    appendMoveOrPromotions(fromSquare, oneAhead, colour, moves);

    const bool isOnStartRow = (rowOf(fromSquare) == pawnStartRow(colour));
    if (!isOnStartRow) {
        return;
    }

    // The double step needs both squares clear, and the square it skips is
    // what Board records as the en passant target.
    const int twoAhead = squareIndex(oneAheadRow + forwardStep, col);
    if (board.pieceAt(twoAhead).isEmpty()) {
        moves.push_back(Move{fromSquare, twoAhead, PieceType::None});
    }
}


void PawnRules::appendCaptures(const Board& board, int fromSquare, Color colour,
                               std::vector<Move>& moves) const {
    const int targetRow = rowOf(fromSquare) + pawnForwardStep(colour);

    for (int colStep : {-1, 1}) {
        const int targetCol = colOf(fromSquare) + colStep;

        if (!isOnBoard(targetRow, targetCol)) {
            continue;
        }

        const int targetSquare = squareIndex(targetRow, targetCol);
        const Piece target = board.pieceAt(targetSquare);

        const bool isEnemyPiece = !target.isEmpty() && (target.colour != colour);

        // §35: the en passant target is the square a pawn skipped over when it
        // double-stepped last move. That square is empty — the pawn being
        // captured is beside us rather than on it — which makes this the only
        // capture in chess that lands on an empty square.
        //
        // No colour test is needed. Board clears the target on every move, so
        // the only target that can still be set belongs to the double step the
        // opponent has just played.
        const bool isEnPassantCapture =
            target.isEmpty() && (targetSquare == board.enPassantTarget());

        if (isEnemyPiece || isEnPassantCapture) {
            appendMoveOrPromotions(fromSquare, targetSquare, colour, moves);
        }
    }
}


void PawnRules::appendPseudoLegalMoves(const Board& board, int fromSquare,
                                       Color colour, std::vector<Move>& moves) const {
    appendForwardMoves(board, fromSquare, colour, moves);
    appendCaptures(board, fromSquare, colour, moves);
}


bool PawnRules::attacksSquare(const Board& /*board*/, int fromSquare, Color colour,
                              int targetSquare) const {
    const int attackRow = rowOf(fromSquare) + pawnForwardStep(colour);

    for (int colStep : {-1, 1}) {
        const int attackCol = colOf(fromSquare) + colStep;

        if (!isOnBoard(attackRow, attackCol)) {
            continue;
        }

        if (squareIndex(attackRow, attackCol) == targetSquare) {
            return true;
        }
    }

    return false;
}


KnightRules::KnightRules()
    : SteppingPieceRules(KNIGHT_STEPS) {
}


BishopRules::BishopRules()
    : SlidingPieceRules(DIAGONAL_DIRECTIONS) {
}


RookRules::RookRules()
    : SlidingPieceRules(STRAIGHT_DIRECTIONS) {
}


QueenRules::QueenRules()
    : SlidingPieceRules(ALL_DIRECTIONS) {
}


KingRules::KingRules()
    : SteppingPieceRules(ALL_DIRECTIONS) {
}


const PieceRules& rulesFor(PieceType type) {
    static const PawnRules pawnRules;
    static const KnightRules knightRules;
    static const BishopRules bishopRules;
    static const RookRules rookRules;
    static const QueenRules queenRules;
    static const KingRules kingRules;

    switch (type) {
        case PieceType::Pawn:   return pawnRules;
        case PieceType::Knight: return knightRules;
        case PieceType::Bishop: return bishopRules;
        case PieceType::Rook:   return rookRules;
        case PieceType::Queen:  return queenRules;
        case PieceType::King:   return kingRules;
        case PieceType::None:   break;
    }

    throw std::logic_error("rulesFor: an empty square has no movement rules");
}
