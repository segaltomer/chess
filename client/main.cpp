#include "BotDifficulty.h"
#include "BotGame.h"
#include "ConnectionScreen.h"
#include "LocalGame.h"
#include "MainMenu.h"
#include "ServerConnection.h"
#include "StockfishEngine.h"

#include <iostream>

namespace {

// §43-44: the difficulty is chosen first, and the game starts once it has
// been. The engine is only started afterwards, and if it cannot start the
// player is told and sent back to the menu rather than shown an empty board.
void runBotGame() {
    BotDifficulty difficulty = BotDifficulty::Medium;
    if (!chooseBotDifficulty(difficulty)) {
        return;
    }

    std::cout << "\nStarting Stockfish..." << std::endl;

    StockfishEngine engine;
    if (!engine.isRunning()) {
        std::cout << "The Bot is unavailable: " << engine.lastError() << "\n";
        return;
    }

    BotGame botGame(engine, settingsFor(difficulty), Color::White);
    botGame.play();
}

// Runs the chosen mode. Returns false when the player wants to leave.
bool runChoice(MainMenuChoice choice) {
    switch (choice) {
        case MainMenuChoice::LocalGame: {
            LocalGame localGame;
            localGame.play();
            return true;
        }

        case MainMenuChoice::PlayerVsBot:
            runBotGame();
            return true;

        case MainMenuChoice::OnlineGame:
            std::cout << "\nOnline Game is not available yet.\n";
            return true;

        case MainMenuChoice::Exit:
            return false;
    }

    return false;
}

}   // namespace


int main() {
    std::cout << "Chess client\n\n";

    // The connection outlives every screen that uses it, so it is created here
    // and handed out by reference. Its destructor closes the socket whichever
    // way the program leaves (§76).
    ServerConnection connection;

    // §8: the main menu is unreachable until this succeeds.
    ConnectionScreen connectionScreen(connection);
    if (!connectionScreen.run()) {
        std::cout << "Exiting.\n";
        return 0;
    }

    while (true) {
        MainMenuChoice choice = MainMenuChoice::Exit;

        if (!showMainMenu(choice)) {
            break;
        }

        if (!runChoice(choice)) {
            break;
        }
    }

    std::cout << "Goodbye.\n";

    return 0;
}
