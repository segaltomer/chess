#include "LocalGame.h"

#include <iostream>


void LocalGame::onGameStarted() {
    gui_.setPlayerNames("White Player", "Black Player");

    // §48: a draw may be offered in a Local game, so the button stays. It is
    // hidden only in Bot games.
    gui_.setOfferDrawVisible(true);

    std::cout << "\nLocal game started. White moves first.\n";
    std::cout << "Click a piece, then the square to move it to.\n";
    std::cout << "Resign ends the game for whoever is to move." << std::endl;
}


Color LocalGame::resigningColour() const {
    // §54 puts both players on this one Client with one Resign button, so the
    // player resigning is the one whose turn it is.
    return game_.sideToMove();
}


void LocalGame::onDrawOffered() {
    // §49: the offer appears on this same Client, for the opponent. Both
    // players are at one screen, so the message has to say who is asking and
    // who is being asked — otherwise the wrong person answers it.
    const std::string message = sideToMoveName() + " offers a draw. "
                              + opponentOfSideToMoveName() + ", do you accept?";

    gui_.showDrawOfferDialog(message);
}
