#pragma once

#include "ChessTypes.h"
#include "Move.h"

#include <SFML/Network/Packet.hpp>

#include <cstdint>
#include <string>

// The application protocol between Client and Server (§61-63).
//
// Nothing about the format is prescribed by the requirements, so this is our
// design. It is built on `sf::Packet`, which solves the one genuinely awkward
// problem in TCP for us: TCP is a stream of bytes with no message boundaries,
// so a hand-written protocol has to buffer partial reads and work out where one
// message ends and the next begins. `sf::Packet` does that framing itself, and
// packs integers and strings without any parsing code.
//
// Every message is a type byte followed by that type's fields, and **every
// in-game message carries its game id**. That is what makes §59 — never tell a
// client about a game it is not in — checkable in one place instead of being a
// rule to remember at each send.
//
// The whole wire format lives in this one file, written and read by the same
// operators, so the two ends cannot drift apart (§63).

// The first byte of every message.
//
// Numbered explicitly: the value is what actually travels between two separately
// compiled programs, so it must not quietly change when someone inserts a name
// into the middle of this list.
enum class MessageType : std::uint8_t {
    // Client to Server.
    FindGame        = 1,    // put me in the matchmaking queue (§40)
    CancelFindGame  = 2,    // take me out of it again (§42)
    MoveRequest     = 3,    // I would like to play this move (§23)
    OfferDraw       = 4,    // §48, §50
    DrawResponse    = 5,    // my answer to an offer (§51)
    Resign          = 6,    // §53, §56

    // Server to Client.
    Waiting         = 20,   // nobody to pair you with yet (§40)
    GameStarted     = 21,   // paired; here is your colour (§41)
    GameState       = 22,   // the position, after every accepted change (§22)
    MoveRejected    = 23,   // your request was refused, and why (§24)
    DrawOffered     = 24,   // your opponent offers a draw (§50)
    DrawDeclined    = 25,   // the offer you made was turned down (§52)
    GameOver        = 26    // the game has ended, with the reason (§39, §56)
};

// A message whose only field is the game it concerns: OfferDraw, Resign and
// DrawOffered all have this shape.
struct GameMessage {
    std::int32_t gameId = 0;
};

// §41: the Server has paired two clients and created the game.
struct GameStartedMessage {
    std::int32_t gameId = 0;
    Color yourColour = Color::White;
    std::string opponentName;
};

// §23: a request, never an instruction. The Server decides whether it happens.
struct MoveRequestMessage {
    std::int32_t gameId = 0;
    Move move;
};

// §22: the authoritative position, as held by the Server. The Client draws this
// and nothing else.
//
// `board` is the 64 display characters in the engine's own ordering, so it goes
// straight to the GUI. `sideToMove` is sent rather than a ready-made caption
// because each of the two Clients words it differently ("Your move" against
// "Opponent to move"), and only the Client knows which one it is.
struct GameStateMessage {
    std::int32_t gameId = 0;
    std::string board;
    Color sideToMove = Color::White;
    bool isInCheck = false;
};

// §24: the answer to a move request that was not legal. Sent so the player is
// told why rather than watching their piece silently refuse to move.
struct MoveRejectedMessage {
    std::int32_t gameId = 0;
    std::string reason;
};

// §51-52: Accept or Decline, travelling back through the Server.
struct DrawResponseMessage {
    std::int32_t gameId = 0;
    bool isAccepted = false;
};

// §39: the result and the reason, worded by `ChessGame` on the Server so that
// both Clients show identical text.
struct GameOverMessage {
    std::int32_t gameId = 0;
    std::string title;
    std::string detail;
};

sf::Packet& operator<<(sf::Packet& packet, MessageType type);
sf::Packet& operator>>(sf::Packet& packet, MessageType& type);

sf::Packet& operator<<(sf::Packet& packet, const Move& move);
sf::Packet& operator>>(sf::Packet& packet, Move& move);

sf::Packet& operator<<(sf::Packet& packet, const GameMessage& message);
sf::Packet& operator>>(sf::Packet& packet, GameMessage& message);

sf::Packet& operator<<(sf::Packet& packet, const GameStartedMessage& message);
sf::Packet& operator>>(sf::Packet& packet, GameStartedMessage& message);

sf::Packet& operator<<(sf::Packet& packet, const MoveRequestMessage& message);
sf::Packet& operator>>(sf::Packet& packet, MoveRequestMessage& message);

sf::Packet& operator<<(sf::Packet& packet, const GameStateMessage& message);
sf::Packet& operator>>(sf::Packet& packet, GameStateMessage& message);

sf::Packet& operator<<(sf::Packet& packet, const MoveRejectedMessage& message);
sf::Packet& operator>>(sf::Packet& packet, MoveRejectedMessage& message);

sf::Packet& operator<<(sf::Packet& packet, const DrawResponseMessage& message);
sf::Packet& operator>>(sf::Packet& packet, DrawResponseMessage& message);

sf::Packet& operator<<(sf::Packet& packet, const GameOverMessage& message);
sf::Packet& operator>>(sf::Packet& packet, GameOverMessage& message);

// Builds a packet holding just a type byte, for the messages that carry no
// fields at all (FindGame, CancelFindGame, Waiting).
sf::Packet makeMessage(MessageType type);

// Builds a packet holding a type byte followed by one message body.
template <typename MessageBody>
sf::Packet makeMessage(MessageType type, const MessageBody& body) {
    sf::Packet packet;
    packet << type << body;
    return packet;
}

// §67: a message that arrived with a type byte outside the list above is
// malformed, and must be rejected rather than acted on.
bool isKnownMessageType(std::uint8_t rawType);

// The name of a message type, for the Server's console log (§68-70).
std::string messageTypeName(MessageType type);
