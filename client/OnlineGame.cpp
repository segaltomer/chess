#include "OnlineGame.h"

#include "../common/Protocol.h"

#include <iostream>


OnlineGame::OnlineGame(ServerConnection& connection)
    : connection_(connection) {
    // Nothing is known about any position until the Server sends one, so the
    // board starts genuinely empty rather than showing a game that has not
    // begun.
    board_.fill(EMPTY_DISPLAY_CHAR);
}


OnlineGame::~OnlineGame() {
    if (!connection_.isConnected()) {
        return;
    }

    if (!hasGameStarted_) {
        // §42: stop being a free client, so the Server does not pair somebody
        // with a window that has already closed.
        send(makeMessage(MessageType::CancelFindGame));
        return;
    }

    if (!isOver_) {
        GameMessage message;
        message.gameId = gameId_;
        send(makeMessage(MessageType::Resign, message));
    }
}


void OnlineGame::send(sf::Packet packet) {
    connection_.send(packet);
}


bool OnlineGame::isMyTurn() const {
    return hasGameStarted_ && !isOver_ && sideToMove_ == myColour_;
}


std::array<char, SQUARE_COUNT> OnlineGame::boardForDisplay() const {
    return board_;
}


bool OnlineGame::isOver() const {
    return isOver_;
}


// The GUI's side panel is 260 pixels wide and does not wrap or shorten what it
// is given, so anything longer runs off the edge of the window. Every caption
// below was measured against that width in the GUI's own font and sizes.
std::string OnlineGame::turnText() const {
    if (!hasGameStarted_) {
        return "Finding a game";
    }

    return isMyTurn() ? "Your move" : (opponentName_ + " to move");
}


std::string OnlineGame::statusText() const {
    if (!notice_.empty()) {
        return notice_;
    }

    if (!hasGameStarted_) {
        // How to cancel is explained on the console, where there is room for a
        // whole sentence.
        return "Waiting for opponent";
    }

    // §30 is decided on the Server; this Client only repeats what it was told.
    return isInCheck_ ? "Check" : "";
}


std::string OnlineGame::resultTitle() const {
    return resultTitle_;
}


std::string OnlineGame::resultDetail() const {
    return resultDetail_;
}


std::string OnlineGame::promotingSideName() const {
    return (myColour_ == Color::White) ? "White" : "Black";
}


bool OnlineGame::needsPromotionChoice(int fromSquare, int toSquare) const {
    if (!isMyTurn() || !isValidSquare(fromSquare) || !isValidSquare(toSquare)) {
        return false;
    }

    // Worked out from the picture on screen, which is display logic rather than
    // a rules decision: is the piece being dragged one of my pawns, and is it
    // landing on the far rank? If this Client guessed wrong, nothing is lost —
    // the Server validates the move either way and rejects it if it is not
    // legal (§22).
    const Piece piece = fromDisplayChar(board_[fromSquare]);

    const bool isMyPawn = piece.type == PieceType::Pawn && piece.colour == myColour_;
    const bool reachesFarRank = rowOf(toSquare) == backRankOf(opposite(myColour_));

    return isMyPawn && reachesFarRank;
}


void OnlineGame::applyBoardInputState() {
    // §24: the board only accepts a click when it is genuinely this player's
    // turn and no earlier request is still unanswered.
    gui_.setBoardInputEnabled(isMyTurn() && !isAwaitingServerReply_);
}


void OnlineGame::onGameStarted() {
    gui_.setPlayerNames("Waiting...", "Waiting...");

    // §48: draw offers belong in an online game, but not before there is an
    // opponent to send one to.
    gui_.setOfferDrawVisible(false);
    gui_.setBoardInputEnabled(false);

    // §40: ask to be matched. Everything after this is driven by what comes
    // back from the Server.
    send(makeMessage(MessageType::FindGame));

    std::cout << "\nLooking for an opponent...\n";
    std::cout << "Close the board window to stop searching." << std::endl;
}


void OnlineGame::onMoveChosen(const Move& move) {
    if (!isMyTurn() || isAwaitingServerReply_) {
        return;
    }

    // §23-24: this is a request, not a move. Nothing on the board changes here
    // — the piece stays where it is until the Server sends back a position in
    // which it has moved.
    MoveRequestMessage request;
    request.gameId = gameId_;
    request.move = move;

    send(makeMessage(MessageType::MoveRequest, request));

    isAwaitingServerReply_ = true;
    notice_.clear();

    applyBoardInputState();
}


void OnlineGame::onResignConfirmed() {
    if (!hasGameStarted_ || isOver_) {
        return;
    }

    // §56: the Server decides the result and tells both Clients. This one does
    // not declare itself the loser locally.
    GameMessage message;
    message.gameId = gameId_;

    send(makeMessage(MessageType::Resign, message));
}


void OnlineGame::onDrawOffered() {
    if (!hasGameStarted_ || isOver_) {
        return;
    }

    // §50: the offer travels through the Server, which shows it on the
    // opponent's Client only.
    GameMessage message;
    message.gameId = gameId_;

    send(makeMessage(MessageType::OfferDraw, message));

    notice_ = "Draw offered";
}


void OnlineGame::onDrawResponse(bool isAccepted) {
    if (!hasGameStarted_ || isOver_) {
        return;
    }

    // §51-52: the answer goes back through the Server, which ends the game on
    // both Clients if it was Accept.
    DrawResponseMessage response;
    response.gameId = gameId_;
    response.isAccepted = isAccepted;

    send(makeMessage(MessageType::DrawResponse, response));
}


