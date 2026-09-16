#include "Protocol.h"

namespace {

// Colours and piece types travel as single bytes rather than as whatever the
// compiler happens to pick for an enum, so the two programs agree regardless of
// how each was built.
std::uint8_t toByte(Color colour) {
    return (colour == Color::White) ? 0 : 1;
}

Color colourFromByte(std::uint8_t raw) {
    return (raw == 0) ? Color::White : Color::Black;
}

std::uint8_t toByte(PieceType type) {
    return static_cast<std::uint8_t>(type);
}

// Anything outside the enum becomes None, so a corrupt byte cannot be turned
// into a piece type that does not exist (§67).
PieceType pieceTypeFromByte(std::uint8_t raw) {
    switch (raw) {
        case static_cast<std::uint8_t>(PieceType::Pawn):   return PieceType::Pawn;
        case static_cast<std::uint8_t>(PieceType::Knight): return PieceType::Knight;
        case static_cast<std::uint8_t>(PieceType::Bishop): return PieceType::Bishop;
        case static_cast<std::uint8_t>(PieceType::Rook):   return PieceType::Rook;
        case static_cast<std::uint8_t>(PieceType::Queen):  return PieceType::Queen;
        case static_cast<std::uint8_t>(PieceType::King):   return PieceType::King;
        default:                                           return PieceType::None;
    }
}

}   // namespace


sf::Packet& operator<<(sf::Packet& packet, MessageType type) {
    return packet << static_cast<std::uint8_t>(type);
}


sf::Packet& operator>>(sf::Packet& packet, MessageType& type) {
    std::uint8_t rawType = 0;
    packet >> rawType;
    type = static_cast<MessageType>(rawType);
    return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const Move& move) {
    return packet << static_cast<std::int32_t>(move.from)
                  << static_cast<std::int32_t>(move.to)
                  << toByte(move.promotion);
}


sf::Packet& operator>>(sf::Packet& packet, Move& move) {
    std::int32_t from = 0;
    std::int32_t to = 0;
    std::uint8_t promotion = 0;

    packet >> from >> to >> promotion;

    move.from = static_cast<int>(from);
    move.to = static_cast<int>(to);
    move.promotion = pieceTypeFromByte(promotion);

    return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const GameMessage& message) {
    return packet << message.gameId;
}


sf::Packet& operator>>(sf::Packet& packet, GameMessage& message) {
    return packet >> message.gameId;
}


sf::Packet& operator<<(sf::Packet& packet, const GameStartedMessage& message) {
    return packet << message.gameId << toByte(message.yourColour)
                  << message.opponentName;
}


sf::Packet& operator>>(sf::Packet& packet, GameStartedMessage& message) {
    std::uint8_t colour = 0;
    packet >> message.gameId >> colour >> message.opponentName;
    message.yourColour = colourFromByte(colour);
    return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const MoveRequestMessage& message) {
    return packet << message.gameId << message.move;
}


sf::Packet& operator>>(sf::Packet& packet, MoveRequestMessage& message) {
    return packet >> message.gameId >> message.move;
}


sf::Packet& operator<<(sf::Packet& packet, const GameStateMessage& message) {
    return packet << message.gameId << message.board
                  << toByte(message.sideToMove) << message.isInCheck;
}


sf::Packet& operator>>(sf::Packet& packet, GameStateMessage& message) {
    std::uint8_t sideToMove = 0;
    packet >> message.gameId >> message.board >> sideToMove >> message.isInCheck;
    message.sideToMove = colourFromByte(sideToMove);
    return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const MoveRejectedMessage& message) {
    return packet << message.gameId << message.reason;
}


sf::Packet& operator>>(sf::Packet& packet, MoveRejectedMessage& message) {
    return packet >> message.gameId >> message.reason;
}


sf::Packet& operator<<(sf::Packet& packet, const DrawResponseMessage& message) {
    return packet << message.gameId << message.isAccepted;
}


sf::Packet& operator>>(sf::Packet& packet, DrawResponseMessage& message) {
    return packet >> message.gameId >> message.isAccepted;
}


sf::Packet& operator<<(sf::Packet& packet, const GameOverMessage& message) {
    return packet << message.gameId << message.title << message.detail;
}


sf::Packet& operator>>(sf::Packet& packet, GameOverMessage& message) {
    return packet >> message.gameId >> message.title >> message.detail;
}


sf::Packet makeMessage(MessageType type) {
    sf::Packet packet;
    packet << type;
    return packet;
}


bool isKnownMessageType(std::uint8_t rawType) {
    switch (static_cast<MessageType>(rawType)) {
        case MessageType::FindGame:
        case MessageType::CancelFindGame:
        case MessageType::MoveRequest:
        case MessageType::OfferDraw:
        case MessageType::DrawResponse:
        case MessageType::Resign:
        case MessageType::Waiting:
        case MessageType::GameStarted:
        case MessageType::GameState:
        case MessageType::MoveRejected:
        case MessageType::DrawOffered:
        case MessageType::DrawDeclined:
        case MessageType::GameOver:
            return true;
    }

    return false;
}


std::string messageTypeName(MessageType type) {
    switch (type) {
        case MessageType::FindGame:       return "FindGame";
        case MessageType::CancelFindGame: return "CancelFindGame";
        case MessageType::MoveRequest:    return "MoveRequest";
        case MessageType::OfferDraw:      return "OfferDraw";
        case MessageType::DrawResponse:   return "DrawResponse";
        case MessageType::Resign:         return "Resign";
        case MessageType::Waiting:        return "Waiting";
        case MessageType::GameStarted:    return "GameStarted";
        case MessageType::GameState:      return "GameState";
        case MessageType::MoveRejected:   return "MoveRejected";
        case MessageType::DrawOffered:    return "DrawOffered";
        case MessageType::DrawDeclined:   return "DrawDeclined";
        case MessageType::GameOver:       return "GameOver";
    }

    return "unknown";
}
