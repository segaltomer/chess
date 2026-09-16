#include "LocalGame.h"

#include <iostream>


LocalGame::LocalGame() {
    gui_.setPlayerNames("White Player", "Black Player");

    // §48: a draw may be offered in a Local game, so the button stays. It is
    // hidden only in Bot games.
    gui_.setOfferDrawVisible(true);
}


bool LocalGame::isAwaitingPromotion() const {
    return promotionFromSquare_ != NO_SQUARE;
}


std::string LocalGame::sideToMoveName() const {
    return (game_.sideToMove() == Color::White) ? "White" : "Black";
}


std::string LocalGame::opponentName() const {
    return (game_.sideToMove() == Color::White) ? "Black" : "White";
}


void LocalGame::refreshDisplay() {
    gui_.setBoard(game_.board().toDisplay());

    if (game_.isOver()) {
        gui_.setTurnText("Game over");
        gui_.setStatusText("");
        return;
    }

    gui_.setTurnText(sideToMoveName() + " to move");

    // §30: the GUI does not work out check for itself; it shows what it is told.
    gui_.setStatusText(game_.isInCheck() ? "Check" : "");
}


void LocalGame::handleMoveRequest(const ChessFrontend::UiEvent& event) {
    if (game_.isOver() || isAwaitingPromotion()) {
        return;
    }

    // The GUI numbers squares exactly as the engine does — row 0 is rank 8,
    // column 0 is file a — so this is a direct translation with no flipping.
    const int fromSquare = squareIndex(event.from.row, event.from.col);
    const int toSquare = squareIndex(event.to.row, event.to.col);

    if (game_.needsPromotionChoice(fromSquare, toSquare)) {
        promotionFromSquare_ = fromSquare;
        promotionToSquare_ = toSquare;
        gui_.showPromotionDialog(sideToMoveName());
        return;
    }

    // An illegal move is simply refused. Nothing has to be undone, because
    // makeMove changes nothing at all unless the move was legal — the board
    // redraws unchanged and the piece appears to stay where it was.
    game_.makeMove(Move{fromSquare, toSquare, PieceType::None});
}


void LocalGame::handlePromotionChoice(char chosenPiece) {
    if (!isAwaitingPromotion()) {
        return;
    }

    // The GUI sends 'Q', 'R', 'B' or 'N' — the same letters a white piece is
    // displayed with, so the existing conversion already reads them.
    const PieceType promotion = fromDisplayChar(chosenPiece).type;

    const Move move{promotionFromSquare_, promotionToSquare_, promotion};

    promotionFromSquare_ = NO_SQUARE;
    promotionToSquare_ = NO_SQUARE;

    game_.makeMove(move);
}


void LocalGame::offerDrawToOpponent() {
    if (game_.isOver()) {
        return;
    }

    // §49: the offer appears on this same Client, for the opponent. Both
    // players are at one screen, so the message has to say who is asking and
    // who is being asked — otherwise the wrong person answers it.
    const std::string message =
        sideToMoveName() + " offers a draw. " + opponentName() + ", do you accept?";

    gui_.showDrawOfferDialog(message);
}


void LocalGame::handleEvent(const ChessFrontend::UiEvent& event) {
    switch (event.type) {
        case ChessFrontend::UiEventType::MoveRequested:
            handleMoveRequest(event);
            break;

        case ChessFrontend::UiEventType::PromotionSelected:
            handlePromotionChoice(event.promotion);
            break;

        case ChessFrontend::UiEventType::ResignRequested:
            // §54: the GUI only reports the click. We decide to confirm it.
            if (!game_.isOver()) {
                gui_.showResignConfirmDialog();
            }
            break;

        case ChessFrontend::UiEventType::ResignConfirmed:
            // §55: only now does anyone actually resign. §54 puts both players
            // on this one Client, so the resigning player is the one to move.
            game_.resign(game_.sideToMove());
            break;

        case ChessFrontend::UiEventType::ResignCancelled:
            break;   // §55: "No" means the game simply carries on.

        case ChessFrontend::UiEventType::DrawOfferRequested:
            offerDrawToOpponent();
            break;

        case ChessFrontend::UiEventType::DrawAccepted:
            game_.agreeDraw();   // §52
            break;

        case ChessFrontend::UiEventType::DrawDeclined:
            break;   // §52: declining leaves the game running.

        case ChessFrontend::UiEventType::CloseRequested:
            break;   // The GUI has already closed its window; the loop ends.

        case ChessFrontend::UiEventType::None:
            break;
    }
}


void LocalGame::showResultOnce() {
    if (!game_.isOver() || hasShownResult_) {
        return;
    }

    hasShownResult_ = true;

    gui_.setBoardInputEnabled(false);
    gui_.setOfferDrawVisible(false);

    // §39: the result and the reason, both decided by our code, never by the
    // GUI. It only displays the two strings it is handed.
    gui_.showGameOver(game_.resultTitle(), game_.resultDetail());

    // Flushed, not just newline-terminated: nothing reads the console while a
    // game is running, so an unflushed line can sit in the buffer unseen for
    // as long as the board window stays open.
    std::cout << "Game over: " << game_.resultTitle()
              << " (" << game_.resultDetail() << ")\n";
    std::cout << "Close the board window to return to the menu." << std::endl;
}


void LocalGame::play() {
    std::cout << "\nLocal game started. White moves first.\n";
    std::cout << "Click a piece, then the square to move it to.\n";
    std::cout << "Resign ends the game for whoever is to move." << std::endl;

    refreshDisplay();

    while (gui_.isOpen()) {
        const ChessFrontend::UiEvent event = gui_.pollEvent();

        handleEvent(event);
        refreshDisplay();
        showResultOnce();

        gui_.render();
    }
}
