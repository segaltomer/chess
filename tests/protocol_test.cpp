// Protocol test: drives a running Server the way real Clients would, and checks
// that its answers are the ones the requirements ask for.
//
// This is the evidence for the parts of online play that cannot be seen by
// playing a game: that two games really are independent (§58-60), that a Client
// is refused when it asks about somebody else's game (§59), that a move is
// validated on the Server rather than the Client (§22), and that neither a
// disconnection nor a malformed message disturbs anything else (§64, §67).
// Several of those need three or four Clients connected at once, which is
// awkward to arrange by hand and easy to arrange here.
//
// It connects as real Clients over a real socket — there is no stub or fake
// anywhere in it — so what it exercises is the actual Server, including its
// framing and its socket handling.
//
// This is a test harness, not part of the Client or the Server. It is not in
// common/, client/ or server/, so it is never swept into their build commands,
// and it is excluded from the submission zip.
//
// Build and run from the project root. Unlike the other two harnesses this one
// needs a Server to talk to, so start one first on the same port:
//
//   g++ -std=c++17 -O2 -Icommon common/*.cpp tests/protocol_test.cpp -o protocol_test.exe -lsfml-network -lsfml-system
//   ./server.exe 53100            in one window
//   ./protocol_test.exe 53100     in another
//
// It exits 0 if every check passed. Use a port nothing else is on; the test
// leaves finished games behind on the Server, so a fresh one is tidiest.

#include "Protocol.h"
#include "ChessTypes.h"
#include "Move.h"

#include <SFML/Network.hpp>

#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace {

int failureCount = 0;
int checkCount = 0;

void check(bool condition, const std::string& what) {
    checkCount++;

    if (condition) {
        std::cout << "  ok   " << what << "\n";
        return;
    }

    std::cout << "  FAIL " << what << "\n";
    failureCount++;
}

// How long to wait for a message that should arrive.
constexpr int REPLY_TIMEOUT_MS = 1500;

// How long to wait before concluding that nothing is coming. Shorter, because
// this one is paid on every check that expects silence.
constexpr int SILENCE_TIMEOUT_MS = 400;

// One Client's end of a connection to the Server. Deliberately does not reuse
// the real `ServerConnection`: a test that shares code with the thing it is
// testing cannot catch a fault in the shared part.
class TestClient {
public:
    bool connect(unsigned short port) {
        const std::optional<sf::IpAddress> address =
            sf::IpAddress::resolve(std::string("127.0.0.1"));

        if (!address.has_value()) {
            return false;
        }

        if (socket_.connect(*address, port, sf::seconds(3)) != sf::Socket::Status::Done) {
            return false;
        }

        socket_.setBlocking(false);

        return true;
    }

    void disconnect() {
        socket_.disconnect();
    }

    void send(sf::Packet packet) {
        sf::Socket::Status status = socket_.send(packet);

        while (status == sf::Socket::Status::Partial) {
            status = socket_.send(packet);
        }
    }

    // Waits for one message. False if none arrived before the timeout.
    bool receive(sf::Packet& packet, int timeoutMs = REPLY_TIMEOUT_MS) {
        sf::Clock clock;

        while (clock.getElapsedTime().asMilliseconds() < timeoutMs) {
            if (socket_.receive(packet) == sf::Socket::Status::Done) {
                return true;
            }

            sf::sleep(sf::milliseconds(5));
        }

        return false;
    }

    // Reads one message and names its type, so a failing check can say what
    // turned up instead of the expected message.
    std::string receiveType(sf::Packet& packet, int timeoutMs = REPLY_TIMEOUT_MS) {
        if (!receive(packet, timeoutMs)) {
            return "<nothing>";
        }

        std::uint8_t rawType = 0;
        packet >> rawType;

        if (!isKnownMessageType(rawType)) {
            return "<unknown>";
        }

        return messageTypeName(static_cast<MessageType>(rawType));
    }

    // True if nothing at all arrives. This is how "the Server ignored that" and
    // "no other game was disturbed" are checked.
    bool receivesNothing(int timeoutMs = SILENCE_TIMEOUT_MS) {
        sf::Packet packet;
        return !receive(packet, timeoutMs);
    }

private:
    sf::TcpSocket socket_;
};

