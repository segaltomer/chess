#include "HostedGame.h"

namespace {

// No client is ever given this id, so it is safe as "nobody".
constexpr int NO_CLIENT = 0;

}   // namespace


HostedGame::HostedGame(int gameId, int whiteClientId, int blackClientId)
    : id_(gameId),
      whiteClientId_(whiteClientId),
      blackClientId_(blackClientId) {
}


int HostedGame::id() const {
    return id_;
}


int HostedGame::whiteClientId() const {
    return whiteClientId_;
}


int HostedGame::blackClientId() const {
    return blackClientId_;
}


bool HostedGame::hasPlayer(int clientId) const {
    return clientId == whiteClientId_ || clientId == blackClientId_;
}


int HostedGame::opponentOf(int clientId) const {
    if (clientId == whiteClientId_) {
        return blackClientId_;
    }

    if (clientId == blackClientId_) {
        return whiteClientId_;
    }

    return NO_CLIENT;
}


Color HostedGame::colourOf(int clientId) const {
    return (clientId == whiteClientId_) ? Color::White : Color::Black;
}


bool HostedGame::isOver() const {
    return game_.isOver();
}


bool HostedGame::tryMove(int clientId, const Move& move, std::string& rejectionReason) {
    if (game_.isOver()) {
        rejectionReason = "the game has already finished";
        return false;
    }

    if (!hasPlayer(clientId)) {
        rejectionReason = "you are not a player in this game";
        return false;
    }

    // §29 checked here rather than inside ChessGame, because ChessGame knows
    // which colour may move but not which person is allowed to move it.
    if (colourOf(clientId) != game_.sideToMove()) {
        rejectionReason = "it is not your turn";
        return false;
    }

    // §22: the rules decide, and they decide here on the Server. A refusal
    // changes nothing at all, so a rejected request cannot leave the position
    // half-updated.
    if (!game_.makeMove(move)) {
        rejectionReason = "that move is not legal";
        return false;
    }

    // A move answers an outstanding offer by ignoring it: play has continued,
    // so the offer is no longer on the table.
    drawOfferedBy_ = NO_CLIENT;

    return true;
}


void HostedGame::resign(int clientId) {
    if (!hasPlayer(clientId)) {
        return;
    }

    game_.resign(colourOf(clientId));
}


void HostedGame::endByDisconnection(int clientId) {
    if (!hasPlayer(clientId)) {
        return;
    }

    game_.endByDisconnection(colourOf(clientId));
}


bool HostedGame::offerDraw(int clientId) {
    const bool offerIsAllowed = hasPlayer(clientId)
                             && !game_.isOver()
                             && drawOfferedBy_ == NO_CLIENT;
    if (!offerIsAllowed) {
        return false;
    }

    drawOfferedBy_ = clientId;
    return true;
}


bool HostedGame::hasPendingDrawOffer() const {
    return drawOfferedBy_ != NO_CLIENT;
}


int HostedGame::drawOfferedBy() const {
    return drawOfferedBy_;
}


bool HostedGame::respondToDrawOffer(int clientId, bool isAccepted) {
    // Only the player who was *offered* the draw may answer it — which means
    // there must be an offer, and it must not be this client's own (§67).
    const bool mayAnswer = hasPlayer(clientId)
                        && !game_.isOver()
                        && drawOfferedBy_ != NO_CLIENT
                        && drawOfferedBy_ != clientId;
    if (!mayAnswer) {
        return false;
    }

    drawOfferedBy_ = NO_CLIENT;

    // §52: accepting ends the game as a draw, declining just clears the offer.
    if (isAccepted) {
        game_.agreeDraw();
    }

    return true;
}


GameStateMessage HostedGame::stateMessage() const {
    GameStateMessage message;
    message.gameId = id_;

    const std::array<char, SQUARE_COUNT> squares = game_.board().toDisplay();
    message.board.assign(squares.begin(), squares.end());

    message.sideToMove = game_.sideToMove();
    message.isInCheck = game_.isInCheck();

    return message;
}


GameOverMessage HostedGame::overMessage() const {
    GameOverMessage message;
    message.gameId = id_;
    message.title = game_.resultTitle();
    message.detail = game_.resultDetail();

    return message;
}
