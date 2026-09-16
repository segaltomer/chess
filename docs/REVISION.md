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

---

## Phase 1 — the chess rules

**How is the board stored, and why that way?**
One flat array of 64 `Piece` values, indexed 0 = a8 up to 63 = h1. Two reasons.
That is exactly the indexing the provided GUI's `setBoard` uses, so displaying
the board needs no conversion and cannot come out flipped. And because a
`Piece` is a plain value — a piece type and a colour — copying an entire board
is copying 64 small structs. That matters because deciding whether a move is
legal is done by copying the board and playing the move on the copy, which
happens for every candidate move. Had each square held a `unique_ptr<Piece>`,
every one of those copies would be a deep clone of up to 32 heap objects.

**How do you know a move does not leave your own king in check (§31)?**
Copy the board, play the move on the copy, then ask whether your own king is
attacked in the resulting position. If it is, the move is not legal.

The reason this is worth doing rather than tracking pins directly is that one
test covers every case without any of them being written down: pinned pieces,
the king walking into check, capturing the piece that is giving check, blocking
a check, and the en passant capture that removes two pawns from a rank and
exposes the king along it. All of them are the same question asked about the
position that results.

**Where is the polymorphism, and why there?**
Two places. `PieceRules` is an abstract base with a subclass per piece type,
each knowing how its own piece moves — so fixing a bug in how bishops move
touches one small class instead of one arm of a six-way switch. And the game
modes (Phase 2) will be a `Game` base with `LocalGame`, `BotGame` and
`OnlineGame`, because the three differ in exactly one thing: who decides a
move.

The piece rules objects are **stateless** — they hold no square and no colour,
they answer questions about a board handed to them, and one shared instance of
each exists. That is what lets the board stay a plain array of values:
behaviour is polymorphic, data is not.

Bot difficulty is deliberately *not* a class hierarchy. The three levels differ
only in the numbers passed to Stockfish, not in behaviour, so classes there
would be decoration. §73 asks for polymorphism where it suits the design.

**Why is there a separate `attacksSquare` instead of just checking whether a
piece has a move to that square?**
Because for a pawn those are different squares. A pawn moves straight ahead but
captures diagonally. Defining "attacks" as "has a move there" would say a pawn
attacks the empty square in front of it — which it does not — and that it does
not attack the diagonals, which it does. Check detection is built on
`attacksSquare`, so getting this wrong would give wrong answers about check in
exactly the positions where it matters.

**What is pseudo-legal, and why generate moves that are not legal?**
A pseudo-legal move is one that obeys how the piece moves but has not been
checked for leaving your own king exposed. Move generation produces those, and
a second pass filters them.

They are separate because otherwise generation would have to call itself.
Deciding "does this move leave me in check?" means asking whether the *enemy*
could then capture my king — which is a question about enemy moves. If move
generation filtered for check, it would need move generation to answer.
Splitting it in two breaks the loop.

**Why is castling generated in `Rules` rather than with the king's other
moves?**
Two of castling's conditions are about attacked squares: you may not castle out
of check, and the king may not cross a square the enemy attacks. Answering that
needs `isSquareAttacked`, which lives in `Rules`. If `KingRules` called into
`Rules`, the two would depend on each other in a circle. Generating it one
level up keeps the dependency pointing one way — and is the honest place for
it, since castling is a fact about a king, a rook and the check state together,
rather than about how a king moves.

**What are castling's conditions, and which one is easy to get wrong?**
The right must still exist (neither the king nor that rook has moved, and that
rook has not been captured on its corner), the squares between king and rook
must be empty, the king must not be in check, must not cross an attacked
square, and must not land on one.

The trap is treating "must be empty" and "must not be attacked" as one list.
Castling queen's side needs b1, c1 and d1 empty because the rook crosses all
three — but the king only goes e1 to d1 to c1, so b1 may be attacked and the
castle is still legal.

**What makes en passant harder than it looks?**
Two things. The right lasts exactly one move, which is handled by clearing the
target square at the start of every move so it can only survive the one that
set it. And it is the only move in chess that removes two pieces from the same
rank at once — the capturing pawn leaves it and the captured pawn disappears
from it — so it can expose your own king along that rank to a rook or queen in
a way no other capture can. Our engine gets that right without any code for it,
because legality is decided by looking at the position that results.

It is also the only capture that lands on an empty square: the pawn being taken
is beside the capturer, not on the square it moves to.

**How is the rules engine tested? Why trust it?**
With **perft**. Perft plays out every legal move to a given depth and counts
the positions reached. Published counts exist for standard positions and every
chess engine in the world agrees on them, so matching is evidence rather than
optimism — it is not "the cases I thought to test pass".

All 26 cases pass, about 15 million positions over the six standard positions,
which were chosen by engine authors precisely because they are nasty: pins,
discovered check, castling rights lost to a rook capture, under-promotion, and
the en passant capture that is illegal because of the rank it clears. From the
starting position, depth 5 is 4,865,609. When a count disagrees, the harness's
`divide` mode prints the count under each individual move, which narrows the
bug to one move and then one position.

**A game has ended. What does the GUI get told, and where does that come
from?**
Two things, because §39 asks for both: the result and the reason.
`ChessGame::resultTitle()` gives `"White wins!"`, `"Black wins!"` or `"Draw"`,
and `resultDetail()` gives `"Checkmate"`, `"Stalemate"`, `"White resigned"` or
`"Draw agreed"` — exactly the two strings `showGameOver` takes. One without the
other is not enough: "Draw" does not say whether it was agreed or forced.

They are produced in `common/` rather than in the Client so that a Local game,
a Bot game and a game running on the Server all word the same outcome the same
way.

**Why are the result and reason stored rather than worked out on demand?**
Because two of the four reasons are events, not positions — you cannot look at
a board and see that a player resigned or that both agreed a draw. Storing them
also makes the outcome permanent: once a game is over nothing can overwrite the
result, and `makeMove` refuses every move from then on.

**When a checkmate happens, how do you know who won?**
The side *to move* is the side that has been mated, so the winner is the other
one. The check runs after the turn has already changed hands, so the player who
just moved is the winner. Getting this backwards reports every mate as a win
for the wrong colour, and it is easy to miss, because the game does end
correctly — only the name on it is wrong.
