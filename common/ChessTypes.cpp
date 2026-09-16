#include "ChessTypes.h"

#include <cctype>


bool Piece::isEmpty() const {
    return type == PieceType::None;
}


Color opposite(Color colour) {
    return (colour == Color::White) ? Color::Black : Color::White;
}


int rowOf(int square) {
    return square / BOARD_SIZE;
}


int colOf(int square) {
    return square % BOARD_SIZE;
}


int squareIndex(int row, int col) {
    return row * BOARD_SIZE + col;
}


bool isOnBoard(int row, int col) {
    const bool isRowOnBoard = (row >= 0 && row < BOARD_SIZE);
    const bool isColOnBoard = (col >= 0 && col < BOARD_SIZE);

    return isRowOnBoard && isColOnBoard;
}


bool isValidSquare(int square) {
    return square >= 0 && square < SQUARE_COUNT;
}


int backRankOf(Color colour) {
    return (colour == Color::White) ? BOARD_SIZE - 1 : 0;
}


char toDisplayChar(const Piece& piece) {
    char letter = EMPTY_DISPLAY_CHAR;

    switch (piece.type) {
        case PieceType::Pawn:   letter = 'P'; break;
        case PieceType::Knight: letter = 'N'; break;
        case PieceType::Bishop: letter = 'B'; break;
        case PieceType::Rook:   letter = 'R'; break;
        case PieceType::Queen:  letter = 'Q'; break;
        case PieceType::King:   letter = 'K'; break;
        case PieceType::None:   return EMPTY_DISPLAY_CHAR;
    }

    if (piece.colour == Color::Black) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));
    }

    return letter;
}


Piece fromDisplayChar(char displayChar) {
    const bool isWhitePiece = (std::isupper(static_cast<unsigned char>(displayChar)) != 0);
    const char letter =
        static_cast<char>(std::toupper(static_cast<unsigned char>(displayChar)));

    PieceType type = PieceType::None;

    switch (letter) {
        case 'P': type = PieceType::Pawn;   break;
        case 'N': type = PieceType::Knight; break;
        case 'B': type = PieceType::Bishop; break;
        case 'R': type = PieceType::Rook;   break;
        case 'Q': type = PieceType::Queen;  break;
        case 'K': type = PieceType::King;   break;
        default:  return Piece{};
    }

    const Color colour = isWhitePiece ? Color::White : Color::Black;

    return Piece{type, colour};
}


std::string squareName(int square) {
    if (!isValidSquare(square)) {
        return "-";
    }

    const char file = static_cast<char>('a' + colOf(square));
    const char rank = static_cast<char>('8' - rowOf(square));

    return std::string{file, rank};
}


int squareFromName(const std::string& name) {
    if (name.size() != 2) {
        return NO_SQUARE;
    }

    const int col = name[0] - 'a';
    const int row = '8' - name[1];

    if (!isOnBoard(row, col)) {
        return NO_SQUARE;
    }

    return squareIndex(row, col);
}
