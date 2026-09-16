#pragma once

#include "HostedGame.h"
#include "Matchmaker.h"

#include <SFML/Network.hpp>

#include <map>
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
// Games live in a container keyed by id, never in single variables, because
// §60 forbids assuming only one game exists. Every message a client sends
// names the game it concerns, and every reply goes out through helpers that
// refuse to send to anyone who is not a player of that game (§59).
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

    Matchmaker matchmaker_;

    // §60: games by id, so nothing in this file can assume there is only one.
    std::map<int, HostedGame> games_;
    int nextGameId_ = 1;

    void acceptNewClient();
    void receiveFromClients();

    // Reads one message and acts on it. Anything unrecognised, or that does not
    // fit the client's current state, is logged and ignored rather than trusted
    // (§67).
    void handleMessage(int clientId, sf::Packet& packet);

    void handleFindGame(int clientId);
    void handleCancelFindGame(int clientId);
    void handleMoveRequest(int clientId, sf::Packet& packet);
    void handleOfferDraw(int clientId, sf::Packet& packet);
    void handleDrawResponse(int clientId, sf::Packet& packet);
    void handleResign(int clientId, sf::Packet& packet);

    // §41: pairs two waiting clients, creates the game and tells them both.
    void pairWaitingClients();

    // Everything that must happen after a client has gone: it leaves the
    // waiting list, and any game it was playing ends with the other player told
    // (§42, §65-66).
    //
    // Called once the connection has already been dropped, so that nothing here
    // tries to send to it. Its console description is therefore passed in,
    // there being nothing left to look it up from.
    void handleClientGone(int clientId, const std::string& description);

    // The game this client is playing, or nullptr. Found by asking the games
    // rather than by keeping a second client-to-game table, because a second
    // table is a second thing that can fall out of step with the first.
    HostedGame* findGameOfClient(int clientId);

    // The game with this id, but only if this client is actually a player in
    // it. This is where §59 is enforced: a client asking about somebody else's
    // game is refused here, once, rather than at each of the call sites.
    HostedGame* findGameForPlayer(int clientId, int gameId);

    ClientConnection* findClient(int clientId);

    // Packets are taken by value because SFML's own send takes a non-const
    // reference and reads the packet as it goes, so each recipient needs its
    // own copy, and callers pass a message built inline.
    void sendTo(int clientId, sf::Packet packet);

    // Sends the same packet to both players of a game (§22 state updates).
    void sendToBothPlayers(const HostedGame& game, sf::Packet packet);

    // Sends the current position to both players, and, if the game has just
    // finished, the result as well — after which the game is forgotten.
    void broadcastGameState(HostedGame& game);

    // The name the other player sees. Client ids are all the server knows about
    // a player, and §41 asks only that both are told the game started.
    static std::string playerName(int clientId);

    // "client 3 (192.168.1.20:51514)" — how a client is named in the console,
    // so that the log of a busy server can be read (§68-70).
    static std::string describe(const ClientConnection& client);

    // The same, found by id. Separate from `describe` because a client that has
    // already been dropped no longer has a connection to describe.
    std::string describeById(int clientId) const;
};
