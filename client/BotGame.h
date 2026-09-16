#pragma once

#include "BotDifficulty.h"
#include "ClientValidatedGame.h"
#include "StockfishEngine.h"

// A game against Stockfish (§14, §18).
//
// Like a Local game, this is played entirely on the Client: our own code owns
// the board, the turn and the result. Stockfish is asked one question only —
// "what would you play here?" — and its answer is then put through exactly the
// same legality check a human's click goes through. §47 requires that split,
// and it is the reason the Bot cannot cheat even if it wanted to.
//
// The engine is passed in rather than owned, because starting it loads a large
// neural network and can fail. Whoever creates the game checks that it started
// before opening a board window (§80).
class BotGame : public ClientValidatedGame {
public:
    BotGame(StockfishEngine& engine, const BotSettings& settings, Color humanColour);

protected:
    void onGameStarted() override;
    void onOpponentTurn() override;
    Color resigningColour() const override;

private:
    StockfishEngine& engine_;
    BotSettings settings_;
    Color humanColour_;
    Color botColour_;

    // Set if the engine stops answering or suggests something illegal. The
    // game then stays on screen so the player can read the result, but the Bot
    // is not asked again.
    bool hasEngineFailed_ = false;

    std::string botDisplayName() const;
};
