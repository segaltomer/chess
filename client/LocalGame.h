#pragma once

// ChessFrontend.hpp is found through -IProvidedGUI, which the documented build
// command supplies. The engine headers are reached by relative path, because
// that command passes no -Icommon and it is fixed by the course requirements.
#include "ChessFrontend.hpp"

#include "../common/ChessGame.h"

#include <string>

// A game between two people sharing one Client (§13).
//
// The Server is not involved in any way. This Client owns the board, decides
// whether each move is legal and decides the result — §17 requires exactly
// that, and it is why the rules engine lives in `common/` and is linked into
// the Client as well as the Server.
//
// Both players use the same screen and the same mouse, which shapes a few
// decisions below: the player resigning is whichever one is to move, and a
// draw offer is answered on this same window by the opponent sitting next to
// them (§49).
class LocalGame {
public:
    LocalGame();

    // Opens the board window and runs until the player closes it. The game
    // itself may end well before that — the final position stays on screen
    // until the window is closed, which is what returns the player to the
    // console menu.
    void play();

private:
    ChessGame game_;
    ChessFrontend gui_;

    // A promotion takes two answers from the player: the squares, then the
    // piece. The move waits here in between (§36). NO_SQUARE means no move is
    // waiting.
    int promotionFromSquare_ = NO_SQUARE;
    int promotionToSquare_ = NO_SQUARE;

    // The game-over summary is shown once. Without this it would reappear
    // every frame, and the player could never dismiss it.
    bool hasShownResult_ = false;

    bool isAwaitingPromotion() const;

    void refreshDisplay();
    void handleEvent(const ChessFrontend::UiEvent& event);
    void handleMoveRequest(const ChessFrontend::UiEvent& event);
    void handlePromotionChoice(char chosenPiece);
    void offerDrawToOpponent();
    void showResultOnce();

    // "White" or "Black" for the side to move, for the dialogs that name it.
    std::string sideToMoveName() const;
    std::string opponentName() const;
};
