#pragma once

#include <string>

// Board geometry.
//
// All 64 squares live in one flat array, indexed exactly the way the provided
// GUI indexes it: 0 = a8, 7 = h8, 56 = a1, 63 = h1. Using one indexing scheme
// for the engine and for the screen means there is no conversion layer between
// them, and therefore no way for the board to end up drawn flipped.
constexpr int BOARD_SIZE = 8;
constexpr int SQUARE_COUNT = 64;
constexpr int NO_SQUARE = -1;

// The four corner squares. Named because the castling rights are decided
// entirely by what happens on them.
constexpr int A8_SQUARE = 0;
constexpr int H8_SQUARE = 7;
constexpr int A1_SQUARE = 56;
constexpr int H1_SQUARE = 63;

constexpr char EMPTY_DISPLAY_CHAR = '.';

enum class Color { White, Black };

enum class PieceType { None, Pawn, Knight, Bishop, Rook, Queen, King };

// What a single square holds.
//
// This is a plain value, not an object with identity. A whole board is 64 of
// these, so copying a board in order to ask "would this move leave my own king
// in check?" costs one cheap copy rather than cloning up to 32 heap objects.
struct Piece {
    PieceType type = PieceType::None;
    Color colour = Color::White;

    bool isEmpty() const;
};

Color opposite(Color colour);

int rowOf(int square);   // 0 = rank 8 ... 7 = rank 1
int colOf(int square);   // 0 = file a ... 7 = file h
int squareIndex(int row, int col);

bool isOnBoard(int row, int col);
bool isValidSquare(int square);

// 'K' 'Q' 'R' 'B' 'N' 'P' for White, the same letters lowercase for Black,
// '.' for an empty square. This is the alphabet ChessFrontend::setBoard takes.
char toDisplayChar(const Piece& piece);

// Inverse of toDisplayChar. Anything unrecognised reads as an empty square,
// so a malformed board arriving over the network cannot produce a bad piece.
Piece fromDisplayChar(char displayChar);

// "e4" for square 28. Returns "-" for NO_SQUARE. Used for console logging and
// for the coordinate notation UCI expects.
std::string squareName(int square);

// Inverse of squareName. Anything that is not a file letter followed by a rank
// digit — including FEN's "-" — reads as NO_SQUARE.
int squareFromName(const std::string& name);
