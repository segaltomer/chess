#include "../ProvidedGUI/ChessFrontend.hpp"

#include <array>
#include <string>

int main() {
    ChessFrontend gui;

    const std::string rows[8] = {
        "rnbqkbnr",
        "pppppppp",
        "........",
        "........",
        "........",
        "........",
        "PPPPPPPP",
        "RNBQKBNR"
    };

    std::array<char, 64> board{};

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            board[row * 8 + col] = rows[row][col];
        }
    }

    gui.setBoard(board);
    gui.setPlayerNames("White Player", "Black Player");
    gui.setTurnText("White to move");
    gui.setStatusText("");

    // Local / Online game: draw offer can be visible.
    gui.setOfferDrawVisible(true);

    // Set to false while your program should not accept board moves,
    // for example while waiting for an Online opponent or for the Bot.
    gui.setBoardInputEnabled(true);

    while (gui.isOpen()) {
        const auto event = gui.pollEvent();

        // This example deliberately does not implement chess.
        // It only demonstrates how to receive GUI events.

        if (event.type ==
            ChessFrontend::UiEventType::MoveRequested) {
            // Use event.from and event.to in YOUR program.
        }

        if (event.type ==
            ChessFrontend::UiEventType::PromotionSelected) {
            // Use event.promotion in YOUR program.
        }

        if (event.type ==
            ChessFrontend::UiEventType::ResignRequested) {
            // The Resign button reports a request only.
            
        }

        if (event.type ==
            ChessFrontend::UiEventType::ResignConfirmed) {
            // The user confirmed the resignation.
            // Local/Bot: end the game in YOUR program.
            // Online: typically send the resignation to the Server.
        }

        if (event.type ==
            ChessFrontend::UiEventType::ResignCancelled) {
            // The user pressed No. Continue the game.
        }

        if (event.type ==
            ChessFrontend::UiEventType::DrawOfferRequested) {
            // Handle draw offer in YOUR program.
        }

        if (event.type ==
            ChessFrontend::UiEventType::DrawAccepted) {
            // Handle accepted incoming draw offer.
        }

        if (event.type ==
            ChessFrontend::UiEventType::DrawDeclined) {
            // Handle declined incoming draw offer.
        }

        gui.render();
    }

    return 0;
}
