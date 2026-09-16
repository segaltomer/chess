#include "ConsoleInput.h"

#include <cctype>
#include <iostream>
#include <string>

namespace {

std::string trimmed(const std::string& text) {
    const std::string blanks = " \t\r\n";

    const std::size_t first = text.find_first_not_of(blanks);
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = text.find_last_not_of(blanks);

    return text.substr(first, last - first + 1);
}

// Accepts a whole number and nothing else. std::stoi alone is not enough: it
// happily reads "53000abc" as 53000 and ignores the rest, which would let a
// typo through as a valid port.
bool parseWholeNumber(const std::string& text, int& value) {
    if (text.empty()) {
        return false;
    }

    std::size_t charactersUsed = 0;
    int parsed = 0;

    try {
        parsed = std::stoi(text, &charactersUsed);
    } catch (const std::exception&) {
        return false;   // not a number at all, or too large for an int
    }

    if (charactersUsed != text.size()) {
        return false;
    }

    value = parsed;

    return true;
}

}   // namespace


bool ConsoleInput::readLine(const std::string& prompt, std::string& line) {
    std::cout << prompt;
    std::cout.flush();

    std::string input;
    if (!std::getline(std::cin, input)) {
        std::cout << "\n";
        return false;
    }

    line = trimmed(input);

    return true;
}


bool ConsoleInput::readNumberInRange(const std::string& prompt, int lowest, int highest,
                                     int& value) {
    while (true) {
        std::string line;
        if (!readLine(prompt, line)) {
            return false;
        }

        int parsed = 0;
        if (!parseWholeNumber(line, parsed)) {
            std::cout << "  Please type a whole number between " << lowest
                      << " and " << highest << ".\n";
            continue;
        }

        const bool isInRange = (parsed >= lowest && parsed <= highest);
        if (!isInRange) {
            std::cout << "  " << parsed << " is outside the range " << lowest
                      << " to " << highest << ".\n";
            continue;
        }

        value = parsed;

        return true;
    }
}
