#include "ChessGame.h"

#include "Rules.h"

#include <algorithm>
#include <sstream>


ChessGame ChessGame::fromFen(const std::string& fen) {
    std::istringstream fields(fen);

    std::string placement;
    std::string sideToMoveField;
    std::string castlingField;
    std::string enPassantField;

    fields >> placement >> sideToMoveField >> castlingField >> enPassantField;

    ChessGame game;
    game.board_ = Board::fromFen(placement, castlingField, enPassantField);
    game.sideToMove_ = (sideToMoveField == "b") ? Color::Black : Color::White;

    // The position loaded may already be a checkmate or a stalemate.
    game.updateResultFromPosition();

    return game;
}


std::string ChessGame::toFen() const {
    const std::string sideToMoveField = (sideToMove_ == Color::White) ? "w" : "b";

    return board_.placementFen()
         + " " + sideToMoveField
         + " " + board_.castlingFen()
         + " " + board_.enPassantFen()
         + " 0 1";
}


const Board& ChessGame::board() const {
    return board_;
}


Color ChessGame::sideToMove() const {
    return sideToMove_;
}


bool ChessGame::isInCheck() const {
    return Rules::isInCheck(board_, sideToMove_);
}


bool ChessGame::isCheckmate() const {
    return Rules::isCheckmate(board_, sideToMove_);
}


bool ChessGame::isStalemate() const {
    return Rules::isStalemate(board_, sideToMove_);
}


std::vector<Move> ChessGame::legalMoves() const {
    return Rules::legalMoves(board_, sideToMove_);
}


bool ChessGame::isLegal(const Move& move) const {
    return Rules::isLegalMove(board_, sideToMove_, move);
}


bool ChessGame::needsPromotionChoice(int fromSquare, int toSquare) const {
    const std::vector<Move> moves = legalMoves();

    // The generator already emits a promoting move once per piece it could
    // become, so the question is simply whether any of them carries a choice.
    return std::any_of(moves.begin(), moves.end(), [fromSquare, toSquare](const Move& move) {
        const bool isSameTravel = (move.from == fromSquare && move.to == toSquare);

        return isSameTravel && move.promotion != PieceType::None;
    });
}


bool ChessGame::makeMove(const Move& move) {
    if (isOver() || !isLegal(move)) {
        return false;
    }

    board_.applyMove(move);
    sideToMove_ = opposite(sideToMove_);

    updateResultFromPosition();

    return true;
}


void ChessGame::updateResultFromPosition() {
    if (Rules::isCheckmate(board_, sideToMove_)) {
        // The side that cannot move is the side that has been mated, so the
        // other one has won.
        const bool whiteIsMated = (sideToMove_ == Color::White);

        result_ = whiteIsMated ? GameResult::BlackWins : GameResult::WhiteWins;
        endReason_ = GameEndReason::Checkmate;
        return;
    }

    if (Rules::isStalemate(board_, sideToMove_)) {
        result_ = GameResult::Draw;
        endReason_ = GameEndReason::Stalemate;
    }
}


void ChessGame::resign(Color resigningColour) {
    if (isOver()) {
        return;
    }

    const bool whiteResigned = (resigningColour == Color::White);

    result_ = whiteResigned ? GameResult::BlackWins : GameResult::WhiteWins;
    endReason_ = GameEndReason::Resignation;
}


void ChessGame::agreeDraw() {
    if (isOver()) {
        return;
    }

    result_ = GameResult::Draw;
    endReason_ = GameEndReason::DrawAgreed;
}


void ChessGame::endByDisconnection(Color disconnectedColour) {
    if (isOver()) {
        return;
    }

    const bool whiteDisconnected = (disconnectedColour == Color::White);

    result_ = whiteDisconnected ? GameResult::BlackWins : GameResult::WhiteWins;
    endReason_ = GameEndReason::Disconnection;
}


bool ChessGame::isOver() const {
    return result_ != GameResult::InProgress;
}


GameResult ChessGame::result() const {
    return result_;
}


GameEndReason ChessGame::endReason() const {
    return endReason_;
}


std::string ChessGame::resultTitle() const {
    switch (result_) {
        case GameResult::WhiteWins:  return "White wins!";
        case GameResult::BlackWins:  return "Black wins!";
        case GameResult::Draw:       return "Draw";
        case GameResult::InProgress: break;
    }

    return "";
}


std::string ChessGame::resultDetail() const {
    switch (endReason_) {
        case GameEndReason::Checkmate:  return "Checkmate";
        case GameEndReason::Stalemate:  return "Stalemate";
        case GameEndReason::DrawAgreed: return "Draw agreed";

        case GameEndReason::Resignation: {
            // Whoever resigned is whoever lost, so the result says which.
            const bool whiteResigned = (result_ == GameResult::BlackWins);

            return whiteResigned ? "White resigned" : "Black resigned";
        }

        case GameEndReason::Disconnection: {
            const bool whiteDisconnected = (result_ == GameResult::BlackWins);

            return whiteDisconnected ? "White disconnected" : "Black disconnected";
        }

        case GameEndReason::None: break;
    }

    return "";
}
