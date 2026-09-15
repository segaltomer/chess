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
