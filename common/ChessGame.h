#pragma once

#include "Board.h"
#include "ChessTypes.h"
#include "Move.h"

#include <string>
#include <vector>

// One game of chess: the position, whose turn it is, and the rules being
// enforced on both. This object is the authority — a move it refuses did not
// happen.
//
// It knows nothing about players, screens, clients or the network, which is
// deliberate. The Client owns one of these for a Local or a Bot game; the
// Server owns one per online game. Because it is the same class in both
// places, §6 — identical rules in every mode — is true by construction rather
// than by remembering to keep two copies in step.
class ChessGame {
public:
    // The standard starting position, White to move — that is what a
    // default-built Board and Color::White already mean.
    ChessGame() = default;

    // Rebuilds a game from a full FEN string. Used by the test harness to load
    // published positions, and by the Bot to tell Stockfish where it stands.
    static ChessGame fromFen(const std::string& fen);

    const Board& board() const;

    // §29: whose turn it is. Nothing else may move.
    Color sideToMove() const;

    // §30: whether the side to move is currently in check.
    bool isInCheck() const;

    // §31: every move the side to move may legally play.
    std::vector<Move> legalMoves() const;

    // The legal moves that start on one square — what the GUI needs once a
    // player has picked a piece up.
    std::vector<Move> legalMovesFrom(int fromSquare) const;

    bool isLegal(const Move& move) const;

    // Plays the move if it is legal for the side to move, then hands the turn
    // over (§29). Returns false and changes nothing at all otherwise, so a
    // rejected move cannot leave the game half-updated.
    bool makeMove(const Move& move);

private:
    Board board_;
    Color sideToMove_ = Color::White;
};
