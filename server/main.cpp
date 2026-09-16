#include "Server.h"

#include <exception>
#include <iostream>
#include <string>

namespace {

// Nothing special about this number; it is well above the ports that need
// privileges and is unlikely to collide with anything already running.
constexpr unsigned short DEFAULT_PORT = 53000;

constexpr int LOWEST_USABLE_PORT = 1024;
constexpr int HIGHEST_PORT = 65535;

// Reads the optional port argument. A bad one is reported and the default is
// used, rather than the server refusing to start over a typo.
unsigned short portFromArguments(int argc, char** argv) {
    const bool hasPortArgument = (argc > 1);
    if (!hasPortArgument) {
        return DEFAULT_PORT;
    }

    const std::string argument = argv[1];

    int requestedPort = 0;
    try {
        requestedPort = std::stoi(argument);
    } catch (const std::exception&) {
        std::cout << "'" << argument << "' is not a port number. Using "
                  << DEFAULT_PORT << " instead.\n";
        return DEFAULT_PORT;
    }

    const bool isUsable = (requestedPort >= LOWEST_USABLE_PORT
                           && requestedPort <= HIGHEST_PORT);
    if (!isUsable) {
        std::cout << "Port " << requestedPort << " is outside the usable range "
                  << LOWEST_USABLE_PORT << "-" << HIGHEST_PORT << ". Using "
                  << DEFAULT_PORT << " instead.\n";
        return DEFAULT_PORT;
    }

    return static_cast<unsigned short>(requestedPort);
}

}   // namespace


int main(int argc, char** argv) {
    std::cout << "Chess server\n";
    std::cout << "Usage: server.exe [port]   (default " << DEFAULT_PORT << ")\n\n";

    try {
        Server server(portFromArguments(argc, argv));
        server.run();
    } catch (const std::exception& error) {
        std::cerr << "The server could not start: " << error.what() << "\n";
        return 1;
    }

    return 0;
}
