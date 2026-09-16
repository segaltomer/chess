#include "MainMenu.h"

#include "ConsoleInput.h"

#include <iostream>


bool showMainMenu(MainMenuChoice& choice) {
    std::cout << "\nMain menu\n";
    std::cout << "  1) Local Game      - two players at this computer\n";
    std::cout << "  2) Player vs Bot   - play against Stockfish\n";
    std::cout << "  3) Online Game     - find an opponent through the server\n";
    std::cout << "  4) Exit\n";

    int selection = 0;
    if (!ConsoleInput::readNumberInRange("Choice: ", 1, 4, selection)) {
        return false;
    }

    switch (selection) {
        case 1:  choice = MainMenuChoice::LocalGame;   break;
        case 2:  choice = MainMenuChoice::PlayerVsBot; break;
        case 3:  choice = MainMenuChoice::OnlineGame;  break;
        default: choice = MainMenuChoice::Exit;        break;
    }

    return true;
}
