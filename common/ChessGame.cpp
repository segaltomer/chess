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

    return game;
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


std::vector<Move> ChessGame::legalMovesFrom(int fromSquare) const {
    const std::vector<Move> all = legalMoves();

    std::vector<Move> fromSquareMoves;
    std::copy_if(all.begin(), all.end(), std::back_inserter(fromSquareMoves),
                 [fromSquare](const Move& move) { return move.from == fromSquare; });

    return fromSquareMoves;
}


bool ChessGame::isLegal(const Move& move) const {
    return Rules::isLegalMove(board_, sideToMove_, move);
}


bool ChessGame::makeMove(const Move& move) {
    if (!isLegal(move)) {
        return false;
    }

    board_.applyMove(move);
    sideToMove_ = opposite(sideToMove_);

    return true;
}
