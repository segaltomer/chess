#pragma once

#include <string>

// Reading from the console, with the checking that §67 asks for.
//
// Every function here returns false to mean "the input stream has ended" —
// the console was closed, or input was piped in from a file that has run out.
// That case has to be distinguishable from bad input, because bad input should
// be re-prompted while an ended stream must not be: a loop that keeps asking a
// stream that can never answer again spins forever.
namespace ConsoleInput {

// Prompts, then reads one line with the surrounding blanks removed.
bool readLine(const std::string& prompt, std::string& line);

// Reads a whole number between lowest and highest inclusive, explaining the
// problem and asking again for anything else — empty input, letters, a number
// out of range, or trailing rubbish after a number.
bool readNumberInRange(const std::string& prompt, int lowest, int highest, int& value);

}   // namespace ConsoleInput
