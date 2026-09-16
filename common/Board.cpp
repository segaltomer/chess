#include "Board.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>

namespace {

// Read in board-index order, so character 0 is a8 and character 63 is h1.
constexpr char STARTING_POSITION[] =
    "rnbqkbnr"
    "pppppppp"
    "........"
    "........"
    "........"
    "........"
    "PPPPPPPP"
    "RNBQKBNR";

constexpr int KING_SIDE_ROOK_COL = 7;
constexpr int QUEEN_SIDE_ROOK_COL = 0;

// Where each rook lands once the king has castled past it.
constexpr int KING_SIDE_ROOK_CASTLED_COL = 5;
constexpr int QUEEN_SIDE_ROOK_CASTLED_COL = 3;

}   // namespace


Board::Board() {
    for (int square = 0; square < SQUARE_COUNT; ++square) {
        squares_[static_cast<std::size_t>(square)] =
            fromDisplayChar(STARTING_POSITION[square]);
    }

    castlingRights_.fill(true);
}


Board Board::fromFen(const std::string& placement, const std::string& castlingField,
                     const std::string& enPassantField) {
    Board board;
    board.squares_.fill(Piece{});
    board.castlingRights_.fill(false);
    board.enPassantTarget_ = NO_SQUARE;

    // FEN lists the ranks from 8 down to 1, which is exactly our row order, so
    // a rank separator simply moves to the next row. A digit is a run of empty
    // squares, and anything else is a piece letter.
    int row = 0;
    int col = 0;

    for (char symbol : placement) {
        if (symbol == '/') {
            ++row;
            col = 0;
            continue;
        }

        const bool isEmptyRun = (std::isdigit(static_cast<unsigned char>(symbol)) != 0);
        if (isEmptyRun) {
            col += symbol - '0';
            continue;
        }

        if (isOnBoard(row, col)) {
            board.squares_[static_cast<std::size_t>(squareIndex(row, col))] =
                fromDisplayChar(symbol);
        }

        ++col;
    }

    const bool hasWhiteKingSide = (castlingField.find('K') != std::string::npos);
    const bool hasWhiteQueenSide = (castlingField.find('Q') != std::string::npos);
    const bool hasBlackKingSide = (castlingField.find('k') != std::string::npos);
    const bool hasBlackQueenSide = (castlingField.find('q') != std::string::npos);

    board.setCastlingRight(Color::White, CastlingSide::KingSide, hasWhiteKingSide);
    board.setCastlingRight(Color::White, CastlingSide::QueenSide, hasWhiteQueenSide);
    board.setCastlingRight(Color::Black, CastlingSide::KingSide, hasBlackKingSide);
    board.setCastlingRight(Color::Black, CastlingSide::QueenSide, hasBlackQueenSide);

    board.enPassantTarget_ = squareFromName(enPassantField);

    return board;
}


Piece Board::pieceAt(int square) const {
    if (!isValidSquare(square)) {
        return Piece{};
    }

    return squares_[static_cast<std::size_t>(square)];
}


void Board::setPiece(int square, const Piece& piece) {
    if (!isValidSquare(square)) {
        return;
    }

    squares_[static_cast<std::size_t>(square)] = piece;
}


int Board::enPassantTarget() const {
    return enPassantTarget_;
}


int Board::castlingRightIndex(Color colour, CastlingSide side) {
    const int colourOffset = (colour == Color::White) ? 0 : 2;
    const int sideOffset = (side == CastlingSide::KingSide) ? 0 : 1;

    return colourOffset + sideOffset;
}


bool Board::canCastle(Color colour, CastlingSide side) const {
    const int index = castlingRightIndex(colour, side);

    return castlingRights_[static_cast<std::size_t>(index)];
}


void Board::setCastlingRight(Color colour, CastlingSide side, bool allowed) {
    const int index = castlingRightIndex(colour, side);

    castlingRights_[static_cast<std::size_t>(index)] = allowed;
}


void Board::clearCastlingRight(Color colour, CastlingSide side) {
    setCastlingRight(colour, side, false);
}


int Board::kingSquare(Color colour) const {
    for (int square = 0; square < SQUARE_COUNT; ++square) {
        const Piece piece = squares_[static_cast<std::size_t>(square)];

        if (piece.type == PieceType::King && piece.colour == colour) {
            return square;
        }
    }

    return NO_SQUARE;
}


