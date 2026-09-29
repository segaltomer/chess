#pragma once

#include "Game.h"
#include "ServerConnection.h"

#include <array>
#include <string>

// A game against another player, hosted on the Server (§15, §21-25).
//
// Note what this class does not have: a `ChessGame`. It cannot check a move,
// decide a result, or work out whose turn it is, because on the Server's side
// of §22 none of that is its business. All it holds is the last position the
// Server sent, in a plain array of characters that it never edits — only
// replaces wholesale when a new `GameState` message arrives.
//
// That is how §24 is kept. A click does not move a piece here; it sends a
// `MoveRequest` and nothing on screen changes. The piece appears to move only
// when the Server sends back a position in which it has moved. There is no code
// path in this class that could apply a move locally, which is a stronger
// guarantee than remembering not to write one — and it is why the class
// inherits `Game` rather than `ClientValidatedGame`.
//
// The board window opens as soon as the player chooses Online, before anyone
// has been found to play against, and shows "Waiting for opponent". That
// is what gives §42 its cancel: closing the window while still waiting sends
// `CancelFindGame`, with no need for the console to watch the keyboard while
// the window is up.
class OnlineGame : public Game {
public:
    explicit OnlineGame(ServerConnection& connection);

    // §42, §53: leaving tells the Server. While still searching that is a
    // cancelled search; once a game is under way it is a resignation, because
    // walking away from a game in progress concedes it — the alternative is an
    // opponent left waiting for a move that will never come.
    ~OnlineGame() override;

protected:
    std::array<char, SQUARE_COUNT> boardForDisplay() const override;
    bool isOver() const override;
    std::string turnText() const override;
    std::string statusText() const override;
    std::string resultTitle() const override;
    std::string resultDetail() const override;
    std::string promotingSideName() const override;

    bool needsPromotionChoice(int fromSquare, int toSquare) const override;

    void onGameStarted() override;
    void onOpponentTurn() override;
    void onMoveChosen(const Move& move) override;
    void onResignConfirmed() override;
    void onDrawOffered() override;
    void onDrawResponse(bool isAccepted) override;

private:
    ServerConnection& connection_;

    // Nothing here is decided by this Client. Every field is either set from a
    // Server message or is local display bookkeeping.
    bool hasGameStarted_ = false;
    bool isOver_ = false;
    std::int32_t gameId_ = 0;
    Color myColour_ = Color::White;
    std::string opponentName_;

    std::array<char, SQUARE_COUNT> board_;
    Color sideToMove_ = Color::White;
    bool isInCheck_ = false;

    std::string resultTitle_;
    std::string resultDetail_;

    // §24: set the moment a move request goes out and cleared when the Server
    // answers. While it is set the board is frozen, so a player cannot fire off
    // a second request in the belief that the first one has already happened.
    bool isAwaitingServerReply_ = false;

    // Shown in the status line until the position next changes — a rejection
    // reason, or a note that a draw offer was declined.
    std::string notice_;

    bool isMyTurn() const;

    // Taken by value, unusually, because SFML's own send takes a non-const
    // reference and reads the packet as it goes. Callers build a message inline
    // and pass the temporary straight in, which a reference parameter cannot
    // bind to, and a sent packet is not wanted afterwards anyway.
    void send(sf::Packet packet);

    // Reads everything the Server has sent since the last frame. One frame can
    // bring several messages — GameStarted and the opening position arrive
    // together — so this drains the socket rather than reading a single one.
    void readServerMessages();

    void handleServerMessage(sf::Packet& packet);
    void handleGameStarted(sf::Packet& packet);
    void handleGameState(sf::Packet& packet);
    void handleMoveRejected(sf::Packet& packet);
    void handleDrawOffered(sf::Packet& packet);
    void handleDrawDeclined();
    void handleGameOver(sf::Packet& packet);

    // §65: the Server going away has to end the game on screen, not hang it.
    void endBecauseServerLost();

    void applyBoardInputState();
};
