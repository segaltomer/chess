#include "Move.h"


bool Move::operator==(const Move& other) const {
    return from == other.from
        && to == other.to
        && promotion == other.promotion;
}


bool Move::operator!=(const Move& other) const {
    return !(*this == other);
}


Move Move::fromString(const std::string& text) {
    const bool hasPlainLength = (text.size() == 4);
    const bool hasPromotionLength = (text.size() == 5);

    if (!hasPlainLength && !hasPromotionLength) {
        return Move{};
    }

    const int fromSquare = squareFromName(text.substr(0, 2));
    const int toSquare = squareFromName(text.substr(2, 2));

    if (fromSquare == NO_SQUARE || toSquare == NO_SQUARE) {
        return Move{};
    }

    PieceType promotion = PieceType::None;
    if (hasPromotionLength) {
        promotion = fromDisplayChar(text[4]).type;
    }

    return Move{fromSquare, toSquare, promotion};
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
