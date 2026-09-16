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

---

## Checkmate and stalemate · `common/Rules`, `common/ChessGame` · §32-33

Both are the same situation: the side to move has no legal move at all. They
differ only in whether that side is in check while it happens. In check with
nothing to play is checkmate and loses the game; not in check with nothing to
play is stalemate and draws it.

Written that way, neither needed any new machinery — they are two lines each on
top of `legalMoves` and `isInCheck`. That is the payoff from defining "legal"
properly in the first place: because a legal move is already one that does not
leave your own king in check, an empty legal-move list already means "there is
genuinely nothing this player can do", with escapes, blocks and captures of the
checking piece all accounted for.

**The tricky part:** it is tempting to detect mate by looking for the king
having no square to run to. That is wrong twice over — it misses the mates
where the king has an empty square but every escape is attacked, and it calls
mate in positions where some *other* piece can block the check or capture the
checker. Asking "is the legal move list empty" is both simpler and correct,
and it gets stalemate right at the same time instead of as a separate rule.

---

## Castling · `common/Rules` · §34

Castling has six conditions and the code checks them in this order: the right
still exists, both pieces are really on their squares, the squares between them
are empty, the king is not in check, the king does not cross an attacked
square, and the king does not land on one.

The first two are `Board`'s job and were done in the representation commit — a
right is lost when the king moves, when that rook moves, or when that rook is
captured where it stands. The last one is free: `legalMoves` already plays
every candidate move on a copy and rejects it if the mover's king ends up
attacked, and for a castle `Board::applyMove` moves the rook too, so the test
sees the real resulting position.

**Why it is generated in `Rules` and not in `KingRules`** with the king's other
moves: two of the conditions ask whether a square is attacked, and that answer
lives in `Rules::isSquareAttacked`. If `KingRules` asked for it, `PieceRules`
and `Rules` would depend on each other in a circle. Generating castling one
level up keeps the dependency pointing one way — `Rules` uses `PieceRules`,
never the reverse. It is also the truthful place for it: castling is a fact
about a king, a rook and the check state together, not about how a king moves.

**The tricky part:** the three "empty" squares and the three "not attacked"
squares are different sets, and it is easy to use one list for both. Castling
queen's side moves the rook from a1 across b1, c1 and d1, so all three must be
empty — but the king only goes e1 → d1 → c1, so b1 may be attacked and the
castle is still legal. A single loop over "the squares between" would wrongly
forbid it. There is a test for exactly this case.

---

## En passant · `common/PieceRules` · §35

Adding the rule took six lines, because the two halves were already in place
from earlier commits. `Board` sets the target square whenever a pawn double
steps and clears it at the start of every following move, so the right expires
after exactly one move on its own. `Board::applyMove` already knew that a pawn
landing on that square captures a pawn standing *beside* it rather than on it.
All that was missing was `PawnRules::appendCaptures` offering the move: the
diagonal square is a legal capture if it holds an enemy piece **or** if it is
the en passant target.

It needs no test for whose pawn it is. The target is cleared on every move, so
the only one that can still be set was created by the opponent's last move, and
therefore belongs to the player now choosing.

**The tricky part** is the one case that catches out most hand-written engines,
and it is worth being able to describe: en passant is the only move that takes
*two* pieces off one rank at once — the capturing pawn leaves and the captured
pawn vanishes, from the same rank. So it can expose your own king along that
rank to a rook or queen, in a way no other capture can. The engine gets this
right for free, because legality is tested by playing the move on a copy and
looking at the resulting position, and `applyMove` removes the captured pawn
before that look. Nothing special was written for it. There is a test, and the
"endgame" position in the perft suite exists specifically to catch engines that
get it wrong.

**Verification.** With en passant in place the whole standard perft suite
became meaningful and all 26 cases pass — about 15 million positions across six
positions chosen by engine authors to be nasty. That is the evidence that
§26-36 are right in general and not merely in the cases we thought to check:
start position to depth 5 is 4,865,609, Kiwipete to depth 4 is 4,085,603.

---

## Promotion, and how the GUI will ask · `common/ChessGame` · §36

The engine side has been done since the movement commit: a pawn move landing on
the far rank is generated four times over, once carrying each piece it could
become. So the promotion is part of the move rather than a step that follows
it, and every later stage — legality, applying it, sending it over the network,
handing it to Stockfish — needs no special case. `e7e8q` is just a move.

What this commit adds is the one thing the Client needs to drive the dialog:
`needsPromotionChoice(from, to)`. The provided GUI reports a click on a source
square and then a target square, with no idea that a promotion is involved — it
never knows the rules. So the Client asks this after the second click, and if
it comes back true, calls `showPromotionDialog`, waits for `PromotionSelected`,
and only then calls `makeMove` with the answer filled in.

**The tricky part:** the choice is not optional and the engine enforces it in
both directions. `a7a8` with no piece named is not a legal move and is refused,
because no such move was ever generated — and equally, `e2e4` carrying a queen
is refused, because that was not generated either. A caller cannot forget to
ask, and cannot smuggle a promotion onto a move that is not one. That falls out
of `Move::promotion` being part of what makes two moves equal.

Under-promotion is real, not decoration: there is a test where a pawn reaching
e8 gives check as a knight and does not as a queen, because e8 and the black
king on g7 share no rank, file or diagonal. §36 requires all four choices.

---

## The result and the reason · `common/ChessGame` · §37-39

A finished game has two facts, and §39 wants both: the **result**
(`GameResult` — who won, or a draw) and the **reason** (`GameEndReason` —
checkmate, stalemate, resignation, draw agreed). One without the other is not
enough to tell a player what happened: "Draw" does not say whether they agreed
to one or were stalemated, and "Black wins" does not say whether White was
mated or gave up.

Both are **stored**, not recomputed. They have to be, because two of the four
reasons are events rather than positions — you cannot look at a board and see
that someone resigned. Storing them also means `isOver()` is a comparison
rather than a full legal-move generation, and it makes the outcome permanent:
once set, nothing overwrites it. A resigned game cannot later be drawn, and
`makeMove` refuses everything once the game is over.

Checkmate and stalemate are filled in by `updateResultFromPosition`, called
after every accepted move — and once by `fromFen`, because a position loaded
from text may already be finished.

`resultTitle()` and `resultDetail()` return exactly the two strings
`ChessFrontend::showGameOver` takes: `"Black wins!"` and `"White resigned"`.
They live in `common/` on purpose. The Server needs the same two strings to
send to both clients when an online game ends, so producing them in one place
is what stops a Local game and an online game describing the same outcome in
different words.

**The tricky part:** the side to move is the side that has *lost* when there is
a checkmate. `updateResultFromPosition` runs after the turn has already
changed hands, so if the new side to move is mated, the winner is the player
who just moved. Getting that backwards would report every checkmate as a win
for the wrong colour — and it would pass a casual test, because a game ending
in a mate looks correct until you read whose name is on it.

Resignation takes the colour rather than assuming the side to move, because a
player may resign at any time, including on the opponent's turn (§53). The
detail string does not need to store who resigned: the resigner is the loser,
so the result already says which.
