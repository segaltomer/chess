#pragma once

#include "Board.h"
#include "ChessTypes.h"
#include "Move.h"

#include <vector>

// The rules of chess, written as questions about a position.
//
// Free functions in a namespace rather than a class, because none of this has
// any state to keep: every answer depends only on the board passed in. There
// would be nothing for an object to own, and a class of nothing but static
// methods is a namespace with extra ceremony.
namespace Rules {

// Every move the colour could physically make, ignoring whether it leaves its
// own king attacked. Only legalMoves() should normally need this.
std::vector<Move> pseudoLegalMoves(const Board& board, Color colour);

// Whether any piece of `attackingColour` attacks `square`. The square may be
// empty or hold a piece of either colour — "attacked" here means "a piece of
// that colour could capture something standing there".
bool isSquareAttacked(const Board& board, int square, Color attackingColour);

// §30: the colour's king stands on a square the opponent attacks.
bool isInCheck(const Board& board, Color colour);

// §31: the pseudo-legal moves minus every move that would leave the mover's
// own king in check. This is the list a game actually plays from, so it is
// also the definition the end-of-game tests rest on: no legal moves and in
// check is checkmate, no legal moves and not in check is stalemate.
std::vector<Move> legalMoves(const Board& board, Color colour);

bool isLegalMove(const Board& board, Color colour, const Move& move);

}   // namespace Rules
