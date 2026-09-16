#pragma once

#include "ClientValidatedGame.h"

// A game between two people sharing one Client (§13).
//
// The Server is not involved in any way. This Client owns the board, decides
// whether each move is legal and decides the result — §17 requires exactly
// that, and it is why the rules engine lives in `common/` and is linked into
// the Client as well as the Server.
//
// The board window and its interaction sequence live in Game; owning the rules
// and applying moves lives in ClientValidatedGame. What is left here is only
// what makes a Local game different: both players share one mouse, so the
// Resign button belongs to whoever is to move, and a draw offer is answered on
// this same window by the opponent sitting next to them (§49).
class LocalGame : public ClientValidatedGame {
protected:
    void onGameStarted() override;
    Color resigningColour() const override;
    void onDrawOffered() override;
};
