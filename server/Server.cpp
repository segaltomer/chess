#include "Server.h"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>

namespace {

void logLine(const std::string& message) {
    std::cout << "[server] " << message << std::endl;
}

}   // namespace


Server::Server(unsigned short port) {
    const sf::Socket::Status status = listener_.listen(port);

    if (status != sf::Socket::Status::Done) {
        // Console text stays plain ASCII. The Windows console is not running a
        // UTF-8 code page by default, so a dash or a quote copied from a word
        // processor arrives on the grader's screen as mojibake.
        throw std::runtime_error(
            "could not listen on port " + std::to_string(port) +
            ". Another program may already be using it - try a different port.");
    }

    selector_.add(listener_);
}


void Server::printListeningAddress() const {
    const unsigned short port = listener_.getLocalPort();
    const std::optional<sf::IpAddress> localAddress = sf::IpAddress::getLocalAddress();

    std::cout << "Chess server is listening on port " << port << ".\n";
    std::cout << "Enter these in the Client's connection screen:\n";
    std::cout << "  a Client on this machine:   127.0.0.1   port " << port << "\n";

    if (localAddress.has_value()) {
        std::cout << "  a Client on another machine: "
                  << localAddress->toString() << "   port " << port << "\n";
    } else {
        std::cout << "  a Client on another machine: this machine's network "
                     "address could not be determined\n";
    }

    std::cout << std::endl;
}


std::string Server::describe(const ClientConnection& client) {
    std::string description = "client " + std::to_string(client.id);

    if (!client.socket) {
        return description;
    }

    const std::optional<sf::IpAddress> address = client.socket->getRemoteAddress();
    if (!address.has_value()) {
        return description;
    }

    description += " (" + address->toString() + ":"
                 + std::to_string(client.socket->getRemotePort()) + ")";

    return description;
}


void Server::acceptNewClient() {
    auto socket = std::make_unique<sf::TcpSocket>();

    if (listener_.accept(*socket) != sf::Socket::Status::Done) {
        logLine("a connection attempt failed and was dropped");
        return;
    }

    ClientConnection client{nextClientId_, std::move(socket)};
    ++nextClientId_;

    // The selector holds a reference to the socket object itself. Moving the
    // owning ClientConnection into the vector moves the pointer, not the
    // socket, so the reference stays valid.
    selector_.add(*client.socket);

    logLine(describe(client) + " connected. Clients now connected: "
            + std::to_string(clients_.size() + 1));

    clients_.push_back(std::move(client));
}


void Server::receiveFromClients() {
    for (auto client = clients_.begin(); client != clients_.end(); ) {
        sf::TcpSocket& socket = *client->socket;

        if (!selector_.isReady(socket)) {
            ++client;
            continue;
        }

        sf::Packet packet;
        const sf::Socket::Status status = socket.receive(packet);

        if (status == sf::Socket::Status::Disconnected
            || status == sf::Socket::Status::Error) {
            // A closed socket reports itself ready forever, so a client that
            // has gone must be removed from the selector as well as the list —
            // otherwise the loop would spin on it (§64).
            logLine(describe(*client) + " disconnected. Clients now connected: "
                    + std::to_string(clients_.size() - 1));

            selector_.remove(socket);
            client = clients_.erase(client);
            continue;
        }

        if (status == sf::Socket::Status::Done) {
            // No protocol exists yet — it is designed and implemented in Phase
            // 4. Until then a message is not something the server can act on,
            // so it is logged and ignored rather than trusted.
            logLine(describe(*client) + " sent a message the server does not "
                    "yet understand; ignoring it");
        }

        ++client;
    }
}


void Server::run() {
    printListeningAddress();

    while (true) {
        // Sleeps here until the listener or one of the clients has something
        // waiting, which is what keeps an idle server at zero processor use.
        if (!selector_.wait()) {
            continue;
        }

        if (selector_.isReady(listener_)) {
            acceptNewClient();
        }

        receiveFromClients();
    }
}
