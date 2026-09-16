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

    // What a poll for an incoming message found.
    enum class ReceiveResult {
        Nothing,        // nothing has arrived yet; try again later
        Received,       // `packet` now holds one whole message
        Disconnected    // the Server has gone (§65)
    };

    // Sends one message. False if the connection has gone.
    //
    // The socket is non-blocking, so a large message can be sent in pieces;
    // this keeps offering the same packet until the whole of it has gone, which
    // is what SFML requires. Our messages are far too small for that to happen
    // in practice, but a partial send that was ignored would corrupt the stream
    // for every message after it.
    bool send(sf::Packet& packet);

    // Looks for a message without waiting. The board window has to keep drawing
    // and responding to the mouse while an online game waits on the Server, so
    // a blocking read here would freeze it (§65).
    ReceiveResult receive(sf::Packet& packet);

private:
    sf::TcpSocket socket_;
    bool isConnected_ = false;
    std::string description_;
};
