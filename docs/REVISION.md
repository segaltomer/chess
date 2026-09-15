# Revision sheet

Questions and answers about this project, written as it was built. This is the
file to read before the test — not the code, and not `NOTES.md`.

Each entry is a question someone could ask about the project, with the answer.
Questions are added at the end of every phase, covering what that phase built.
Answers are short enough to hold in your head and specific enough to say out
loud.

Read it by covering the answers first and trying to answer from memory. The
reason the answers are written down is so you can check yourself, not so you
can read them passively — recognising an answer and being able to produce one
are different things, and only the second survives being asked.

---

## Setup and toolchain

**Why MSYS2 / MinGW and not Visual Studio?**
The requirements pin it: §85 says MSYS2 UCRT64 / MinGW, and §88-89 give the
exact `g++` command lines the README has to document. Visual Studio uses MSVC,
a different compiler, and its linker cannot read the MinGW-format SFML
libraries that pacman installs. The lecturer's own `build_example.bat` also
hardcodes `C:\msys64\ucrt64\bin` and calls `g++`.

**What does the provided GUI do, and what does it not do?**
It draws the board and pieces, shows player names, turn and status text, and
reports clicks. It does *not* know the rules of chess, does not validate moves,
does not manage games, does not do networking, and does not decide who won
(§2). It reports intent — `MoveRequested`, `ResignRequested` — and our code
decides what that means.

**Why are 18 DLLs sitting in the project folder?**
The executables link dynamically against SFML and the MinGW runtime. Windows
looks for those DLLs next to the executable and on `PATH`. The grading machine
will not have our `PATH`, so the DLLs ship beside the executables in the
submission zip. Without them the program dies at launch with
"libsfml-graphics-3.dll was not found".

**Why is the chess rules code in `common/` rather than in the client?**
Because it compiles into both programs. The Client validates Local and Bot
games (§17-18), the Server validates Online games (§22), and §6 says the rules
are identical in every mode. One copy in `common/`, built into both binaries,
is the only way to guarantee that without maintaining two implementations.
