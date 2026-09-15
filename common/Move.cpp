#include "Move.h"


bool Move::operator==(const Move& other) const {
    return from == other.from
        && to == other.to
        && promotion == other.promotion;
}


bool Move::operator!=(const Move& other) const {
    return !(*this == other);
}


std::string Move::toString() const {
    const bool hasRealSquares = isValidSquare(from) && isValidSquare(to);
    if (!hasRealSquares) {
        return "0000";   // UCI's spelling of "no move"
    }

    std::string text = squareName(from) + squareName(to);

    if (promotion != PieceType::None) {
        // UCI wants the promotion piece as a lowercase letter, which is
        // exactly how a black piece is already displayed.
        text += toDisplayChar(Piece{promotion, Color::Black});
    }

    return text;
}
