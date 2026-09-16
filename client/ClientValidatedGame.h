#pragma once

#include "Game.h"

#include "../common/ChessGame.h"

// A game this Client is the authority on.
//
// §17 and §18 put Local and Bot games entirely in the Client's hands: it owns
// the board, decides whether each move is legal, and decides the result, with
// the Server not involved at all. Both modes therefore need exactly the same
// thing — a `ChessGame` to answer every question the base class asks — so it
// lives here once rather than in each of them.
//
// This is the layer that `OnlineGame` deliberately does not inherit. It holds
// the rules engine, and an Online game is required not to have one (§22-24).
// Splitting the hierarchy here is what turns that requirement from a rule
// somebody has to remember into something the compiler enforces.
class ClientValidatedGame : public Game {
protected:
    ChessGame game_;

    // Answered straight from the game this Client owns.
    std::array<char, SQUARE_COUNT> boardForDisplay() const override;
    bool isOver() const override;
    std::string turnText() const override;
    std::string statusText() const override;
    std::string resultTitle() const override;
    std::string resultDetail() const override;
    std::string promotingSideName() const override;

    bool needsPromotionChoice(int fromSquare, int toSquare) const override;

    // An illegal move is simply refused. Nothing has to be undone, because
    // makeMove changes nothing at all unless the move was legal — the board
    // redraws unchanged and the piece appears to stay where it was.
    void onMoveChosen(const Move& move) override;

    void onResignConfirmed() override;
    void onDrawResponse(bool isAccepted) override;

    // Which colour the Resign button resigns. Local play has both players at
    // one mouse, so it is whoever is to move; a Bot game always resigns the
    // human's colour, whichever way round they are sitting.
    virtual Color resigningColour() const = 0;

    // "White" or "Black" for the side to move, for the dialogs that name it.
    std::string sideToMoveName() const;
    std::string opponentOfSideToMoveName() const;
};
