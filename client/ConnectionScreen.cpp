#include "ConnectionScreen.h"

#include "ConsoleInput.h"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace {

// A client may legitimately connect to any port, including the low ones, so
// this range is wider than the one the server will bind for itself.
constexpr int LOWEST_PORT = 1;
constexpr int HIGHEST_PORT = 65535;

// Typing the port into the address box is the easiest mistake to make on this
// screen, and it is not caught for us: Winsock reads a bare number as a packed
// 32-bit address, so "53000" resolves happily to 0.0.207.8 and then fails to
// connect for reasons that look nothing like the actual mistake.
bool looksLikeAPortNumber(const std::string& text) {
    if (text.empty()) {
        return false;
    }

    return std::all_of(text.begin(), text.end(), [](unsigned char character) {
        return std::isdigit(character) != 0;
    });
}

}   // namespace


ConnectionScreen::ConnectionScreen(ServerConnection& connection)
    : connection_(connection) {
}


bool ConnectionScreen::askForAddress() {
    while (true) {
        std::string typed;
        if (!ConsoleInput::readLine("Server IP address: ", typed)) {
            return false;
        }

        if (typed.empty()) {
            std::cout << "  Please type an address. Use 127.0.0.1 if the server "
                         "is running on this machine.\n";
            continue;
        }

        if (looksLikeAPortNumber(typed)) {
            std::cout << "  '" << typed << "' looks like a port number rather "
                         "than an address.\n";
            std::cout << "  The address is asked for first, the port next.\n";
            continue;
        }

        // Resolving here rather than at connection time means a mistyped
        // address is caught the moment it is entered, instead of looking like
        // the server is missing (§9).
        const std::optional<sf::IpAddress> resolved = sf::IpAddress::resolve(typed);
        if (!resolved.has_value()) {
            std::cout << "  '" << typed << "' is not a valid IP address or "
                         "computer name.\n";
            std::cout << "  An address looks like 127.0.0.1 or 192.168.1.20.\n";
            continue;
        }

        addressText_ = typed;
        address_ = resolved;

        return true;
    }
}


bool ConnectionScreen::askForPort() {
    int typedPort = 0;
    if (!ConsoleInput::readNumberInRange("Server port: ", LOWEST_PORT, HIGHEST_PORT,
                                         typedPort)) {
        return false;
    }

    port_ = static_cast<unsigned short>(typedPort);

    return true;
}


bool ConnectionScreen::attemptConnection() {
    std::cout << "\nConnecting to " << addressText_ << " port " << port_ << " ...\n";

    if (connection_.connectTo(*address_, port_)) {
        std::cout << "Connected to " << connection_.describe() << ".\n";
        return true;
    }

    // §9: say what failed and what to check, rather than "error".
    std::cout << "Could not connect to " << addressText_ << " port " << port_ << ".\n";
    std::cout << "Nothing answered there. Check that:\n";
    std::cout << "  - the server program is running,\n";
    std::cout << "  - the address and port match what the server printed when "
                 "it started,\n";
    std::cout << "  - both machines are on the same network and no firewall is "
                 "blocking it.\n";

    return false;
}


bool ConnectionScreen::askWhatNext(NextStep& choice) {
    std::cout << "\nWhat would you like to do?\n";
    std::cout << "  1) Try " << addressText_ << " port " << port_ << " again\n";
    std::cout << "  2) Enter a different IP address\n";
    std::cout << "  3) Enter a different port\n";
    std::cout << "  4) Exit\n";

    int selection = 0;
    if (!ConsoleInput::readNumberInRange("Choice: ", 1, 4, selection)) {
        return false;
    }

    switch (selection) {
        case 1:  choice = NextStep::Retry;         break;
        case 2:  choice = NextStep::ChangeAddress; break;
        case 3:  choice = NextStep::ChangePort;    break;
        default: choice = NextStep::Exit;          break;
    }

    return true;
}


bool ConnectionScreen::run() {
    std::cout << "Connect to a chess server\n";
    std::cout << "The server prints the address and port to use when it starts.\n\n";

    const bool haveDetails = askForAddress() && askForPort();
    if (!haveDetails) {
        return false;
    }

    // §8: this loop is the gate. There is no way past it except a connection
    // that actually succeeded, or the player choosing to leave.
    while (true) {
        if (attemptConnection()) {
            return true;
        }

        NextStep choice = NextStep::Exit;
        if (!askWhatNext(choice)) {
            return false;
        }

        switch (choice) {
            case NextStep::Retry:
                break;

            case NextStep::ChangeAddress:
                if (!askForAddress()) {
                    return false;
                }
                break;

            case NextStep::ChangePort:
                if (!askForPort()) {
                    return false;
                }
                break;

            case NextStep::Exit:
                return false;
        }
    }
}
