#pragma once

#include <SFML/Network.hpp>

#include <memory>
#include <string>
#include <vector>

// The chess server.
//
// One thread, one loop, one `sf::SocketSelector`. The selector is told about
// the listening socket and every connected client, and `wait()` sleeps until
// at least one of them has something to say — so a server with fifty idle
// clients uses no processor time, without a thread per client.
//
// That single thread is a deliberate choice (DESIGN decision 5). With no
// shared state between threads there are no locks, and therefore an entire
// family of crash cannot happen. §64 says one client disconnecting must never
// bring the server down, and thread-per-client over unlocked shared game state
// is the classic way to fail exactly that.
//
// At this stage the server accepts and tracks connections and nothing else.
// Matchmaking and online games arrive in Phase 4, which is why clients are
// already held in a container: §60 forbids assuming only one game exists, and
// that is far cheaper to honour from the first line than to retrofit.
class Server {
public:
    // Binds the port and starts listening immediately, so a Server that exists
    // is a Server that is listening. Throws std::runtime_error if the port
    // cannot be bound — almost always because something else already has it.
    explicit Server(unsigned short port);

    // Runs the accept-and-listen loop. Does not return; the server is stopped
    // by closing its console window or pressing Ctrl+C.
    void run();

    // §11: prints the addresses and port a player types into the Client's
    // connection screen, including the LAN address another machine needs.
    void printListeningAddress() const;

private:
    // One connected client. The socket is held by pointer because
    // `sf::TcpSocket` cannot be copied, and because the selector keeps a
    // reference to it — moving the owning object around a vector must not move
    // the socket itself out from under the selector.
    struct ClientConnection {
        int id = 0;
        std::unique_ptr<sf::TcpSocket> socket;
    };

    sf::TcpListener listener_;
    sf::SocketSelector selector_;
    std::vector<ClientConnection> clients_;
    int nextClientId_ = 1;

    void acceptNewClient();
    void receiveFromClients();

    // "client 3 (192.168.1.20:51514)" — how a client is named in the console,
    // so that the log of a busy server can be read (§68-70).
    static std::string describe(const ClientConnection& client);
};
