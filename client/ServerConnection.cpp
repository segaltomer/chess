#include "ServerConnection.h"

namespace {

// Long enough for a slow network, short enough that a player who typed the
// wrong address is not left staring at a frozen console. Without a timeout,
// an address with no machine at it can take the better part of a minute to
// give up, which looks exactly like a hung program.
const sf::Time CONNECT_TIMEOUT = sf::seconds(5);

}   // namespace


ServerConnection::~ServerConnection() {
    disconnect();
}


bool ServerConnection::connectTo(sf::IpAddress address, unsigned short port) {
    disconnect();

    const sf::Socket::Status status = socket_.connect(address, port, CONNECT_TIMEOUT);
    if (status != sf::Socket::Status::Done) {
        return false;
    }

    // The connect above needed to block so it could time out. From here on the
    // socket is polled from inside the board window's loop, which must not stop
    // drawing while it waits for the Server.
    socket_.setBlocking(false);

    isConnected_ = true;
    description_ = address.toString() + ":" + std::to_string(port);

    return true;
}


void ServerConnection::disconnect() {
    if (!isConnected_) {
        return;
    }

    socket_.disconnect();
    isConnected_ = false;
    description_.clear();
}


bool ServerConnection::isConnected() const {
    return isConnected_;
}


std::string ServerConnection::describe() const {
    return description_;
}


bool ServerConnection::send(sf::Packet& packet) {
    if (!isConnected_) {
        return false;
    }

    // Partial means some of the message went and the rest did not. SFML's
    // contract is to offer the same packet again until it reports Done; it
    // keeps track of how much is left itself.
    sf::Socket::Status status = socket_.send(packet);

    while (status == sf::Socket::Status::Partial) {
        status = socket_.send(packet);
    }

    if (status != sf::Socket::Status::Done) {
        isConnected_ = false;
        return false;
    }

    return true;
}


ServerConnection::ReceiveResult ServerConnection::receive(sf::Packet& packet) {
    if (!isConnected_) {
        return ReceiveResult::Disconnected;
    }

    const sf::Socket::Status status = socket_.receive(packet);

    switch (status) {
        case sf::Socket::Status::Done:
            return ReceiveResult::Received;

        // NotReady means nothing has arrived. Partial means a message is on its
        // way but incomplete; SFML keeps the piece it has, so the answer in
        // both cases is simply to come back next frame.
        case sf::Socket::Status::NotReady:
        case sf::Socket::Status::Partial:
            return ReceiveResult::Nothing;

        default:
            break;
    }

    isConnected_ = false;
    return ReceiveResult::Disconnected;
}
