# How this project works

A running explanation, written as the project is built — one entry per piece of
work, added at the time it is written, not reconstructed at the end.

**Purpose:** I have to be able to explain this project out loud. This file is
what I revise from. So entries are short, in plain language, and say *why* as
well as *what*. If an entry needs a diagram to make sense, it gets one.

**Format for each entry:**

```
## <what was built>  ·  <files>  ·  <requirement refs>

What it does — two or three sentences, plain language.

Why it is built this way — the alternative that was rejected and why.

The tricky part — the one thing that was not obvious, or that broke first.
```

---

## Project setup · toolchain, ProvidedGUI, folder layout · §85-89

The project compiles with `g++` from MSYS2 UCRT64 (g++ 16.2.0, SFML 3.0.2),
because §85-89 require it and the lecturer's own `build_example.bat` hardcodes
that path. Visual Studio would mean MSVC, a different compiler whose linker
cannot read the MinGW SFML libraries installed by pacman.

`ProvidedGUI/` is the lecturer's GUI component, unmodified and submitted as-is.
It only draws and reports clicks — it does not know chess rules, does not
validate moves and does not do networking (§2). Our code tells it what to show
via `setBoard`, `setTurnText`, `setStatusText`, and receives user intent back
from `pollEvent`.

Code is split three ways: `common/` for the chess rules and protocol messages,
`client/` for screens and GUI wiring, `server/` for hosting online games. The
rules are in `common/` because they compile into *both* programs — the Client
validates Local and Bot games (§17-18) while the Server validates Online ones
(§22), and §6 requires those to be the same rules.

**The tricky part:** the executables need 18 SFML and MinGW DLLs at runtime.
Windows only looks next to the executable and on `PATH`, so the DLLs are copied
into the project folder — and they must ship in the submission zip, because the
grading machine will not have our `PATH`.

---

## Board and piece representation · `common/ChessTypes`, `common/Move`, `common/Board` · §26-28, §34-36

A position is one flat `std::array<Piece, 64>`, indexed 0 = a8 up to 63 = h1 —
the same numbering `ChessFrontend::setBoard` uses, so `Board::toDisplay()` is a
straight character-by-character translation with no row flipping anywhere.
`Piece` is a two-field value (`PieceType`, `Color`), where `PieceType::None`
means the square is empty. `Board` also holds the two facts that belong to a
position but are invisible on it: the four castling rights, and the en passant
target square.

**Why values and not piece objects:** deciding whether a move is legal means
asking "if I played this, would my own king be attacked?" (§31). The cheapest
honest way to answer is to copy the board, play the move on the copy and look.
With plain values that copy is 64 small structs. With a `unique_ptr<Piece>` per
square it would be a deep clone of up to 32 heap objects, on every candidate
move — slower, and far more code to get wrong. Behaviour still gets to be
polymorphic; it just lives in the separate `PieceRules` classes rather than in
the board (DESIGN decision 6).

`Board::applyMove` does the mechanical work of a move and asks no questions —
the caller has already decided the move is legal. It is the one place that
knows a castle also moves a rook, that a pawn captured en passant is not on the
square the capturer lands on, and that a promoting pawn is replaced. Having
exactly one such place is what lets the legality test and the real move share
code instead of drifting apart.

**The tricky part:** the castling rights. A right is lost when the king moves,
when the rook moves, *and* when the rook is captured where it stands — and that
last one is easy to forget, because nothing about the move looks like castling.
All three collapse into one test: if a move's from-square or to-square is a
corner, that corner's right is gone. Either the rook left it or the rook died
on it; either way the rook that started there is no longer there.

The en passant target is reset at the top of `applyMove` before anything else
runs, so it survives exactly one move — which is the whole rule (§35).

---

## How each piece moves · `common/PieceRules` · §26-28, §36

`PieceRules` is an abstract base with one question per method: "from this
square, on this board, as this colour, what moves could you make?" and "do you
attack this square?". There is one instance per piece type, handed out by
`rulesFor(PieceType)`. The objects hold no position and no colour — everything
they need arrives as arguments — so one shared instance of each serves the
whole program, and the board can stay a plain array of values.

