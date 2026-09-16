#include "ConnectionScreen.h"
#include "ServerConnection.h"

#include <iostream>

int main() {
    std::cout << "Chess client\n\n";

    // The connection outlives every screen that uses it, so it is created here
    // and handed out by reference. Its destructor closes the socket whichever
    // way the program leaves (§76).
    ServerConnection connection;

    ConnectionScreen connectionScreen(connection);
    if (!connectionScreen.run()) {
        std::cout << "Exiting.\n";
        return 0;
    }

    return 0;
}
