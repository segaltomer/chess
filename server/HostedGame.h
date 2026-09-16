#pragma once

// Reached by relative path because the documented server build command passes
// no -Icommon, and that command is fixed by the course requirements.
#include "../common/ChessGame.h"
#include "../common/Protocol.h"

#include <string>

// One online game, living on the Server (§22).
//
// This is the authority for that game: it holds the board, whose turn it is and
// the result, and every move is validated here. The two Clients hold no game of
// their own — they draw the states this object produces and send requests back.
// §24 is therefore true by construction rather than by trusting the Clients.
//
// Players are identified by their client id rather than by a socket or an
// index, because ids stay meaningful when a client disconnects and the
// Server's connection list shuffles underneath.
class HostedGame {
public:
    HostedGame(int gameId, int whiteClientId, int blackClientId);

    int id() const;
    int whiteClientId() const;
    int blackClientId() const;

    bool hasPlayer(int clientId) const;

    // The other player's client id, or 0 if the id given is not in this game.
    int opponentOf(int clientId) const;

    // Which colour that client plays. Only meaningful for a player of this game.
    Color colourOf(int clientId) const;

    bool isOver() const;

    // §22: validates the move against the rules and against whose turn it is,
    // and plays it only if it passes. Returns false and fills `rejectionReason`
    // with something a player can read otherwise, changing nothing.
    bool tryMove(int clientId, const Move& move, std::string& rejectionReason);

    // §56: the named player gives up and the other wins.
    void resign(int clientId);

    // §66: the named player's connection has gone, which ends the game for the
    // one still here.
    void endByDisconnection(int clientId);

    // §48-52. An offer stands until it is answered; a second one cannot be made
    // while the first is outstanding, and a player cannot offer to themselves.
    // Returns false if the offer is not allowed right now (§67).
    bool offerDraw(int clientId);
    bool hasPendingDrawOffer() const;

    // The client who made the outstanding offer, or 0 if none stands.
    int drawOfferedBy() const;

    // §52: answers the outstanding offer. Accepting ends the game as a draw.
    // Returns false if this client had no offer to answer (§67).
    bool respondToDrawOffer(int clientId, bool isAccepted);

    // The position as the Clients should draw it (§23).
    GameStateMessage stateMessage() const;

    // The result and the reason, worded by ChessGame so that both Clients and a
    // Local game all say the same thing (§39).
    GameOverMessage overMessage() const;

private:
    int id_ = 0;
    int whiteClientId_ = 0;
    int blackClientId_ = 0;

    ChessGame game_;

    // The client id with an offer outstanding, or 0 for none. Held as an id
    // rather than a colour so that "you cannot answer your own offer" is one
    // comparison (§67).
    int drawOfferedBy_ = 0;
};
