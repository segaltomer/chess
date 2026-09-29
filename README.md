# C++ Chess — Client and Server

A chess system in two separate programs, as the project requires: `client.exe`
and `server.exe`. Each compiles and runs independently of the other.

The Client offers four things: a **Local Game** between two people at one
keyboard, a **Player vs Bot** game against Stockfish at one of three difficulty
levels, an **Online Game** against another Client through the Server, and Exit.

Both executables are included and ready to run, so nothing has to be compiled to
try it. If you would rather build it yourself, the commands are below.

---

## 1. What you need

- **MSYS2 UCRT64 / MinGW-w64**, installed at `C:\msys64`
- **g++ 16.2.0** — `mingw-w64-ucrt-x86_64-gcc 16.2.0-3`
- **SFML 3.0.2** — `mingw-w64-ucrt-x86_64-sfml 3.0.2-3`
- C++17

To install the toolchain, in the **MSYS2 UCRT64** shell:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sfml
```

Nothing else is needed. Stockfish is included; see section 10.

## 2. Compiling

Run both commands from the project root, the folder that holds this README.

**Client:**

```bash
g++ -std=c++17 common/*.cpp client/*.cpp ProvidedGUI/ChessFrontend.cpp -IProvidedGUI -o client.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
```

**Server:**

```bash
g++ -std=c++17 common/*.cpp server/*.cpp -o server.exe -lsfml-network -lsfml-system
```

Each is a single line, so it can be pasted into the MSYS2 UCRT64 shell, `cmd`
or PowerShell without change. The `*.cpp` wildcards are expanded by g++ itself
on MinGW, so they work in all three even where the shell does not expand them.
In `cmd` or PowerShell, `C:\msys64\ucrt64\bin` has to be on `PATH` for `g++` to
be found; the MSYS2 UCRT64 shell has it already.

`common/` holds the chess rules and the protocol messages, and is compiled into
both programs — the rules have to be identical in every mode, and the Server
validates online games while the Client validates local and Bot games. The
Server links no `sfml-graphics`, since it shows no window.

Both build without warnings under `-Wall -Wextra`.

## 3. Before you run: two things that matter

**Run the executables from the project root.** The provided GUI looks for the
piece images relative to the current working directory. Started from anywhere
else, the board draws lettered placeholder squares instead of pieces.
Double-clicking `client.exe` or `server.exe` in Explorer is fine too, because
Windows then starts each one in its own folder.

**The 19 runtime DLLs must sit beside the executables.** They are included in
this folder. Alternatively, run from a shell with `C:\msys64\ucrt64\bin` on
`PATH`, such as the MSYS2 UCRT64 shell.

If a DLL is missing, Windows does not start the program at all. Depending on how
it was launched, it either shows a dialog naming the missing DLL or ends the
process at once with code `0xC0000135` and **no message at all**. If either
program appears to do nothing when launched, that is why.

The included DLLs match the included executables, built with the versions in
section 1. Windows loads a DLL from the executable's own folder before it looks
on `PATH`, so if you rebuild with a different MSYS2 version, copy the
same-named DLLs from your `C:\msys64\ucrt64\bin` over these. Otherwise the new
build may refuse to start with an "entry point not found" error.

## 4. Running the Server

```bash
./server.exe          # listens on the default port, 53000
./server.exe 53001    # or any port from 1024 to 65535
```

The `./` is for the MSYS2 shell and PowerShell. In `cmd`, leave it out and type
`server.exe`; the same applies to every command below.

On startup it prints the port and **the two addresses a Client can use**:

```text
Chess server is listening on port 53000.
Enter these in the Client's connection screen:
  a Client on this machine:   127.0.0.1   port 53000
  a Client on another machine: 192.168.1.20   port 53000
```

Use `127.0.0.1` for a Client on the same computer, and the second address for a
Client on another computer on the same network.

If it reports that it cannot listen on the port, pick a different one. Besides
another program already holding it, Windows reserves whole ranges of ports for
its own use, and a server cannot bind inside one; `netsh int ipv4 show
excludedportrange protocol=tcp` lists them. The default 53000 is outside them on
a normal machine.

It then runs until you close the window, logging every connection and
disconnection, every entry to and exit from the matchmaking queue, every game
created and who was assigned which colour, every move accepted or rejected and
why, every draw offer and answer, every resignation, and every game ending.

## 5. Running a Client and connecting

```bash
./client.exe
```

It asks for the Server's address and then the port — the two values the Server
printed. **The main menu appears only after the connection succeeds.** An
address or port that is not valid is explained and asked for again. A Server
that cannot be reached produces a clear message and the choice to try again,
change the address, change the port, or exit.

Once connected:

```text
Main menu
  1) Local Game      - two players at this computer
  2) Player vs Bot   - play against Stockfish
  3) Online Game     - find an opponent through the server
  4) Exit
```

In every game, a move is made by clicking the piece and then the destination
square. The side panel shows whose turn it is and the game status, and holds the
Offer Draw and Resign buttons.

## 6. Local Game (menu option 1)

Choose **1**. The board opens immediately with two players at this one
keyboard — White moves first, then Black, on the same screen. The Server is not
involved at all; this Client owns the position and checks every move itself.

- **Offer Draw** shows the offer to the other player on this same window, who
  can Accept or Decline. Accepting ends the game as a draw.
- **Resign** asks for confirmation first, and resigns for whoever is to move.
- Closing the board window returns to the menu.

## 7. Player vs Bot (menu option 2)

Choose **2**. Before the game starts you are asked for a difficulty:

```text
Choose the Bot's difficulty
  1) Easy     - blunders often, good for learning
  2) Medium   - plays sensibly, misses some tactics
  3) Hard     - full strength, very difficult to beat
```

The game begins as soon as a level is chosen. You play White; the Bot replies
after each of your moves, and the status line reads "Bot is thinking..." while
it does. Offer Draw is hidden in this mode, as there is nobody to consider an
offer. Resign works as it does everywhere else.

The three levels differ in Stockfish's own Skill Level (0, 8 and 20) and in how
long it may search (100 ms, 400 ms and 1000 ms). Skill Level is the setting that
matters: below 20 Stockfish deliberately plays moves it has judged inferior, so
a low level blunders the way a weak human does. Section 11 shows how to verify
the difference rather than take it on trust.

## 8. Online Game (menu option 3)

Online play needs the Server and **two** Clients. Start the Server, then run
`client.exe` twice — either twice on this computer, or once here and once on
another computer on the same network, using the LAN address the Server printed.
The first time `server.exe` runs, Windows Firewall may ask whether to allow it on
the network. Allow it on private networks, or a Client on another computer will
not be able to connect.

In both Clients, connect and choose **3**.

The first Client to ask opens its board showing "Waiting for opponent".
When the second Client asks, the **Server pairs them**, creates the game,
assigns one White and one Black, and tells both that it has started. Closing the
window while still waiting cancels the search.

The Server owns the game. A click sends it a request and nothing on the board
changes; the piece moves only when the Server sends back a position in which it
has moved. So a move that is not legal simply does nothing on the board: the
Server refuses it, and the reason appears in the status panel and on the
Server's console. While it is the opponent's turn the board does not accept
clicks at all.

- **Offer Draw** travels through the Server and appears only on the opponent's
  window. Accepting ends the game as a draw on both; declining is reported back
  to whoever offered.
- **Resign** asks for confirmation on the resigning player's window only, and
  the Server then tells both Clients the result.
- Closing a board window during a game concedes it, and the opponent is told.
- If a Client disconnects, the other is told the game has ended because the
  opponent disconnected — not that they resigned.

## 9. Several Clients and several games at once

The Server supports any number of connected Clients and any number of
independent games running at the same time. To see it:

1. Start one Server.
2. Start **four** Clients and connect all of them.
3. Choose Online Game in the first two — they are paired into game 1.
4. Choose Online Game in the other two — they are paired into game 2.

Both games can then be played simultaneously. The Server's console shows each
game by number, and every move it logs names the game it belongs to. Nothing
done in one game reaches the other: each is validated separately, and an update
is only ever sent to the two Clients playing that game. A Client disconnecting
ends its own game and leaves the other running.

## 10. How Stockfish is integrated

Stockfish is **a separate program, not a library**. It is included as
`stockfish/stockfish.exe` (Stockfish 19, release tag `sf_19`, Windows x86-64
universal build), together with its GPL licence in `stockfish/Copying.txt`,
which is required when redistributing the binary.

When a Bot game starts, the Client launches that executable as a **child
process** with two pipes — one to its standard input, one from its standard
output — and talks to it in **UCI**, the plain-text protocol chess engines use.
The Client sends the position and a time limit, and Stockfish answers with a
line such as `bestmove e2e4`. Nothing is shared but those two pipes. The path is
resolved relative to the running executable, never as an absolute path, so the
project works from any folder.

Stockfish **chooses the Bot's move and nothing else.** Its answer is a string,
which this project converts to a move and puts through exactly the same legality
check a human's mouse click goes through. Move legality, whose turn it is, and
the result of the game all stay in this project's own code. The engine process
is started when the game begins and shut down when its board window is closed,
so it never outlives the Client.

You can confirm it really is a separate process: start a Bot game and look in
Task Manager's **Details** tab, where `stockfish.exe` appears while the game is
running and disappears once its board window is closed.

## 11. Verifying it (optional)

Three test programs are included under `tests/`. None of them is part of the
Client or the Server — each has its own `main()` and none is picked up by the
build commands in section 2.

**The chess rules**, checked with perft, which plays every legal move to a given
depth and counts the positions reached. The counts for the standard test
positions are published and agreed on across chess engines, so matching them is
real evidence rather than a self-written test passing:

```bash
g++ -std=c++17 -O2 -Icommon common/*.cpp tests/perft_main.cpp -o perft.exe -lsfml-network -lsfml-system
./perft.exe
```

(`common/` holds the protocol messages as well as the rules, so anything built
from `common/*.cpp` links SFML Network even when, as here, it opens no socket.)

All 26 cases pass, about 16.5 million positions, in roughly 15 seconds.

**The Bot's difficulty levels**, checked by making them play each other, so the
difference is measured rather than judged by eye:

```bash
g++ -std=c++17 -O2 -Icommon -Iclient common/*.cpp client/StockfishEngine.cpp client/BotDifficulty.cpp client/ConsoleInput.cpp tests/bot_match.cpp -o bot_match.exe -lsfml-network -lsfml-system
./bot_match.exe
```

Each pairing is played from both colours, so a win cannot be explained by moving
first. Every move is a real engine search, so the six games take three to four
minutes. In every run so far the stronger level has won all six by checkmate;
the weaker levels choose partly at random, so the margins vary from run to run.

**The Server and the protocol**, checked by connecting as several Clients at
once — which is how the things that cannot be seen by playing one game get
tested: two independent games, a Client refused when it asks about another
Client's game, moves rejected by the Server, disconnections, and malformed
messages. It needs a Server to talk to:

```bash
g++ -std=c++17 -O2 -Icommon common/*.cpp tests/protocol_test.cpp -o protocol_test.exe -lsfml-network -lsfml-system
./server.exe 53100            # one window
./protocol_test.exe 53100     # another
```

103 checks; it exits 0 only if every one of them passed. As well as the cases
above it plays a game to checkmate and promotes a pawn through the Server —
including an under-promotion to a knight — so §32 and §36 are covered over the
network and not only in a local game. It finishes by sending 1,600 random
malformed packets from four Clients at once, dropping two of them mid-stream,
and then checking that the Server still pairs new Clients and still validates a
real move.

## 12. Folder layout

```text
.
├── client.exe            the Client
├── server.exe            the Server
├── *.dll                 the 19 runtime DLLs
├── common/               chess rules and protocol, compiled into both programs
├── client/               console screens, GUI wiring, Bot, client networking
├── server/               listener, matchmaking, hosted games
├── tests/                the three verification programs
├── ProvidedGUI/          the supplied GUI, unmodified
│   ├── ChessFrontend.hpp
│   ├── ChessFrontend.cpp
│   └── assets/           12 piece images
├── stockfish/            Stockfish 19 engine, its licence and authors
└── README.md
```

`ProvidedGUI/` is exactly as supplied and has not been modified.

SHA-256 of `stockfish/stockfish.exe`:
`45bc8e4969147db9c2eb533810637994619bff0eacc81ccfd9854394901bcbd0`
