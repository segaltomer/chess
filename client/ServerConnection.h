#pragma once

#include <SFML/Network.hpp>

#include <string>

// The Client's single connection to the Server.
//
// RAII (§76): the socket is closed in the destructor, so no path out of the
// program — including an exception unwinding past it — can leave it open. The
// class is non-copyable because two objects must never believe they own the
// same connection.
class ServerConnection {
public:
    ServerConnection() = default;
    ~ServerConnection();

    ServerConnection(const ServerConnection&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;

    // Tries to connect, giving up after a few seconds rather than letting the
    // player stare at a frozen console. A failed attempt leaves the object
    // unconnected and safe to try again with different details.
    bool connectTo(sf::IpAddress address, unsigned short port);

    void disconnect();
    bool isConnected() const;

    // Where we are connected, for display. Empty when not connected.
    std::string describe() const;

    // The socket itself, for the code that will speak the protocol in Phase 4.
    sf::TcpSocket& socket();

private:
    sf::TcpSocket socket_;
    bool isConnected_ = false;
    std::string description_;
};
