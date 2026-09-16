#pragma once

#include "ChessTypes.h"

#include <string>

// A move is "from this square to that square", plus the piece a pawn turns
// into when it reaches the far rank (§36).
//
// Nothing else is stored. Whether a move is a capture, a castle or an en
// passant is a fact about the board it is applied to, not about the move, so
// deriving it on the spot means it can never disagree with the position.
struct Move {
    int from = NO_SQUARE;
    int to = NO_SQUARE;
    PieceType promotion = PieceType::None;

    bool operator==(const Move& other) const;
    bool operator!=(const Move& other) const;

    // Coordinate notation: "e2e4", or "e7e8q" for a promotion. This is the
    // form UCI uses, so it is also what Stockfish is given and sends back.
    std::string toString() const;

    // The inverse, for reading Stockfish's answer. Anything that is not a move
    // comes back with NO_SQUARE squares, which no game will accept — so a
    // garbled reply is refused by the ordinary legality check rather than
    // needing a special case (§47).
    static Move fromString(const std::string& text);
};