sf::Packet gameMessage(MessageType type, std::int32_t gameId) {
    GameMessage message;
    message.gameId = gameId;
    return makeMessage(type, message);
}

sf::Packet moveRequest(std::int32_t gameId, const std::string& uci) {
    MoveRequestMessage request;
    request.gameId = gameId;
    request.move = Move::fromString(uci);
    return makeMessage(MessageType::MoveRequest, request);
}

// Reads a GameStarted, returning the game id and colour it carried.
bool expectGameStarted(TestClient& client, std::int32_t& gameId, Color& colour) {
    sf::Packet packet;
    if (client.receiveType(packet) != "GameStarted") {
        return false;
    }
    GameStartedMessage message;
    if (!(packet >> message)) {
        return false;
    }
    gameId = message.gameId;
    colour = message.yourColour;
    return true;
}

bool expectGameState(TestClient& client, GameStateMessage& state) {
    sf::Packet packet;
    if (client.receiveType(packet) != "GameState") {
        return false;
    }
    return static_cast<bool>(packet >> state);
}

bool expectGameOver(TestClient& client, GameOverMessage& over) {
    sf::Packet packet;
    if (client.receiveType(packet) != "GameOver") {
        return false;
    }
    return static_cast<bool>(packet >> over);
}

// Plays one move that is expected to be accepted, and reads the resulting
// position from both players. `state` ends up holding the new position.
bool playAcceptedMove(TestClient& mover, TestClient& waiter, std::int32_t gameId,
                      const std::string& uci, GameStateMessage& state) {
    mover.send(moveRequest(gameId, uci));

    return expectGameState(mover, state) && expectGameState(waiter, state);
}

// Pairs two fresh clients and reads everything the Server sends about the new
// game, leaving both sockets clean. Returns the game id, or 0 on failure.
std::int32_t startGame(TestClient& white, TestClient& black, unsigned short port) {
    if (!white.connect(port) || !black.connect(port)) {
        return 0;
    }

    sf::Packet packet;
    white.send(makeMessage(MessageType::FindGame));
    if (white.receiveType(packet) != "Waiting") {
        return 0;
    }

    black.send(makeMessage(MessageType::FindGame));

    std::int32_t whiteGameId = 0;
    std::int32_t blackGameId = 0;
    Color whiteColour = Color::Black;
    Color blackColour = Color::White;

    if (!expectGameStarted(white, whiteGameId, whiteColour)
        || !expectGameStarted(black, blackGameId, blackColour)) {
        return 0;
    }

    GameStateMessage opening;
    if (!expectGameState(white, opening) || !expectGameState(black, opening)) {
        return 0;
    }

    const bool coloursAreRight =
        (whiteColour == Color::White && blackColour == Color::Black);

    return (whiteGameId == blackGameId && coloursAreRight) ? whiteGameId : 0;
}

}   // namespace


