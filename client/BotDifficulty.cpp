#include "BotDifficulty.h"

#include "ConsoleInput.h"

#include <iostream>

namespace {

// Skill Level 0 is the setting that makes Easy genuinely beatable: Stockfish
// then chooses moves it has itself judged inferior, so it hangs pieces and
// misses simple threats. Raising the time limit alone would not do that — a
// full-strength engine given a tenth of a second still plays very well.
constexpr int EASY_SKILL_LEVEL = 0;
constexpr int EASY_THINKING_MS = 100;

constexpr int MEDIUM_SKILL_LEVEL = 8;
constexpr int MEDIUM_THINKING_MS = 400;

constexpr int HARD_SKILL_LEVEL = 20;
constexpr int HARD_THINKING_MS = 1000;

}   // namespace


BotSettings settingsFor(BotDifficulty difficulty) {
    switch (difficulty) {
        case BotDifficulty::Easy:
            return BotSettings{EASY_SKILL_LEVEL, EASY_THINKING_MS, "Easy"};

        case BotDifficulty::Medium:
            return BotSettings{MEDIUM_SKILL_LEVEL, MEDIUM_THINKING_MS, "Medium"};

        case BotDifficulty::Hard:
            break;
    }

    return BotSettings{HARD_SKILL_LEVEL, HARD_THINKING_MS, "Hard"};
}


bool chooseBotDifficulty(BotDifficulty& difficulty) {
    std::cout << "\nChoose the Bot's difficulty\n";
    std::cout << "  1) Easy     - blunders often, good for learning\n";
    std::cout << "  2) Medium   - plays sensibly, misses some tactics\n";
    std::cout << "  3) Hard     - full strength, very difficult to beat\n";

    int selection = 0;
    if (!ConsoleInput::readNumberInRange("Choice: ", 1, 3, selection)) {
        return false;
    }

    switch (selection) {
        case 1:  difficulty = BotDifficulty::Easy;   break;
        case 2:  difficulty = BotDifficulty::Medium; break;
        default: difficulty = BotDifficulty::Hard;   break;
    }

    return true;
}
