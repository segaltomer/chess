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


sf::TcpSocket& ServerConnection::socket() {
    return socket_;
}
