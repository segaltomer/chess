#pragma once

// The four things a player can choose (§12).
enum class MainMenuChoice { LocalGame, PlayerVsBot, OnlineGame, Exit };

// Shows the menu and reads a choice. §8 puts this behind the connection
// screen: it is only ever reached once a connection has actually succeeded.
//
// Returns false if the console input ended, which the caller must treat as
// "stop" rather than asking again.
bool showMainMenu(MainMenuChoice& choice);