int main(int argc, char* argv[]) {
    const unsigned short port =
        (argc > 1) ? static_cast<unsigned short>(std::stoi(argv[1])) : 53100;

    std::cout << "=== matchmaking (secs 40-41) ===\n";

    TestClient white;
    TestClient black;
    check(white.connect(port), "first client connects");
    check(black.connect(port), "second client connects");

    white.send(makeMessage(MessageType::FindGame));

    sf::Packet packet;
    check(white.receiveType(packet) == "Waiting",
          "a lone client is told it is waiting");

    black.send(makeMessage(MessageType::FindGame));

    std::int32_t whiteGameId = 0;
    std::int32_t blackGameId = 0;
    Color whiteColour = Color::Black;
    Color blackColour = Color::White;

    check(expectGameStarted(white, whiteGameId, whiteColour),
          "first client is told the game started");
    check(expectGameStarted(black, blackGameId, blackColour),
          "second client is told the game started");
    check(whiteGameId == blackGameId, "both are given the same game id");
    check(whiteColour == Color::White && blackColour == Color::Black,
          "the two are given opposite colours");

    GameStateMessage state;
    check(expectGameState(white, state), "first client is sent the position");
    check(expectGameState(black, state), "second client is sent the position");
    check(state.board.size() == 64, "the position is 64 squares");
    check(state.board[squareFromName("e1")] == 'K',
          "the position is the standard opening one");
    check(state.sideToMove == Color::White, "White is to move first");

    std::cout << "\n=== move validation on the server (secs 22, 29) ===\n";

    black.send(moveRequest(whiteGameId, "e7e5"));
    check(black.receiveType(packet) == "MoveRejected",
          "a move played out of turn is rejected");
    check(white.receivesNothing(),
          "a rejected move is not broadcast to the opponent");

    white.send(moveRequest(whiteGameId, "e2e5"));
    check(white.receiveType(packet) == "MoveRejected",
          "an illegal move is rejected");

    white.send(moveRequest(whiteGameId, "e2e4"));
    check(expectGameState(white, state), "a legal move is confirmed to the mover");
    check(state.board[squareFromName("e4")] == 'P'
          && state.board[squareFromName("e2")] == EMPTY_DISPLAY_CHAR,
          "the confirmed position has the pawn on its new square");
    check(state.sideToMove == Color::Black, "the turn passes to the opponent");
    check(expectGameState(black, state), "the opponent is sent the new position");

    std::cout << "\n=== a second, independent game (secs 57-60) ===\n";

    TestClient third;
    TestClient fourth;
    check(third.connect(port), "third client connects");
    check(fourth.connect(port), "fourth client connects");

    third.send(makeMessage(MessageType::FindGame));
    check(third.receiveType(packet) == "Waiting", "the third client waits");
    fourth.send(makeMessage(MessageType::FindGame));

    std::int32_t secondGameId = 0;
    Color thirdColour = Color::Black;
    check(expectGameStarted(third, secondGameId, thirdColour),
          "the third and fourth clients are paired");
    std::int32_t fourthGameId = 0;
    Color fourthColour = Color::White;
    check(expectGameStarted(fourth, fourthGameId, fourthColour),
          "the fourth client is told too");
    check(secondGameId != whiteGameId, "the second game has its own id");
    check(expectGameState(third, state), "the second game gets its own position");
    check(expectGameState(fourth, state), "both players of it do");

    check(white.receivesNothing() && black.receivesNothing(),
          "the first game's players hear nothing about the second game");

    // sec 59: a client asking about a game it is not in must be refused.
    third.send(moveRequest(whiteGameId, "e7e5"));
    check(third.receivesNothing(),
          "a move aimed at somebody else's game gets no reply");
    check(white.receivesNothing() && black.receivesNothing(),
          "and does not disturb that game's players");

    std::cout << "\n=== draw offers (secs 50-52) ===\n";

    white.send(gameMessage(MessageType::OfferDraw, whiteGameId));
    check(black.receiveType(packet) == "DrawOffered",
          "the offer reaches the opponent");
    check(white.receivesNothing(), "and is not echoed to the player who made it");
    check(third.receivesNothing() && fourth.receivesNothing(),
          "and does not reach the other game");

    DrawResponseMessage decline;
    decline.gameId = whiteGameId;
    decline.isAccepted = false;
    black.send(makeMessage(MessageType::DrawResponse, decline));
    check(white.receiveType(packet) == "DrawDeclined",
          "declining is reported back to the offering player");

    white.send(gameMessage(MessageType::OfferDraw, whiteGameId));
    check(black.receiveType(packet) == "DrawOffered", "a second offer is allowed");

    DrawResponseMessage accept;
    accept.gameId = whiteGameId;
    accept.isAccepted = true;
    black.send(makeMessage(MessageType::DrawResponse, accept));

    check(expectGameState(white, state), "accepting sends a final position");
    GameOverMessage over;
    check(expectGameOver(white, over), "and ends the game for the offering player");
    check(over.title == "Draw" && over.detail == "Draw agreed",
          "the result is a draw by agreement, worded by the server");
    check(expectGameState(black, state), "the accepting player gets it too");
    check(expectGameOver(black, over), "and the same result");

    std::cout << "\n=== resignation (secs 53-56) ===\n";

    third.send(gameMessage(MessageType::Resign, secondGameId));
    check(expectGameState(third, state), "resigning sends a final position");
    check(expectGameOver(third, over), "and ends the game for the resigner");
    check(over.title == "Black wins!" && over.detail == "White resigned",
          "the opponent wins and the reason says who resigned");
    check(expectGameState(fourth, state), "the opponent is updated by the server");
    check(expectGameOver(fourth, over), "and told the result");

    std::cout << "\n=== disconnection mid-game (secs 64-66) ===\n";

    TestClient fifth;
    TestClient sixth;
    check(fifth.connect(port), "fifth client connects");
    check(sixth.connect(port), "sixth client connects");
    fifth.send(makeMessage(MessageType::FindGame));
    check(fifth.receiveType(packet) == "Waiting", "the fifth client waits");
    sixth.send(makeMessage(MessageType::FindGame));

    std::int32_t thirdGameId = 0;
    Color fifthColour = Color::Black;
    check(expectGameStarted(fifth, thirdGameId, fifthColour), "a third game starts");
    std::int32_t sixthGameId = 0;
    Color sixthColour = Color::White;
    check(expectGameStarted(sixth, sixthGameId, sixthColour), "both are told");
    check(expectGameState(fifth, state), "the third game gets a position");
    check(expectGameState(sixth, state), "both players of it do");

    fifth.disconnect();

    check(expectGameState(sixth, state),
          "the remaining player is sent a final position");
    check(expectGameOver(sixth, over), "and told the game is over");
    check(over.title == "Black wins!" && over.detail == "White disconnected",
          "the reason says the opponent disconnected, not that they resigned");

    std::cout << "\n=== bad input, and the server surviving it (sec 67) ===\n";

    TestClient rude;
    check(rude.connect(port), "another client connects");

    sf::Packet nonsense;
    nonsense << static_cast<std::uint8_t>(99);
    rude.send(nonsense);
    check(rude.receivesNothing(), "an unknown message type gets no reply");

    sf::Packet empty;
    rude.send(empty);
    check(rude.receivesNothing(), "an empty message gets no reply");

    // A MoveRequest whose body is missing: the type byte is valid, the rest is not.
    sf::Packet truncated;
    truncated << MessageType::MoveRequest;
    rude.send(truncated);
    check(rude.receivesNothing(), "a truncated message gets no reply");

    // A message only the server is supposed to send.
    rude.send(makeMessage(MessageType::GameOver, over));
    check(rude.receivesNothing(), "a server-only message from a client is ignored");

    // sec 67: a move for a game that has finished and been forgotten.
    rude.send(moveRequest(whiteGameId, "e2e4"));
    check(rude.receivesNothing(), "a move for a finished game is ignored");

    std::cout << "\n=== the server is still working afterwards ===\n";

    rude.send(makeMessage(MessageType::FindGame));
    check(rude.receiveType(packet) == "Waiting",
          "the server still answers after all of that");

    rude.send(makeMessage(MessageType::CancelFindGame));

    TestClient seventh;
    TestClient eighth;
    check(seventh.connect(port), "more clients can still connect");
    check(eighth.connect(port), "and be paired");
    seventh.send(makeMessage(MessageType::FindGame));
    check(seventh.receiveType(packet) == "Waiting",
          "a cancelled searcher is not paired with the next arrival");
    eighth.send(makeMessage(MessageType::FindGame));

    std::int32_t lastGameId = 0;
    Color seventhColour = Color::Black;
    check(expectGameStarted(seventh, lastGameId, seventhColour),
          "matchmaking still works at the end");
    check(rude.receivesNothing(), "the cancelled client was not put in a game");

    std::cout << "\n=== messages that do not fit the current state (sec 67) ===\n";

    // Finish reading the pairing of seventh and eighth so the socket is clean.
    std::int32_t eighthGameId = 0;
    Color eighthColour = Color::White;
    check(expectGameStarted(eighth, eighthGameId, eighthColour),
          "the eighth client is told the game started");
    check(expectGameState(seventh, state), "and both are sent the position");
    check(expectGameState(eighth, state), "both of them");

    seventh.send(makeMessage(MessageType::FindGame));
    check(seventh.receivesNothing(),
          "asking for a game while already in one is ignored");

    DrawResponseMessage strayResponse;
    strayResponse.gameId = lastGameId;
    strayResponse.isAccepted = true;
    seventh.send(makeMessage(MessageType::DrawResponse, strayResponse));
    check(seventh.receivesNothing() && eighth.receivesNothing(),
          "answering a draw offer nobody made is ignored");

    seventh.send(gameMessage(MessageType::OfferDraw, lastGameId));
    check(eighth.receiveType(packet) == "DrawOffered", "a real offer still works");

    seventh.send(gameMessage(MessageType::OfferDraw, lastGameId));
    check(seventh.receivesNothing() && eighth.receivesNothing(),
          "a second offer while one is outstanding is ignored");

    DrawResponseMessage ownAnswer;
    ownAnswer.gameId = lastGameId;
    ownAnswer.isAccepted = true;
    seventh.send(makeMessage(MessageType::DrawResponse, ownAnswer));
    check(seventh.receivesNothing() && eighth.receivesNothing(),
          "answering your own draw offer is ignored");

    std::cout << "\n=== disconnecting while waiting to be paired (sec 42) ===\n";

    TestClient ghost;
    check(ghost.connect(port), "a client connects and searches");
    ghost.send(makeMessage(MessageType::FindGame));
    check(ghost.receiveType(packet) == "Waiting", "it is told it is waiting");

    ghost.disconnect();
    sf::sleep(sf::milliseconds(250));

    TestClient ninth;
    check(ninth.connect(port), "another client connects afterwards");
    ninth.send(makeMessage(MessageType::FindGame));
    check(ninth.receiveType(packet) == "Waiting",
          "it waits rather than being paired with the client that vanished");

    TestClient tenth;
    check(tenth.connect(port), "and one more connects");
    tenth.send(makeMessage(MessageType::FindGame));

    std::int32_t ninthGameId = 0;
    Color ninthColour = Color::Black;
    check(expectGameStarted(ninth, ninthGameId, ninthColour),
          "those two are paired normally");

    std::cout << "\n=== a game ending in checkmate (secs 32, 39) ===\n";

    TestClient mateWhite;
    TestClient mateBlack;
    const std::int32_t mateGame = startGame(mateWhite, mateBlack, port);
    check(mateGame != 0, "a game starts for the checkmate test");

    // The fastest mate there is: White opens the diagonal to its own king and
    // Black's queen arrives on h4.
    GameStateMessage mateState;
    check(playAcceptedMove(mateWhite, mateBlack, mateGame, "f2f3", mateState), "1. f3");
    check(playAcceptedMove(mateBlack, mateWhite, mateGame, "e7e5", mateState), "1... e5");
    check(playAcceptedMove(mateWhite, mateBlack, mateGame, "g2g4", mateState), "2. g4");

    mateBlack.send(moveRequest(mateGame, "d8h4"));

    GameOverMessage mateOver;
    check(expectGameState(mateBlack, mateState), "the mating move is confirmed");
    check(expectGameOver(mateBlack, mateOver), "and the game is declared over");
    check(mateOver.title == "Black wins!" && mateOver.detail == "Checkmate",
          "the Server works out checkmate itself and words the result");
    check(expectGameState(mateWhite, mateState), "the mated player is updated too");
    check(expectGameOver(mateWhite, mateOver), "and told the same result");

    // sec 67: the game is finished and forgotten, so nothing more is accepted.
    mateWhite.send(moveRequest(mateGame, "e2e4"));
    check(mateWhite.receivesNothing(), "a move after checkmate is ignored");

    std::cout << "\n=== promotion through the Server (sec 36) ===\n";

    TestClient promoWhite;
    TestClient promoBlack;
    const std::int32_t promoGame = startGame(promoWhite, promoBlack, port);
    check(promoGame != 0, "a game starts for the promotion test");

    // March the a-pawn up, taking on b7, while Black shuffles on the far side.
    GameStateMessage promoState;
    const std::string whitePlan[] = {"a2a4", "a4a5", "a5a6", "a6b7"};
    const std::string blackPlan[] = {"h7h6", "h6h5", "h5h4", "h4h3"};

    bool marchWorked = true;
    for (int i = 0; i < 4; ++i) {
        marchWorked = marchWorked
            && playAcceptedMove(promoWhite, promoBlack, promoGame, whitePlan[i], promoState)
            && playAcceptedMove(promoBlack, promoWhite, promoGame, blackPlan[i], promoState);
    }
    check(marchWorked, "a pawn reaches the seventh rank");

    // sec 36: a promoting move is not complete without a piece chosen, so this
    // one must be refused rather than half-applied.
    promoWhite.send(moveRequest(promoGame, "b7a8"));
    sf::Packet promoPacket;
    check(promoWhite.receiveType(promoPacket) == "MoveRejected",
          "promoting without naming a piece is rejected, not guessed at");

    check(playAcceptedMove(promoWhite, promoBlack, promoGame, "b7a8q", promoState),
          "the same move with a queen chosen is accepted");
    check(promoState.board[squareFromName("a8")] == 'Q',
          "a white queen now stands on a8");

    // The other three choices sec 36 requires are the same path with a
    // different letter; that the piece asked for is the piece that appears is
    // what matters, and an under-promotion proves it is not hardcoded.
    TestClient underWhite;
    TestClient underBlack;
    const std::int32_t underGame = startGame(underWhite, underBlack, port);
    check(underGame != 0, "a game starts for the under-promotion test");

    GameStateMessage underState;
    bool underMarch = true;
    for (int i = 0; i < 4; ++i) {
        underMarch = underMarch
            && playAcceptedMove(underWhite, underBlack, underGame, whitePlan[i], underState)
            && playAcceptedMove(underBlack, underWhite, underGame, blackPlan[i], underState);
    }
    check(underMarch, "a pawn reaches the seventh rank again");

    check(playAcceptedMove(underWhite, underBlack, underGame, "b7a8n", underState),
          "promoting to a knight is accepted");
    check(underState.board[squareFromName("a8")] == 'N',
          "a white knight stands on a8, so the choice is honoured");

    std::cout << "\n=== random rubbish, in quantity (secs 64, 67) ===\n";

    // The crafted bad messages above cover the cases worth naming. This covers
    // the ones nobody thought of: random bytes, in bulk, from several clients
    // at once. Half of them begin with a plausible type byte so that the
    // Server's per-message parsers are actually reached and fed nonsense,
    // rather than every packet being turned away at the first check.
    //
    // The seed is fixed, so a failure here can be reproduced exactly.
    {
        constexpr int FUZZ_CLIENTS = 4;
        constexpr int PACKETS_EACH = 400;

        std::mt19937 random(12345);
        std::uniform_int_distribution<int> byteValue(0, 255);
        std::uniform_int_distribution<int> lengthValue(0, 250);
        std::uniform_int_distribution<int> typeValue(0, 40);

        std::vector<TestClient> fuzzers(FUZZ_CLIENTS);
        bool allConnected = true;
        for (TestClient& fuzzer : fuzzers) {
            allConnected = allConnected && fuzzer.connect(port);
        }
        check(allConnected, "four clients connect to send rubbish");

        for (int round = 0; round < PACKETS_EACH; ++round) {
            for (TestClient& fuzzer : fuzzers) {
                sf::Packet rubbish;

                if (round % 2 == 0) {
                    rubbish << static_cast<std::uint8_t>(typeValue(random));
                }

                const int length = lengthValue(random);
                for (int b = 0; b < length; ++b) {
                    rubbish << static_cast<std::uint8_t>(byteValue(random));
                }

                fuzzer.send(rubbish);
            }
        }

        // Dropped mid-stream, so the Server also has to cope with a client
        // vanishing while its rubbish is still unread.
        fuzzers[1].disconnect();
        fuzzers[3].disconnect();

        sf::sleep(sf::milliseconds(700));
    }

    TestClient afterWhite;
    TestClient afterBlack;
    const std::int32_t afterGame = startGame(afterWhite, afterBlack, port);
    check(afterGame != 0,
          "after 1600 malformed packets the Server still pairs new clients");

    GameStateMessage afterState;
    check(playAcceptedMove(afterWhite, afterBlack, afterGame, "e2e4", afterState),
          "and still validates and broadcasts a real move");

    std::cout << "\nchecks: " << checkCount
              << ", failures: " << failureCount << "\n";

    return (failureCount == 0) ? 0 : 1;
}
