#pragma once

#include "ServerConnection.h"

#include <optional>
#include <string>

// The first screen the Client shows (§7): where is the Server?
//
// §8 makes this a gate rather than a step — the main menu is unreachable until
// a connection has actually succeeded. So this screen keeps going until one of
// two things happens: it connects, or the player gives up.
//
// It is a class rather than a function because it remembers the address and
// port between attempts. §10 requires "try again" and "change only the port"
// to be separate choices, and neither means anything without that memory.
class ConnectionScreen {
public:
    explicit ConnectionScreen(ServerConnection& connection);

    // Returns true once connected. False means the player chose to exit, or
    // the console input ended.
    bool run();

private:
    // What the player wants after a failed attempt (§10).
    enum class NextStep { Retry, ChangeAddress, ChangePort, Exit };

    ServerConnection& connection_;

    // What the player typed, kept so messages can echo it back in their own
    // words — "localhost" rather than the 127.0.0.1 it resolved to.
    std::string addressText_;

    // The resolved address. std::optional because sf::IpAddress has no
    // "nothing yet" value of its own, and because resolving can fail.
    std::optional<sf::IpAddress> address_;

    unsigned short port_ = 0;

    bool askForAddress();
    bool askForPort();

    // Resolves the typed address and attempts the connection, reporting in
    // plain language what went wrong (§9).
    bool attemptConnection();

    bool askWhatNext(NextStep& choice);
};
