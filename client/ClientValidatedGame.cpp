#include "ClientValidatedGame.h"


std::array<char, SQUARE_COUNT> ClientValidatedGame::boardForDisplay() const {
    return game_.board().toDisplay();
}


bool ClientValidatedGame::isOver() const {
    return game_.isOver();
}


std::string ClientValidatedGame::sideToMoveName() const {
    return (game_.sideToMove() == Color::White) ? "White" : "Black";
}


std::string ClientValidatedGame::opponentOfSideToMoveName() const {
    return (game_.sideToMove() == Color::White) ? "Black" : "White";
}


std::string ClientValidatedGame::turnText() const {
    return sideToMoveName() + " to move";
}


std::string ClientValidatedGame::statusText() const {
    // §30: the GUI does not work out check for itself; it shows what it is told.
    return game_.isInCheck() ? "Check" : "";
}


std::string ClientValidatedGame::resultTitle() const {
    return game_.resultTitle();
}


std::string ClientValidatedGame::resultDetail() const {
    return game_.resultDetail();
}


std::string ClientValidatedGame::promotingSideName() const {
    return sideToMoveName();
}


bool ClientValidatedGame::needsPromotionChoice(int fromSquare, int toSquare) const {
    return game_.needsPromotionChoice(fromSquare, toSquare);
}


void ClientValidatedGame::onMoveChosen(const Move& move) {
    game_.makeMove(move);
}


void ClientValidatedGame::onResignConfirmed() {
    game_.resign(resigningColour());
}


void ClientValidatedGame::onDrawResponse(bool isAccepted) {
    // §52: accepting ends the game as a draw, declining leaves it running.
    if (isAccepted) {
        game_.agreeDraw();
    }
}
