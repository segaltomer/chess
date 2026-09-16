#pragma once

// ChessFrontend.hpp is found through -IProvidedGUI, which the documented build
// command supplies. The engine headers are reached by relative path, because
// that command passes no -Icommon and it is fixed by the course requirements.
#include "ChessFrontend.hpp"

#include "../common/ChessTypes.h"
#include "../common/Move.h"

#include <array>
#include <string>

// What every game played at this Client has in common.
//
// This class owns the board window and the *sequence* of an interaction: a
// click becomes a move, a move onto the last rank becomes a promotion dialog
// and then a move, Resign becomes a confirmation dialog and only then a
// resignation, the result is shown exactly once. That sequence is identical in
// all three modes, which is what §6 requires, and `play()` is deliberately not
// virtual so no mode can quietly reimplement it.
//
// What this class pointedly does **not** own is a `ChessGame`. The three modes
// do not agree on who is allowed to decide a move: a Local or Bot game is
// validated here on the Client (§17-18), an Online game is validated on the
// Server and the Client may only ask (§22-24). If the board and the rules sat
// in this base class, `OnlineGame` would inherit `makeMove` — the exact
// capability §24 forbids it to use. Inheriting a power you are required never
// to exercise is a trap rather than a design, so the state lives below this
// class instead: `ClientValidatedGame` holds a `ChessGame`, `OnlineGame` holds
// only the last position the Server sent.
//
// So the hooks below come in two kinds. The first kind asks the mode what to
// draw; the second tells the mode what the player asked for, and lets it decide
// what that means.
class Game {
public:
    virtual ~Game() = default;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // Opens the board window and runs until the player closes it. The game
    // itself may end well before that; the final position stays on screen so
    // it can be looked at.
    void play();

protected:
    Game() = default;

    ChessFrontend gui_;

    // --- What the loop draws. The mode is the authority on all of it. ---

    virtual std::array<char, SQUARE_COUNT> boardForDisplay() const = 0;
    virtual bool isOver() const = 0;

    // The side panel while the game runs, and the game-over box when it ends.
    virtual std::string turnText() const = 0;
    virtual std::string statusText() const = 0;
    virtual std::string resultTitle() const = 0;
    virtual std::string resultDetail() const = 0;

    // Whose promotion dialog this is, for the caption naming the player (§36).
    virtual std::string promotingSideName() const = 0;

    // --- What the player asked for. The mode decides what it means. ---

    // Called once before the first frame: set the player names, decide whether
    // the Offer Draw button belongs in this mode, print any greeting.
    virtual void onGameStarted() = 0;

    // Called every frame, after any human move has been applied and drawn. A
    // mode whose opponent is not the person at this keyboard acts here — the
    // Bot searches, an Online game reads the Server. Called repeatedly, so it
    // must decide for itself whether there is anything to do.
    virtual void onOpponentTurn() {}

    // §36: whether a move between these squares still needs a piece chosen
    // before it is complete. Asked before the move is offered to the mode.
    virtual bool needsPromotionChoice(int fromSquare, int toSquare) const = 0;

    // The player has chosen a complete move. Locally validated modes play it if
    // it is legal; an Online game sends it to the Server as a request and
    // changes nothing on screen (§24).
    virtual void onMoveChosen(const Move& move) = 0;

    // §55: the player pressed Confirm on the resignation dialog. Not before.
    virtual void onResignConfirmed() = 0;

    // §48: the player pressed Offer Draw. Modes without draw offers do nothing.
    virtual void onDrawOffered() {}

    // §51-52: an offer that was shown to this Client has been answered. The
    // parameter is left unnamed here because the default does nothing with it;
    // the modes that override it name it.
    virtual void onDrawResponse(bool /* isAccepted */) {}

    // Pushes the current position and captions into the GUI. Public to the
    // subclasses because an Online game also calls it when a state message
    // arrives between frames.
    void refreshDisplay();

private:
    // A promotion takes two answers from the player: the squares, then the
    // piece. The move waits here in between (§36). NO_SQUARE means none is
    // waiting.
    int promotionFromSquare_ = NO_SQUARE;
    int promotionToSquare_ = NO_SQUARE;

    // The game-over summary is shown once. Without this it would reappear
    // every frame and could never be dismissed.
    bool hasShownResult_ = false;

    bool isAwaitingPromotion() const;

    void handleEvent(const ChessFrontend::UiEvent& event);
    void handleMoveRequest(const ChessFrontend::UiEvent& event);
    void handlePromotionChoice(char chosenPiece);
    void showResultOnce();
};
