#pragma once

// ChessFrontend.hpp is found through -IProvidedGUI, which the documented build
// command supplies. The engine headers are reached by relative path, because
// that command passes no -Icommon and it is fixed by the course requirements.
#include "ChessFrontend.hpp"

#include "../common/ChessGame.h"

#include <string>

// What every game played at this Client has in common.
//
// The three modes differ in exactly one thing: who decides the next move.
// A Local game asks the second person at the same keyboard, a Bot game asks
// Stockfish, an Online game asks the Server. Everything else — reading clicks,
// turning them into moves, the promotion dialog, the resign confirmation, the
// draw offer, deciding when to show the result — is identical, which is what
// §6 requires and what this class holds.
//
// `play()` is the shared loop. Subclasses do not override it; they fill in the
// few hooks below, so a mode cannot accidentally reimplement the parts that
// must behave the same everywhere (§71, §73).
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

    ChessGame game_;
    ChessFrontend gui_;

    // Called once before the first frame: set the player names, decide whether
    // the Offer Draw button belongs in this mode, print any greeting.
    virtual void onGameStarted() = 0;

    // Called every frame, after any human move has been applied and drawn.
    // A mode with a non-human opponent takes its turn here. Called repeatedly,
    // so it must decide for itself whether there is anything to do.
    virtual void onOpponentTurn() {}

    // Which colour the Resign button resigns. Local play has both players at
    // one mouse, so it is whoever is to move; a Bot game always resigns the
    // human's colour, whichever way round they are sitting.
    virtual Color resigningColour() const = 0;

    // What the Offer Draw button does. Only modes that allow draw offers need
    // to do anything here (§48).
    virtual void onDrawOffered() {}

    // Text for the side panel while the game is running. The default names the
    // side to move; a mode that is waiting on something else can say so.
    virtual std::string turnText() const;
    virtual std::string statusText() const;

    // "White" or "Black" for the side to move, for the dialogs that name it.
    std::string sideToMoveName() const;
    std::string opponentOfSideToMoveName() const;

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
