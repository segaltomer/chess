#pragma once

#include "ChessTypes.h"
#include "Move.h"

#include <array>

enum class CastlingSide { KingSide, QueenSide };

// A chess position: where every piece stands, plus the two pieces of state
// that belong to a position but cannot be seen on it — who may still castle,
// and which square is capturable en passant right now.
//
// Board deliberately does not know whose turn it is and does not decide
// whether a move is legal. It knows how to hold a position and how to apply a
// move to it, nothing more. `Rules` decides what is legal; `ChessGame` owns
// the turn. Keeping those apart is what lets the legality check copy a board,
// play a candidate move on the copy and throw it away.
class Board {
public:
    // The standard starting position, White at the bottom.
    Board();

    // An empty square reads as Piece{} — as does any square index off the
    // board, so a bad index cannot produce a bad piece.
    Piece pieceAt(int square) const;
    void setPiece(int square, const Piece& piece);

    // NO_SQUARE unless the previous move was a pawn double step. Otherwise the
    // square the double-stepping pawn skipped over, which is where a capturing
    // pawn would land.
    int enPassantTarget() const;

    // False once the king or that rook has moved, or once that rook has been
    // captured on its corner. Says nothing about whether the squares between
    // them are empty or attacked — that is a rules question, not a board one.
    bool canCastle(Color colour, CastlingSide side) const;

    // NO_SQUARE if that colour has no king on the board, which can only happen
    // in a position built by hand.
    int kingSquare(Color colour) const;

    // Applies the move with no legality checking whatsoever: the caller has
    // already decided it is legal. Moves the rook for a castle, removes the
    // pawn taken en passant, promotes, and updates the castling rights and the
    // en passant target. Does nothing if `move.from` is empty.
    void applyMove(const Move& move);

    // The 64 characters ChessFrontend::setBoard takes. Also the form a board
    // travels in over the network, so the Client can hand it straight to the
    // GUI without decoding it.
    std::array<char, SQUARE_COUNT> toDisplay() const;

private:
    std::array<Piece, SQUARE_COUNT> squares_{};
    int enPassantTarget_ = NO_SQUARE;
    std::array<bool, 4> castlingRights_{};

    static int castlingRightIndex(Color colour, CastlingSide side);

    void clearCastlingRight(Color colour, CastlingSide side);

    // A castling king has already been moved by applyMove; this brings its
    // rook across to the square the king passed over.
    void moveCastlingRook(const Move& kingMove);

    void updateCastlingRights(const Move& move, const Piece& movingPiece);
};
