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

// Whether the Server is still there, asked rather than assumed.
//
// A dropped TCP connection is only discovered by trying to use it, so the
// `isConnected` flag on its own is just the result of the last attempt — it
// stays true after a Server has quietly gone away. One non-blocking read
// settles it.
//
// Anything still queued is discarded on purpose: the Client is between games
// here, so the only thing that can be waiting is the tail of one that has
// already finished.
bool isServerStillThere(ServerConnection& connection) {
    if (!connection.isConnected()) {
        return false;
    }

    sf::Packet ignored;
    while (connection.receive(ignored) == ServerConnection::ReceiveResult::Received) {
        ignored.clear();
    }

    return connection.isConnected();
}

// §65: the menu is one of the "system screens" a disconnection has to be
// handled on. Nothing on it is waiting for the Server, so a connection that
// died during a Local or Bot game would otherwise go unnoticed.
//
// A lost connection is reported but does not end the program. §8 makes the menu
// conditional on connecting successfully, which this Client did; and Local and
// Bot games never needed the Server anyway, so shutting them down over it would
// take away two working modes for no reason. It is said once rather than above
// every menu.
void reportIfConnectionLost(ServerConnection& connection, bool& hasBeenReported) {
    if (hasBeenReported || isServerStillThere(connection)) {
        return;
    }

    hasBeenReported = true;

    std::cout << "\nThe connection to the server has been lost.\n";
    std::cout << "Local Game and Player vs Bot still work; Online Game does not.\n";
    std::cout << "Restart the client to connect again." << std::endl;
}

// §15, §21: the Server does the pairing, so all the Client has to do is open
// the board and ask. The connection is checked here too, so that choosing
// Online with no Server gives an explanation on the console instead of a board
// window that opens and immediately reports a lost connection (§65).
void runOnlineGame(ServerConnection& connection) {
    if (!isServerStillThere(connection)) {
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

    bool hasReportedLostConnection = false;

    while (true) {
        reportIfConnectionLost(connection, hasReportedLostConnection);

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
