#pragma once

#include "Board.h"
#include "ChessTypes.h"
#include "Move.h"

#include <vector>

// One step of movement, as a row and column delta. A sliding piece repeats its
// direction until something stops it; a stepping piece takes it exactly once.
struct Direction {
    int rowStep = 0;
    int colStep = 0;
};

// How one kind of piece moves.
//
// These objects are stateless: they hold no square and no colour, they answer
// questions about a board handed to them. Exactly one instance of each exists,
// found with rulesFor(). That is what lets the board stay a plain array of
// values — the behaviour is polymorphic, the data is not (DESIGN decision 6).
class PieceRules {
public:
    virtual ~PieceRules() = default;

    // Appends every move this piece could make from `fromSquare`, without
    // considering whether the move would leave its own king in check. `Rules`
    // filters those out afterwards. The two stay separate because deciding
    // "am I in check?" needs move generation, so if generation also needed the
    // check test it would have to call itself.
    virtual void appendPseudoLegalMoves(const Board& board, int fromSquare,
                                        Color colour,
                                        std::vector<Move>& moves) const = 0;

    // Whether this piece, standing on `fromSquare`, attacks `targetSquare`.
    //
    // Deliberately a different question from "could it move there": a pawn
    // attacks diagonally but moves straight ahead, so asking the move
    // generator would wrongly report that a pawn attacks the empty square in
    // front of it — and a king would appear to attack the square it castles
    // onto, which would make castling out of check look legal.
    virtual bool attacksSquare(const Board& board, int fromSquare, Color colour,
                               int targetSquare) const = 0;
};

// Bishop, rook and queen: they differ only in which directions they travel, so
// the ray walking — and the rule that a ray stops at the first piece it meets
// (§28) — is written once here.
class SlidingPieceRules : public PieceRules {
public:
    explicit SlidingPieceRules(std::vector<Direction> directions);

    void appendPseudoLegalMoves(const Board& board, int fromSquare, Color colour,
                                std::vector<Move>& moves) const override;

    bool attacksSquare(const Board& board, int fromSquare, Color colour,
                       int targetSquare) const override;

private:
    std::vector<Direction> directions_;
};

// Knight and king: a fixed set of offsets, taken once, blocked only by a piece
// of their own colour standing on the destination.
class SteppingPieceRules : public PieceRules {
public:
    explicit SteppingPieceRules(std::vector<Direction> steps);

    void appendPseudoLegalMoves(const Board& board, int fromSquare, Color colour,
                                std::vector<Move>& moves) const override;

    bool attacksSquare(const Board& board, int fromSquare, Color colour,
                       int targetSquare) const override;

private:
    std::vector<Direction> steps_;
};

// The pawn is the one piece that does not move the way it captures, so it
// cannot reuse either shape above.
class PawnRules : public PieceRules {
public:
    void appendPseudoLegalMoves(const Board& board, int fromSquare, Color colour,
                                std::vector<Move>& moves) const override;

    bool attacksSquare(const Board& board, int fromSquare, Color colour,
                       int targetSquare) const override;

private:
    void appendForwardMoves(const Board& board, int fromSquare, Color colour,
                            std::vector<Move>& moves) const;

    void appendCaptures(const Board& board, int fromSquare, Color colour,
                        std::vector<Move>& moves) const;

    // Emits one move, or the four promotion choices of §36 when the pawn is
    // landing on the far rank.
    static void appendMoveOrPromotions(int fromSquare, int toSquare, Color colour,
                                       std::vector<Move>& moves);
};

class KnightRules : public SteppingPieceRules {
public:
    KnightRules();
};

class BishopRules : public SlidingPieceRules {
public:
    BishopRules();
};

class RookRules : public SlidingPieceRules {
public:
    RookRules();
};

class QueenRules : public SlidingPieceRules {
public:
    QueenRules();
};

class KingRules : public SteppingPieceRules {
public:
    KingRules();
};

// The single shared instance for a piece type. Throws std::logic_error for
// PieceType::None — an empty square has no movement rules, and every caller
// checks the square is occupied before asking.
const PieceRules& rulesFor(PieceType type);

// Which row a pawn of this colour starts on, and which row promotes it.
// Shared with Rules, which needs them for castling and for promotion checks.
int pawnForwardStep(Color colour);
int pawnStartRow(Color colour);
int pawnPromotionRow(Color colour);