void Board::moveCastlingRook(const Move& kingMove) {
    const int backRank = rowOf(kingMove.to);
    const bool isKingSide = (colOf(kingMove.to) > colOf(kingMove.from));

    const int rookFromCol = isKingSide ? KING_SIDE_ROOK_COL : QUEEN_SIDE_ROOK_COL;
    const int rookToCol =
        isKingSide ? KING_SIDE_ROOK_CASTLED_COL : QUEEN_SIDE_ROOK_CASTLED_COL;

    const int rookFrom = squareIndex(backRank, rookFromCol);
    const int rookTo = squareIndex(backRank, rookToCol);

    setPiece(rookTo, pieceAt(rookFrom));
    setPiece(rookFrom, Piece{});
}


void Board::updateCastlingRights(const Move& move, const Piece& movingPiece) {
    if (movingPiece.type == PieceType::King) {
        clearCastlingRight(movingPiece.colour, CastlingSide::KingSide);
        clearCastlingRight(movingPiece.colour, CastlingSide::QueenSide);
    }

    // A rook leaving its corner and a rook being captured on its corner cost
    // the same right, so one test covers both cases: either way that corner no
    // longer holds the rook that started there.
    const bool touchesA1 = (move.from == A1_SQUARE || move.to == A1_SQUARE);
    const bool touchesH1 = (move.from == H1_SQUARE || move.to == H1_SQUARE);
    const bool touchesA8 = (move.from == A8_SQUARE || move.to == A8_SQUARE);
    const bool touchesH8 = (move.from == H8_SQUARE || move.to == H8_SQUARE);

    if (touchesA1) { clearCastlingRight(Color::White, CastlingSide::QueenSide); }
    if (touchesH1) { clearCastlingRight(Color::White, CastlingSide::KingSide); }
    if (touchesA8) { clearCastlingRight(Color::Black, CastlingSide::QueenSide); }
    if (touchesH8) { clearCastlingRight(Color::Black, CastlingSide::KingSide); }
}


void Board::applyMove(const Move& move) {
    const Piece movingPiece = pieceAt(move.from);
    if (movingPiece.isEmpty()) {
        return;
    }

    const int previousEnPassantTarget = enPassantTarget_;
    enPassantTarget_ = NO_SQUARE;

    const bool isPawnMove = (movingPiece.type == PieceType::Pawn);
    const bool changesFile = (colOf(move.from) != colOf(move.to));

    // A pawn capturing en passant takes a pawn that is not on the square it
    // lands on: the captured pawn sits beside the moving pawn, on the rank the
    // moving pawn started from.
    const bool isEnPassantCapture =
        isPawnMove && changesFile && (move.to == previousEnPassantTarget);
    if (isEnPassantCapture) {
        const int capturedPawnSquare = squareIndex(rowOf(move.from), colOf(move.to));
        setPiece(capturedPawnSquare, Piece{});
    }

    setPiece(move.to, movingPiece);
    setPiece(move.from, Piece{});

    const bool isKingMove = (movingPiece.type == PieceType::King);
    const int filesMoved = std::abs(colOf(move.to) - colOf(move.from));
    if (isKingMove && filesMoved == 2) {
        moveCastlingRook(move);
    }

    if (isPawnMove && move.promotion != PieceType::None) {
        setPiece(move.to, Piece{move.promotion, movingPiece.colour});
    }

    // A double step leaves the square it skipped over capturable, but only
    // until the opponent's very next move — which is why the target was reset
    // at the top of this function before anything else happened.
    const int ranksMoved = std::abs(rowOf(move.to) - rowOf(move.from));
    if (isPawnMove && ranksMoved == 2) {
        const int skippedRow = (rowOf(move.from) + rowOf(move.to)) / 2;
        enPassantTarget_ = squareIndex(skippedRow, colOf(move.from));
    }

    updateCastlingRights(move, movingPiece);
}


std::array<char, SQUARE_COUNT> Board::toDisplay() const {
    std::array<char, SQUARE_COUNT> display{};

    std::transform(squares_.begin(), squares_.end(), display.begin(),
                   [](const Piece& piece) { return toDisplayChar(piece); });

    return display;
}
