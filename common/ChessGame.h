#pragma once

#include "Board.h"
#include "ChessTypes.h"
#include "Move.h"

#include <string>
#include <vector>

// How a finished game turned out. InProgress means it has not finished.
enum class GameResult { InProgress, WhiteWins, BlackWins, Draw };

// Why it finished. §39 requires the reason as well as the result, because
// "Draw" alone does not tell a player whether they agreed one or were
// stalemated, and "Black wins" does not say whether White was mated or resigned.
enum class GameEndReason { None, Checkmate, Stalemate, Resignation, DrawAgreed };

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

    // §32: the side to move is in check and has no legal move — it has lost.
    bool isCheckmate() const;

    // §33: the side to move is not in check but has no legal move — a draw.
    bool isStalemate() const;

    // §31: every move the side to move may legally play.
    std::vector<Move> legalMoves() const;

    // The legal moves that start on one square — what the GUI needs once a
    // player has picked a piece up.
    std::vector<Move> legalMovesFrom(int fromSquare) const;

    bool isLegal(const Move& move) const;

    // §36: true when a legal move between these two squares would land a pawn
    // on the far rank. A move that needs a choice is not complete until it
    // carries one — the Client clicks from/to, asks this, and if it is true
    // shows the promotion dialog and only then calls makeMove with the answer.
    bool needsPromotionChoice(int fromSquare, int toSquare) const;

    // Plays the move if it is legal for the side to move, then hands the turn
    // over (§29). Returns false and changes nothing at all otherwise, so a
    // rejected move cannot leave the game half-updated. A finished game
    // refuses every move.
    bool makeMove(const Move& move);

    // §37: the named colour gives up; the other wins. Ignored if the game has
    // already finished. Either colour may resign at any time, including on the
    // opponent's turn, which is why this takes a colour rather than assuming
    // the side to move.
    void resign(Color resigningColour);

    // §38: both players have agreed to a draw. Whether they really agreed is
    // the caller's business — on one Client for a Local game, through the
    // Server for an Online one. This only records the outcome.
    void agreeDraw();

    bool isOver() const;
    GameResult result() const;
    GameEndReason endReason() const;

    // §39: the result and the reason, worded for ChessFrontend::showGameOver,
    // which takes exactly these two strings — for example "Black wins!" and
    // "White resigned". Both are empty while the game is still going.
    //
    // They are produced here, in common/, so that a Local game, a Bot game and
    // a game running on the Server all word the same outcome identically.
    std::string resultTitle() const;
    std::string resultDetail() const;

private:
    Board board_;
    Color sideToMove_ = Color::White;
    GameResult result_ = GameResult::InProgress;
    GameEndReason endReason_ = GameEndReason::None;

    // Records checkmate or stalemate if the position has become one. Called
    // after every move, and once when a game is loaded from FEN, since that
    // position may already be finished.
    void updateResultFromPosition();
};