Three subclasses would have been six near-identical files, so the shared shapes
are factored out. `SlidingPieceRules` walks a ray until it leaves the board or
meets a piece; bishop, rook and queen are then one constructor each, differing
only in their direction list. `SteppingPieceRules` takes each offset once;
knight and king are one constructor each. Only `PawnRules` is written out in
full, because the pawn is the one piece that does not capture the way it moves.

**Why `attacksSquare` exists separately from move generation.** It is tempting
to define "X attacks square S" as "X has a move to S", and for five pieces that
works. It is wrong for the pawn: a pawn moves straight ahead but captures
diagonally, so that definition would say a pawn attacks the empty square in
front of it, and a king would be reported as unable to stand next to an enemy
pawn's push square while happily walking onto the diagonal that really is
attacked. Check detection is built on `attacksSquare`, so getting this wrong
would mean wrong answers about check in exactly the positions that matter.

**The tricky part:** pseudo-legal versus legal. These functions deliberately
generate moves that leave your own king in check, which §31 forbids. They have
to. Deciding "does this move leave me in check?" means asking whether the enemy
could capture my king, which is a question about enemy moves — so if move
generation filtered for check, it would have to call itself to answer. Cutting
it in two breaks the loop: `PieceRules` generates freely, `Rules` filters
afterwards by playing the move on a copy of the board and looking.

Promotion is handled here too: when a pawn's move lands on the far rank, the
generator emits the same from/to square four times, once per piece it could
become (§36). Making the promotion part of the move means the rest of the
engine never needs a special case for it — a promotion is just a move that
happens to carry a piece type.

---

## Check, legality and the turn · `common/Rules`, `common/ChessGame` · §29-31

`Rules` answers three questions about a position. `isSquareAttacked` asks every
enemy piece in turn whether it attacks a square. `isInCheck` finds the king and
asks that. `legalMoves` takes the pseudo-legal list and keeps only the moves
that pass one test: copy the board, play the move on the copy, and ask whether
the mover's own king is attacked in the result (§31). If it is, the move never
existed.

That copy-and-test is worth understanding, because it is the reason the rest of
the engine is so short. It needs no notion of a pin, no special case for
"moving a piece that was shielding the king", no list of who is defending whom
— and it handles the awkward cases for free, including the king walking into
check, capturing the piece that was giving check, and an en passant capture
that removes a pawn which was blocking a rook. Every one of those is the same
question asked of the resulting position. The price is copying 64 bytes per
candidate move, which is why the board is values (DESIGN decision 1).

`ChessGame` is the authority: it holds the position, whose turn it is, and
nothing else. `makeMove` checks the move against the legal list for the side to
move and either plays it and passes the turn (§29) or changes nothing at all.
There is no half-applied state to recover from, so a rejected move — a wrong
turn, an illegal move, a malformed one out of a network message — is simply a
`false`. Phase 5's §67 leans on that.

**The tricky part:** `isSquareAttacked` uses `attacksSquare`, *not* the move
generator, and this is load-bearing rather than an optimisation. A pawn's moves
and a pawn's attacks are different squares. Checking "is the king in check" by
looking for an enemy move onto the king's square would report the square in
front of a pawn as attacked and the diagonals as safe — backwards, in exactly
the situation that matters.

**How it is verified — `tests/perft_main.cpp`.** Perft plays out every legal
move to a given depth and counts the positions reached. Published counts exist
for standard positions and every chess engine agrees on them, so matching is
evidence rather than optimism: from the starting position, depth 4 must be
exactly 197,281. It is. A wrong number is also a usable bug report — the
harness's `divide` mode prints the count under each individual move, so
comparing against a known-good engine narrows a miscount to one move, then one
position. Depths 1-4 from the start are reachable without castling or en
passant, so those are the cases checked for now; the rest of the suite is added
as those moves are implemented.
