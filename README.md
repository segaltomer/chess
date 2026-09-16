# C++ Chess — Client / Server

Final project for the C++ course. **Work in progress.** The full chess rules
engine is implemented and verified in `common/`, and `server/` currently listens
for connections and tracks them. The Client, the Bot and the online protocol are
not written yet.

This file is rewritten for submission in Phase 7, where §83 requires it to walk
the checker through compiling and running every mode.

## Build environment

- MSYS2 **UCRT64** / MinGW-w64, installed at `C:\msys64`
- C++17
- SFML **3.x** (graphics + network)
- Stockfish, used as a separate process for the Bot (added later)

Installed versions (MSYS2 UCRT64):

- g++ 16.2.0 — `mingw-w64-ucrt-x86_64-gcc 16.2.0-3`
- SFML 3.0.2 — `mingw-w64-ucrt-x86_64-sfml 3.0.2-3`
- gdb 17.2 — `mingw-w64-ucrt-x86_64-gdb 17.2-1`

## Installing the toolchain

In the **MSYS2 UCRT64** shell:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb mingw-w64-ucrt-x86_64-sfml
```

## Building

Run all commands from the project root.

Provided GUI example:

```bat
build_example.bat
```

Client:

```bash
g++ -std=c++17 common/*.cpp client/*.cpp ProvidedGUI/ChessFrontend.cpp -IProvidedGUI \
    -o client.exe \
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
```

Server:

```bash
g++ -std=c++17 common/*.cpp server/*.cpp -o server.exe -lsfml-network -lsfml-system
```

The server links no `sfml-graphics`, since it displays no GUI.

### Running the server

```bash
./server.exe          # listens on the default port, 53000
./server.exe 53001    # or a port of your choosing
```

It prints the port and the two addresses a Client can use — `127.0.0.1` for a
Client on the same machine, and this machine's LAN address for a Client on
another. Those are what you type into the Client's connection screen. It then
waits, logging every connection, pairing, move, draw offer, resignation and
disconnection, until you close it.

### Running the client

```bash
./client.exe
```

It asks for the server's address and port first; the main menu appears only
once that connection succeeds. The menu offers a Local Game (two players at one
keyboard), Player vs Bot (choose Easy, Medium or Hard first), Online Game, and
Exit.

### Playing an online game

Online play needs the server and **two** clients. Start the server, then run
`client.exe` twice — either twice on this machine, or once here and once on
another machine on the same network, using the LAN address the server printed.

In both clients, connect and choose **Online Game**. The first to ask sees
"Waiting for an opponent..."; when the second asks, the server pairs them,
assigns colours and opens both boards. The server owns the position: a click
sends a request and the piece moves only once the server has approved it, so
moving out of turn or making an illegal move simply does nothing on the board
and is logged as rejected on the server console.

Closing a client's board window while a game is running concedes that game, and
the opponent is told. The server keeps running, and any other games in progress
are unaffected.

### Verifying the chess rules

The rules engine is checked with **perft**, which plays every legal move to a
given depth and counts the positions reached. The counts for the six standard
test positions are published and agreed on across chess engines, so matching
them all is meaningful evidence rather than a self-written test passing.

```bash
g++ -std=c++17 -O2 -Icommon common/*.cpp tests/perft_main.cpp -o perft.exe \
    -lsfml-network -lsfml-system
./perft.exe
```

(`common/` holds the protocol message definitions as well as the rules, so
anything built from `common/*.cpp` links SFML Network even when, as here, it
never opens a socket.)

All 26 cases pass — roughly 15 million positions. Re-run it after any change
under `common/`. `tests/` is a development tool and is not part of the
submission.

### Verifying the Bot's difficulty levels

The three levels are checked by making them play each other, so the difference
is measured rather than judged by eye:

```bash
g++ -std=c++17 -O2 -Icommon -Iclient common/*.cpp client/StockfishEngine.cpp \
    client/BotDifficulty.cpp client/ConsoleInput.cpp tests/bot_match.cpp \
    -o bot_match.exe -lsfml-network -lsfml-system
./bot_match.exe
```

Each pairing is played from both colours, so a win cannot be explained by
moving first. The stronger level wins all six games by checkmate. Every engine
move is played through `ChessGame::makeMove`, so the run also confirms our own
rules accept everything a real engine produces.

## Stockfish

Stockfish 19 (release tag `sf_19`), Windows x86-64 **universal** build, in
`stockfish/`:

- `stockfish/stockfish.exe` — the engine, run as a separate process
- `stockfish/Copying.txt` — GNU GPL v3, required when redistributing the binary
- `stockfish/AUTHORS`

The universal build picks the best instruction set for the host CPU at runtime,
so it runs on any x86-64 Windows machine without choosing a per-CPU variant.

Stockfish is launched by a path **relative to the executable**, never an
absolute one. It selects the Bot's move only; move legality and game state
remain the responsibility of this project.

SHA-256 of `stockfish.exe`:
`45bc8e4969147db9c2eb533810637994619bff0eacc81ccfd9854394901bcbd0`

## Running

### Runtime DLLs

The executables link dynamically against SFML and the MinGW runtime, so
Windows must be able to find those DLLs. Either run from a shell that has
`C:\msys64\ucrt64\bin` on `PATH` (the MSYS2 UCRT64 shell does), or copy the
DLLs next to the executables. Collect them from **both** binaries — they do not
need the same set:

```bash
ldd client.exe server.exe | grep -i '/ucrt64/bin' | awk '{print $3}' | sort -u \
    | xargs -I{} cp -u {} .
```

There are 19 of them. Collecting from `gui_example.exe` alone is not enough: it
does no networking, so it misses `libsfml-network-3.dll`, and `server.exe` then
dies the moment it is launched — with **no error message at all**, just an exit
code of `0xC0000135`, "DLL not found". The graphics DLLs are equally invisible
to `server.exe`, which does not link them.

Without the DLLs, double-clicking an executable fails with
"libsfml-graphics-3.dll was not found" or dies silently as above.

**For submission:** the required DLLs must sit beside `client.exe` and
`server.exe` in the zip, otherwise the project will not run on the checker's
machine. They are excluded from git but must be included in the zip.

### Working directory

Start the executables **from the project root**. `ChessFrontend.cpp` looks for
the piece images relative to the current working directory — it tries
`ProvidedGUI/assets/`, then `assets/`, then the same two paths one and two
levels up. Launched from anywhere else the board renders lettered placeholder
squares instead of pieces.

## Folder layout

```text
.
├── ProvidedGUI/          supplied, not modified
│   ├── ChessFrontend.hpp
│   ├── ChessFrontend.cpp
│   └── assets/           12 piece images
├── Example/
│   └── main.cpp          supplied GUI demo
├── common/               chess rules, shared by both programs
├── client/               console screens, GUI wiring, Bot, client networking
├── server/               server sources
├── tests/                perft and Bot-match harnesses; not submitted
├── stockfish/            Stockfish 19 engine + GPL licence
├── build_example.bat     supplied
├── GUI_USAGE.md          supplied GUI API reference
└── README.md
```

`client/` still holds a `.gitkeep` so git tracks the directory while it is
empty. Delete it once real sources are added.
