#include "BotDifficulty.h"
#include "BotGame.h"
#include "ConnectionScreen.h"
#include "LocalGame.h"
#include "MainMenu.h"
#include "OnlineGame.h"
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

// §15, §21: the Server does the pairing, so all the Client has to do is open
// the board and ask. Losing the connection is caught here rather than inside the
// game, because without it there is nothing to ask (§65).
void runOnlineGame(ServerConnection& connection) {
    if (!connection.isConnected()) {
        std::cout << "\nThe connection to the server has been lost, so an "
                     "online game cannot be started.\n";
        std::cout << "Restart the client to connect again." << std::endl;
        return;
    }

    OnlineGame onlineGame(connection);
    onlineGame.play();
}

// Runs the chosen mode. Returns false when the player wants to leave.
bool runChoice(MainMenuChoice choice, ServerConnection& connection) {
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
            runOnlineGame(connection);
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

        if (!runChoice(choice, connection)) {
            break;
        }
    }

    std::cout << "Goodbye.\n";

    return 0;
}
