# C++ Chess — Client / Server

Final project for the C++ course. This is the **setup skeleton only**: no chess
rules, no networking and no protocol are implemented yet.

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

Client (once `client/` contains sources):

```bash
g++ -std=c++17 common/*.cpp client/*.cpp ProvidedGUI/ChessFrontend.cpp -IProvidedGUI \
    -o client.exe \
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
```

Server (once `server/` contains sources):

```bash
g++ -std=c++17 common/*.cpp server/*.cpp -o server.exe -lsfml-network -lsfml-system
```

The server links no `sfml-graphics`, since it displays no GUI.

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
DLLs next to the executable:

```bash
ldd gui_example.exe | grep -i '/ucrt64/bin' | awk '{print $3}' | xargs -I{} cp -u {} .
```

Without this, double-clicking the executable fails with
"libsfml-graphics-3.dll was not found" and similar.

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
├── common/               shared rules + protocol
├── client/               client sources
├── server/               server sources
├── stockfish/            Stockfish 19 engine + GPL licence
├── build_example.bat     supplied
├── GUI_USAGE.md          supplied GUI API reference
└── README.md
```

`client/` and `server/` each hold a `.gitkeep` so git tracks the directory
while it is still empty. Delete those once real sources are added.