void OnlineGame::handleGameStarted(sf::Packet& packet) {
    GameStartedMessage message;

    if (!(packet >> message)) {
        return;
    }

    hasGameStarted_ = true;
    gameId_ = message.gameId;
    myColour_ = message.yourColour;
    opponentName_ = message.opponentName;

    const bool iAmWhite = (myColour_ == Color::White);
    const std::string myName = "You";

    gui_.setPlayerNames(iAmWhite ? myName : opponentName_,
                        iAmWhite ? opponentName_ : myName);

    // §48: there is somebody to offer a draw to now.
    gui_.setOfferDrawVisible(true);

    notice_.clear();

    std::cout << "Game " << gameId_ << " started against " << opponentName_ << ".\n";
    std::cout << "You are " << (iAmWhite ? "White" : "Black")
              << ". White moves first." << std::endl;
}


void OnlineGame::handleGameState(sf::Packet& packet) {
    GameStateMessage message;

    if (!(packet >> message)) {
        return;
    }

    // §59 is enforced on the Server, but a Client in one game should still not
    // draw a position belonging to another.
    if (message.gameId != gameId_) {
        return;
    }

    if (message.board.size() != static_cast<std::size_t>(SQUARE_COUNT)) {
        return;
    }

    // §24: the position is replaced wholesale by what the Server sent. This
    // Client never edits it, which is why there is no way for the two to drift.
    for (int square = 0; square < SQUARE_COUNT; ++square) {
        board_[square] = message.board[square];
    }

    sideToMove_ = message.sideToMove;
    isInCheck_ = message.isInCheck;

    isAwaitingServerReply_ = false;
    notice_.clear();
}


void OnlineGame::handleMoveRejected(sf::Packet& packet) {
    MoveRejectedMessage message;

    if (!(packet >> message)) {
        return;
    }

    if (message.gameId != gameId_) {
        return;
    }

    // §24 again: the request was not final, and this is the Server saying so.
    // Nothing on the board has to be put back, because nothing was moved.
    isAwaitingServerReply_ = false;

    // The Server words these to fit the panel, so they are shown as they came
    // rather than with a prefix that would push them off the edge.
    notice_ = message.reason;

    std::cout << "Move rejected: " << message.reason << std::endl;
}


void OnlineGame::handleDrawOffered(sf::Packet& packet) {
    GameMessage message;

    if (!(packet >> message)) {
        return;
    }

    if (message.gameId != gameId_ || isOver_) {
        return;
    }

    // §50-51: this Client is the one being asked, so the dialog belongs here.
    gui_.showDrawOfferDialog(opponentName_ + " offers a draw. Do you accept?");
}


void OnlineGame::handleDrawDeclined() {
    notice_ = "Draw offer declined";
    std::cout << "Your draw offer was declined." << std::endl;
}


void OnlineGame::handleGameOver(sf::Packet& packet) {
    GameOverMessage message;

    if (!(packet >> message)) {
        return;
    }

    if (message.gameId != gameId_) {
        return;
    }

    // §39, §56: the wording is the Server's, so both Clients show the same text.
    isOver_ = true;
    resultTitle_ = message.title;
    resultDetail_ = message.detail;
    notice_.clear();
}


void OnlineGame::handleServerMessage(sf::Packet& packet) {
    std::uint8_t rawType = 0;

    if (!(packet >> rawType) || !isKnownMessageType(rawType)) {
        // §67: anything that is not one of our messages is dropped rather than
        // guessed at.
        return;
    }

    const MessageType type = static_cast<MessageType>(rawType);

    // §67: a message can be perfectly well formed and still be wrong for the
    // moment it arrived in. Everything below the first two only means anything
    // once this Client is in a game, and a second GameStarted would mean the
    // Server thinks it has paired us twice. Neither is acted on.
    const bool isAboutAGameInProgress =
        (type != MessageType::Waiting && type != MessageType::GameStarted);

    if (isAboutAGameInProgress && !hasGameStarted_) {
        return;
    }

    if (type == MessageType::GameStarted && hasGameStarted_) {
        return;
    }

    switch (type) {
        case MessageType::Waiting:
            // §40: no opponent yet. The window already says so, so there is
            // nothing to change.
            return;

        case MessageType::GameStarted:  handleGameStarted(packet); return;
        case MessageType::GameState:    handleGameState(packet); return;
        case MessageType::MoveRejected: handleMoveRejected(packet); return;
        case MessageType::DrawOffered:  handleDrawOffered(packet); return;
        case MessageType::DrawDeclined: handleDrawDeclined(); return;
        case MessageType::GameOver:     handleGameOver(packet); return;

        // §67: these are the Client's own messages coming back at it, which
        // should never happen and is not acted on if it does.
        case MessageType::FindGame:
        case MessageType::CancelFindGame:
        case MessageType::MoveRequest:
        case MessageType::OfferDraw:
        case MessageType::DrawResponse:
        case MessageType::Resign:
            return;
    }
}


void OnlineGame::endBecauseServerLost() {
    if (isOver_) {
        return;
    }

    // §65: the game cannot continue and must not simply hang. It ends on screen
    // with an honest reason rather than a result nobody won.
    isOver_ = true;
    resultTitle_ = "Game ended";
    resultDetail_ = "Lost connection to the server";
}


void OnlineGame::readServerMessages() {
    if (isOver_) {
        return;
    }

    while (true) {
        sf::Packet packet;
        const ServerConnection::ReceiveResult result = connection_.receive(packet);

        if (result == ServerConnection::ReceiveResult::Nothing) {
            return;
        }

        if (result == ServerConnection::ReceiveResult::Disconnected) {
            endBecauseServerLost();
            return;
        }

        handleServerMessage(packet);

        if (isOver_) {
            return;
        }
    }
}


void OnlineGame::onOpponentTurn() {
    readServerMessages();

    refreshDisplay();
    applyBoardInputState();
}
