#pragma once

#include <string>

// §43: the three levels the player picks from before a Bot game starts.
enum class BotDifficulty { Easy, Medium, Hard };

// What a level actually means, in the two numbers Stockfish is given.
//
// §19-20 ask for levels that differ significantly and demonstrably, and these
// are two independent ways of weakening an engine. `skillLevel` is Stockfish's
// own 0-20 scale: below 20 it deliberately picks moves it knows are not the
// best, so a low setting blunders the way a weak human does rather than merely
// playing slower. `thinkingTimeMs` then limits how deeply it looks.
//
// DESIGN decision 6 keeps this as data rather than a class per level: the
// levels differ only in these values, not in behaviour, so classes would be
// decoration. §73 asks for polymorphism where it suits the design, and here it
// does not.
struct BotSettings {
    int skillLevel = 20;
    int thinkingTimeMs = 1000;
    std::string name = "Hard";
};

BotSettings settingsFor(BotDifficulty difficulty);

// Asks the player to choose (§43). Returns false if the console input ended,
// which the caller must treat as "go back" rather than asking again.
bool chooseBotDifficulty(BotDifficulty& difficulty);
