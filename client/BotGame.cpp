#include "BotGame.h"

#include <iostream>


BotGame::BotGame(StockfishEngine& engine, const BotSettings& settings, Color humanColour)
    : engine_(engine),
      settings_(settings),
      humanColour_(humanColour),
      botColour_(opposite(humanColour)) {
}


std::string BotGame::botDisplayName() const {
    return "Bot (" + settings_.name + ")";
}


void BotGame::onGameStarted() {
    const bool humanIsWhite = (humanColour_ == Color::White);

    const std::string whiteName = humanIsWhite ? std::string("You") : botDisplayName();
    const std::string blackName = humanIsWhite ? botDisplayName() : std::string("You");

    gui_.setPlayerNames(whiteName, blackName);

    // §48 allows draw offers in Local and Online games only. There is nobody
    // to consider an offer here, so the button is hidden rather than left to
    // do nothing when clicked.
    gui_.setOfferDrawVisible(false);

    engine_.startNewGame();
    engine_.applyDifficulty(settings_.skillLevel, settings_.thinkingTimeMs);

    std::cout << "\nBot game started against " << botDisplayName() << ".\n";
    std::cout << "You are " << (humanIsWhite ? "White" : "Black")
              << ". White moves first.\n";
    std::cout << "Offer Draw is hidden in Bot games." << std::endl;
}


Color BotGame::resigningColour() const {
    // §53: a human may resign. There is only one human here, so the Resign
    // button always belongs to them regardless of whose turn it is.
    return humanColour_;
}


void BotGame::onOpponentTurn() {
    const bool botHasNothingToDo = game_.isOver()
                                || hasEngineFailed_
                                || game_.sideToMove() != botColour_;
    if (botHasNothingToDo) {
        return;
    }

    // The engine blocks while it searches, so say what is happening and stop
    // the player dragging the Bot's pieces in the meantime.
    gui_.setBoardInputEnabled(false);
    gui_.setStatusText("Bot is thinking...");
    gui_.render();

    const std::string suggestion = engine_.chooseMove(game_.toFen());

    gui_.setBoardInputEnabled(true);

    if (suggestion.empty()) {
        hasEngineFailed_ = true;
        std::cout << "The Bot could not choose a move: " << engine_.lastError() << "\n";
        std::cout << "Resign or close the window to end the game." << std::endl;
        return;
    }

    // §47: Stockfish only suggests. The move goes through exactly the same
    // legality check a human's click does, so a bad answer is refused here
    // rather than quietly corrupting the position.
    const Move botMove = Move::fromString(suggestion);

    if (!game_.makeMove(botMove)) {
        hasEngineFailed_ = true;
        std::cout << "The Bot suggested '" << suggestion
                  << "', which is not legal in this position. It has been refused.\n";
        std::cout << "Resign or close the window to end the game." << std::endl;
    }
}
