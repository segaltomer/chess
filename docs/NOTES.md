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
grading machine will not have our `PATH`. *(Superseded: the lecturer later ruled
out DLLs in the submission, so the programs are run from the MSYS2 UCRT64 shell
instead. See the last entry.)*

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
became meaningful and all 26 cases pass — about 16.5 million positions across six
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

---

## The minimal Server · `server/Server`, `server/main.cpp` · §11

`Server` binds a port in its constructor — so a `Server` that exists is a
`Server` that is listening, and there is no half-built state where the object
exists but is not serving. If the port cannot be bound it throws, and `main`
turns that into a readable line rather than a stack trace. That is almost
always another program already holding the port, including a second copy of
this server.

The loop is one thread around an `sf::SocketSelector`. The selector is told
about the listening socket and every connected client, and `wait()` sleeps
until at least one of them has something to say. So an idle server with fifty
clients uses no processor time at all, without a thread per client.

**Why one thread** (DESIGN decision 5): with no state shared between threads
there are no locks, and a whole family of crash becomes impossible. §64 says a
client disconnecting must never bring the server down, and thread-per-client
over unlocked shared game state is the classic way to fail exactly that.

§11 is why it prints two addresses. `127.0.0.1` works for a Client on the same
machine; a Client on another machine needs this machine's LAN address, which is
what `sf::IpAddress::getLocalAddress()` returns. §25 requires the second case
to work, and a player cannot type an address nobody has told them.

Clients live in a `std::vector` with id numbers from the first line of code,
even though there is nothing yet for a second client to do. §60 forbids
assuming a single game exists on the server, and that is far cheaper to honour
from the start than to retrofit around code written for one.

**The tricky part:** a closed socket reports itself *ready to read* forever. So
a client that has gone away must be removed from the selector as well as from
the list — miss that and the loop spins at 100% CPU on a socket that will never
have anything to say. `receive` returning `Disconnected` is how that is caught.

**What actually went wrong:** `server.exe` built cleanly and then died
instantly on launch printing nothing whatsoever — exit code `0xC0000135`. The
19 runtime DLLs in the project folder had been collected from
`gui_example.exe`, which does no networking, so `libsfml-network-3.dll` was
never among them. Windows reports a missing DLL by killing the process before
`main` runs, with no message on the console. Worth remembering, because the
symptom looks like a crash in code that has not executed yet. The submission
later dropped DLLs altogether; see the last entry.

---

## The connection screen · `client/ConnectionScreen`, `ServerConnection`, `ConsoleInput` · §7-10

§8 makes this a **gate**, not a step: the main menu is unreachable until a
connection has actually succeeded. So `ConnectionScreen::run` loops until one
of exactly two things happens — it connects, or the player leaves. There is no
third way out, and that is the requirement rather than a stylistic choice.

It is a class rather than a function because it has to remember the address and
port between attempts. §10 requires "try again", "change the address" and
"change the port" to be three separate offers, and the last two mean nothing
without remembering what the other one was.

`ServerConnection` wraps the socket and closes it in its destructor (§76), so
no exit path — including an exception on the way out of `main` — leaves it
open. It is non-copyable, because two objects believing they own one connection
is a bug waiting to happen.

Three separate §9 error cases, caught in three different places:

- **A bad address** is caught the moment it is typed, by resolving it there
  rather than at connection time. Otherwise a typo would look identical to the
  server being switched off.
- **A bad port** is caught by `ConsoleInput::readNumberInRange`, which rejects
  letters, an empty line, a number out of range, *and* trailing rubbish —
  `std::stoi` alone reads "53000abc" as 53000 and silently drops the rest.
- **Nothing answering** is caught by the connection attempt, and the message
  lists what to check rather than saying "error".

**The tricky part, and it is not obvious:** typing the *port* into the address
box passes validation. Winsock reads a bare number as a packed 32-bit address,
so `sf::IpAddress::resolve("53000")` cheerfully succeeds and yields 0.0.207.8,
and the player is then told that address did not answer — which is true and
completely unhelpful. It is the easiest mistake to make on this screen, so an
all-digits address is rejected with a message naming the actual mistake.

There is a second subtlety in `ConsoleInput`: every read distinguishes *bad
input* from *the input stream having ended*. Bad input is re-prompted; an ended
stream must not be. A loop that keeps asking a stream which can never answer
again spins forever at full processor, and that is exactly what happens when
the console window is closed or input is piped from a file that runs out.

**The connect timeout** is five seconds. Without one, an address with no
machine at it can take most of a minute to give up, which is indistinguishable
from a hung program.

---

## The main menu and the Local Game · `client/MainMenu`, `client/LocalGame` · §8, §12-13, §17, §39, §48-56

The Local Game is where the rules engine finally meets the screen. The loop is
the one `GUI_USAGE.md` documents — poll an event, react to it, render — and
everything interesting is in the reacting.

**Square numbering needed no conversion at all.** The GUI reports a click as a
row and a column with row 0 meaning rank 8, which is exactly how the engine
numbers its 64 squares. So a click becomes `squareIndex(row, col)` and that is
the whole translation. That was the point of choosing the GUI's own indexing
back in the representation commit; the payment for it comes here.

**An illegal move needs no handling.** `makeMove` returns false and changes
nothing, the board is redrawn from the unchanged position, and the piece simply
appears not to have moved. There is no "undo" path because nothing was ever
done — which is why `makeMove` was written to be all-or-nothing.

**Promotion is the one move that takes two answers.** The player picks the
squares, and only then the piece. So the from/to squares wait in
`promotionFromSquare_`/`promotionToSquare_` while the dialog is up, and the
move is only built and played once `PromotionSelected` arrives. The GUI's
promotion overlay swallows every other click while it is open, so there is no
way to wander off and leave a half-made move behind. The letter it sends —
'Q', 'R', 'B', 'N' — is the same letter a white piece is drawn with, so
`fromDisplayChar` already converts it.

**Resignation is two steps on purpose** (§54-55). The GUI reports only that the
button was clicked; it does not decide anything. Our code answers that by
showing the confirmation dialog, and nobody actually resigns until
`ResignConfirmed` arrives. `ResignCancelled` does nothing at all, which is the
requirement.

Because both players share one screen and one mouse, the player who resigns is
taken to be the one whose turn it is. A player wanting to resign on their
opponent's turn has to wait for their own — a limit of having a single Resign
button, which is the button the provided GUI gives us.

**The draw offer names both players** (§49). In an online game the offer
appears on the other person's machine and there is no doubt who is being asked.
Here both players are at the same window, so the dialog says "White offers a
draw. Black, do you accept?" — otherwise the wrong person answers it.

**The tricky part:** the game-over summary must be shown exactly once. Its
close button hides the overlay and reports *no event at all*, so if the result
were shown whenever `isOver()` is true, it would reappear on the very next
frame and could never be dismissed. Hence `hasShownResult_`. Once the game ends
the board input is switched off and the Offer Draw button is hidden, so the
final position can be looked at but not disturbed; closing the window returns
to the console menu.

One small thing worth knowing: console messages printed just before the game
loop are flushed explicitly. Nothing reads the console while a game is running,
so an unflushed line can sit in the buffer, unseen, for as long as the board
window stays open.

---

## Talking to Stockfish · `client/StockfishEngine` · §45-47

Stockfish is a separate program, not a library. We start it with
`CreateProcess`, having first made two pipes — one we write its input to, one
we read its output from — and then talk **UCI**, the plain-text protocol every
chess engine speaks. Commands go in as lines (`position fen ...`, `go movetime
500`) and replies come back as lines (`bestmove e2e4`).

It is found relative to the running executable, never by an absolute path,
because the project is marked on another machine. `GetModuleFileNameW` gives
the client's own folder and `stockfish/stockfish.exe` is looked for there
first, then relative to the working directory.

This is the only file in the project with Windows-specific code in it. The
process and pipe handles are declared `void*` in the header so `<windows.h>`
never leaks into anything that includes it.

**The tricky part, and it cost real time to find:** an engine reading from a
pipe treats **end-of-file as `quit`**. While testing this from the shell by
feeding Stockfish a file of commands, it kept answering `bestmove a1b1` in a
position that is mate in one — with `nodes 1` in its own output. It was not
searching at all: it reached the end of the file, took that as `quit`, and
abandoned the search the instant it began. Nothing was wrong with the engine or
the position. The lesson carried into the real code is that our end of the pipe
must stay **open** for as long as the engine is wanted, and that we must never
send anything after `go` until `bestmove` has come back.

**Why reads are polled rather than blocking.** A plain `ReadFile` on a pipe
waits until something arrives, so an engine that died mid-search would hang the
whole board window with it, with no way even to close it. `PeekNamedPipe` asks
whether anything is waiting before committing to a read, so every read has a
deadline and a silent engine turns into a reported error instead of a freeze.

**The Bot cannot cheat.** §47 requires that Stockfish choose the Bot's move and
nothing more, and that is visible in the code: `chooseMove` returns a *string*,
which `BotGame` turns into a `Move` and puts through `ChessGame::makeMove` —
the identical call a human's click makes. An illegal or garbled suggestion is
refused by the ordinary legality check, not by a special case. This was tested
by playing 120 engine half-moves through our own rules; every one was accepted,
which also cross-checks our move generator against Stockfish's.

---

## The Bot and its difficulty levels · `client/BotGame`, `client/BotDifficulty` · §18-20, §43-44

A level is two numbers: how much of its skill Stockfish may use (its own `Skill
Level`, 0-20) and how long it may think. No class hierarchy — the levels differ
in values, not behaviour, so classes would be decoration, and §73 asks for
polymorphism only where it suits the design.

`Skill Level` is the lever that matters. Below 20, Stockfish deliberately picks
moves it has itself judged inferior, so a low setting **blunders** the way a
weak human does. Limiting the time alone would not achieve that: a full-strength
engine given a tenth of a second still plays far better than any student.

**How §20 was actually demonstrated**, rather than assumed: the levels were
played head to head. Hard beat Easy from both colours, Medium beat Easy, and
Hard beat Medium — every game by checkmate, and the material counts at the end
were lopsided (Medium finished with 3 points against Hard's 26). The ordering
is real and the gaps are wide.

**The tricky part:** the engine blocks while it searches, for up to a second on
Hard. The loop therefore renders the human's move *before* asking the Bot, so
the player sees their own piece land immediately instead of only when the reply
arrives, and the status line says "Bot is thinking..." with board input
switched off while it does. Doing it the other way round makes the program look
like it has frozen after every move.

`onOpponentTurn` is called on every frame, so it decides for itself whether
there is anything to do — it returns at once unless the game is running and it
is the Bot's turn. That keeps the base class free of any knowledge about which
modes have a non-human opponent.


## Proving the levels differ · `tests/bot_match.cpp` · §19-20

**What it does.** Starts two Stockfish processes at different difficulty
settings and makes them play a full game against each other, then repeats with
the colours swapped. Six games in all: Hard–Easy, Hard–Medium, Medium–Easy,
each from both sides.

**Why it exists.** §20 asks that the levels be *demonstrably* different, and
the obvious way to check — play them and see — turned out not to work here. The
student does not play chess well enough to feel the difference, and even a
strong player's impression would be an anecdote rather than evidence. Making
the levels play each other replaces the judgement with a result: the stronger
setting won all six games by checkmate, with final material as lopsided as 33
against 9. Swapping colours matters, because otherwise a win could just be the
advantage of moving first.

It also re-runs. The difficulty numbers are three constants someone could
change; this is what catches it if changing them stopped the ordering holding.

**The tricky part — two of them.**

The harness calls the shipped `settingsFor()` rather than repeating the skill
levels and times. A test holding its own copy of the values would keep passing
after the real ones were changed, which is worse than no test.

Neither engine is trusted to be right. Every move either one produces is played
through `ChessGame::makeMove`, and a refusal ends the game as a reported failure
instead of being worked around. That is §47 restated as a test, and it doubles
as a cross-check of our move generator against a real engine — several hundred
half-moves per run, all accepted.

Two things had to be handled for the games to end at all. Neither the fifty-move
rule nor draw by repetition is implemented, since §26-39 does not ask for
either, so two engines can shuffle forever; a game reaching 300 half-moves is
stopped and judged on material. And each pairing gets fresh `startNewGame()`
calls, so nothing carries over from the previous game.

**A test that was tried and thrown away.** The first attempt was to find a fixed
sequence of moves the student could click without understanding them — the
four-move "Scholar's Mate" — hoping Easy would fall for it and Hard would not.
Measured over eight games per level, every level defended it and White lost its
queen every time, so it separated nothing. Worth recording because the idea is
tempting: a beginner's trap is not a strength test, since avoiding one specific
known trap is exactly the kind of thing even a deliberately weakened engine
does by accident.


## The protocol · `common/Protocol` · §61-63

**What it does.** Defines every message the Client and Server send each other: a
one-byte type followed by that type's fields, packed into an `sf::Packet`.

**Why `sf::Packet`.** TCP is a stream of bytes with no idea where one message
ends and the next begins. Send two messages quickly and they can arrive stuck
together, or one can arrive in halves. A hand-written protocol has to buffer
partial reads and split them itself, which is a classic source of bugs for no
benefit in a chess project. `sf::Packet` does that framing, and packs integers
and strings with no parsing code.

**The tricky part — the format lives in exactly one file.** Each message is a
small struct with an `operator<<` and an `operator>>` next to each other, so the
side that writes a field and the side that reads it are three lines apart. Had
the Server written fields inline and the Client read them inline, the two would
be in different files and would drift the first time a field was added.

Two details that are easy to get wrong across separately compiled programs. The
type numbers are written out explicitly (`FindGame = 1`) rather than left to the
compiler, so inserting a name in the middle of the list cannot silently
renumber every message after it. And colours and piece types travel as a byte
through a converter that maps anything unrecognised to `None`, so a corrupt
message cannot produce a piece type that does not exist.

**`isKnownMessageType`** exists for §67: a type byte outside the list means the
message is malformed or came from something that is not our Client, and it is
refused before anything reads further into it.


## The Server: matchmaking and hosted games · `server/` · §21-25, §40-42, §57-60

**What it does.** Pairs waiting Clients, creates a game for each pair, validates
every move, and keeps the two players of each game up to date.

**The shape.** `Matchmaker` holds only the list of Clients waiting to be paired.
`HostedGame` is one game: the `ChessGame`, the two client ids, and any
outstanding draw offer. `Server` owns the sockets, decides what each arriving
message means, and logs.

**Why games are found by searching rather than by a lookup table.** The Server
could keep a client-id-to-game map alongside the games themselves. It does not,
because that is a second structure that has to be updated everywhere the first
one is, and the day it is not, a client is in a game the map says it is not in.
Searching the games for the one containing a client cannot fall out of step
with anything. At the scale of a classroom demonstration the cost is nil.

**Where §59 is enforced — one function.** `findGameForPlayer(clientId, gameId)`
returns the game only if that client is one of its two players. Every handler
starts by calling it, so "never tell a client about a game it is not in" is one
check in one place rather than a rule to remember at each of a dozen sends. A
client sending a move for somebody else's game gets a log line and nothing else.

**The tricky part — a disconnection is handled after the client is gone, not
before.** The order matters. The connection is taken out of the selector and out
of the client list *first*, and only then does the Server end that client's game
and tell the opponent. Done the other way round, ending the game tries to send
the result to the player who just vanished, which logs a confusing "could not
send" line about a client already reported as disconnected.

The other half of that is §64: a closed socket reports itself ready forever, so
a client that has gone must leave the selector as well as the list. Leaving it
in the selector turns the loop into a busy spin at full processor use.

**Finished games are dropped** once both players have been told. Nothing more
will ever be sent about them, and a server left running all day would otherwise
accumulate every game it had ever hosted.


## The Client's online game · `client/OnlineGame` · §22-25, §50, §56

**What it does.** Draws the position the Server sent, and turns clicks into
requests.

**Why it has no `ChessGame`, and what that buys.** §24 says the Client must
never apply an online move itself. The way to satisfy that is not to be careful
— it is to have nothing to be careful with. `OnlineGame` inherits `Game`, not
`ClientValidatedGame`, so there is no rules engine anywhere in the object and
no `makeMove` to call by accident. The board is 64 characters that are only
ever replaced wholesale from a `GameState` message.

So a click does not move a piece. It sends a `MoveRequest` and the screen does
not change. The piece appears to move only when the Server sends a position in
which it has moved, which is §24 stated as a mechanism rather than a promise.

**The tricky part — the window opens before there is a game.** The alternative
was to wait at the console for the pairing, but then cancelling a search (§42)
means watching the keyboard while blocked on a socket, which on Windows needs
platform-specific console code. Opening the board immediately, showing "Waiting
for opponent", and treating a window close as the cancel avoids all of
that and reuses the loop that already exists. The same idea covers walking out
of a live game: closing the window then sends `Resign`, because leaving a game
in progress concedes it, and the alternative is an opponent waiting forever for
a move that is not coming.

**Why the socket is polled and never blocked on.** The board window has to keep
drawing and answering the mouse while it waits for the Server. A blocking read
would freeze it exactly as a blocking engine search would in a Bot game — the
same problem as `StockfishEngine`, solved the same way.

**Two guards worth knowing.** The board is disabled unless it is genuinely this
player's turn *and* no earlier request is still unanswered, so a player cannot
fire off a second move believing the first has happened. And the promotion
dialog is decided from the picture on screen — is that my pawn, is it landing on
the far rank — which is display logic, not a rules decision: if the Client
guesses wrong the Server rejects the move anyway.


## Robustness · `client/main.cpp`, `client/OnlineGame`, `server/` · §64-70

Most of this was already standing, because online play could not be written
without it. What follows is what the pass over §64-70 actually added or settled.

**You cannot tell a TCP connection has died without using it.** This is the idea
behind `isServerStillThere`. `ServerConnection::isConnected()` is not a live
fact — it is the result of the *last* attempt to read or write, so it stays true
indefinitely after a Server has quietly gone away. §65 asks for disconnection to
be handled on the system screens as well as in a game, and the menu is exactly
where nothing is talking to the Server, so nothing would ever find out. One
non-blocking read settles it, and it is asked both before the menu is shown and
again when Online is chosen.

The reply to a lost connection is deliberately *not* to quit. §8 makes the menu
conditional on having connected, which this Client did; and Local and Bot games
never needed the Server at all, so shutting the program down would remove two
working modes to punish the loss of a third. It says so once and carries on.

There is a race here that cannot be designed away: the Server can die in the
moment between the check and the board window opening. That is handled rather
than prevented — the window opens and immediately shows "Lost connection to the
server" as the game's result. Both orderings are honest and neither hangs.

**§67 is two different problems.** A message can be malformed — a type byte that
is not ours, a body that is missing or truncated — and it can be perfectly well
formed but wrong for the moment it arrived in. The first is caught by
`isKnownMessageType` and by checking that every field actually read. The second
needs state: the Server refuses a move for a game the sender is not in, a draw
answer where no offer stands, a player answering their own offer, a second offer
while one is outstanding, a search from a client already playing. The Client
does the same in reverse, ignoring anything about a game before it has been told
one started, and a second `GameStarted` if it already has one.

**A known trade-off, worth being able to defend.** The Server's sends are
blocking. A client that stopped reading but kept its socket open could therefore
stall the single thread. Fixing that properly means an output queue per client
and non-blocking writes, which is a real amount of machinery for a case that
needs a deliberately hostile client. What §64 actually requires — that a client
*disconnecting* never brings the Server down or disturbs other games — is met
and tested, because a dropped socket fails the send immediately rather than
blocking. The simpler version was chosen knowingly, not by accident.

**Testing.** 81 automated checks against a running Server (103 after the Phase 7
audit added checkmate, promotion and bulk malformed input), plus the console
prompts driven with deliberate rubbish at every one: a bad address, an address
where a port belongs, a port out of range, words where numbers belong, and menu
choices above, below and outside the range. Every one is refused with a specific
message and re-asked, and nothing is left in a bad state.


## The quality pass · all of `common/`, `client/`, `server/` · §71-80

Ten requirements about the C++ rather than the chess. Mostly a reading and
measuring pass over code already written; what it actually turned up:

**§78 was measured, not argued.** The tempting answer to "are there leaks?" is
"there is no `new` anywhere, so no" — true, and worth saying, but it is an
argument rather than evidence. AddressSanitizer is not built into this MinGW
toolchain, so the check used the real binary instead: one Server process, six
complete runs of the protocol test through it, watching its memory and handle
count after each.

    run 1: private 1,140 KB   handles 94
    run 6: private 1,048 KB   handles 94

Flat, after roughly seventy client connections and two dozen games created and
finished. The handle count matters as much as the memory: it is what would climb
if a socket were ever not closed, so it is §76 demonstrated as well as §78.

**Two dead methods, found by counting references.** `HostedGame::hasPending
DrawOffer` and `ChessGame::legalMovesFrom` were each declared and defined and
called from nowhere — `legalMovesFrom` had been written for a
highlight-the-legal-moves feature the provided GUI turns out to do itself. Both
deleted. An unused public method is not harmless: it is code the student would
have to explain, and it invites the question of what it was for.

**Where `const` was actually wrong.** Only one place: `Server::describeById`
builds a string for the log and changes nothing, but was non-const because it
called `findClient`, which hands back a pointer the caller may write through.
Rather than add a second const overload of `findClient` and the usual
`const_cast` between the two, `describeById` now does its own const search — six
lines, and no clever pointer laundering to explain.

**Two `sf::Packet` parameters are taken by value on purpose**, which looks like
a §75 slip and is not. SFML's `send` takes a non-const reference and consumes
the packet as it writes, and every caller builds its message inline and passes
the temporary, which a reference cannot bind to. Both now say so in a comment,
because the alternative is a reader assuming it was carelessness.

**The protected members are a considered position, not an oversight** — see
DESIGN decision 6. Wrapping `gui_` in a protected accessor that returns a
mutable reference would give a subclass identical power while looking more
encapsulated, which is worse than being honest about it.


## The final audit · everything · all 89

A deliberate pass over the whole project looking for what was still only
assumed. What it changed:

**The requirements document was re-read in the original**, not through the
transcription, because a transcription error would be invisible from inside the
project. Two things came out of that.

The first is §25. Its full wording says the implementation must not assume every
Client runs on one computer — and then adds that *during checking it is also
possible to test using several Clients on the same computer*. So the requirement
is about the implementation, and testing on one machine is explicitly allowed.
That is now provable rather than asserted: `netstat` shows the Server listening
on `0.0.0.0`, not `127.0.0.1`, and `listen(port, sf::IpAddress::Any)` says so in
the source instead of relying on SFML's default. `127.0.0.1` appears nowhere in
the code except in printed help text.

The second is §88-89, the two compile commands. They are in the PDF as templates
with the source files left blank, and they had never been compared against ours
character by character. They are:

    g++ -std=c++17 ... -o client.exe  -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
    g++ -std=c++17 ... -o server.exe  -lsfml-network -lsfml-system

Ours match exactly, including the order of the link flags, and the lecturer's
own `build_example.bat` uses `-IProvidedGUI` in the same position we do. The
README's commands were also put on single lines, because a backslash
continuation works in the MSYS2 shell but not in `cmd` or PowerShell, and his
own build script is a `cmd` batch file. Both were then pasted into both shells
to confirm.

**Two gaps in testing were found and closed.** Promotion and checkmate had never
been exercised *through the Server* — only locally. The protocol test now plays
a game to checkmate, and marches a pawn to the eighth rank three ways: without
naming a piece, which must be refused; as a queen; and as a knight, which proves
the choice is honoured rather than hardcoded. Refusing the first matters more
than it looks: `Rules::isLegalMove` compares whole moves including the promotion
field, so a promoting move with no piece named is simply not in the legal list.
The failure mode is a rejection, never a pawn stranded on the last rank.

**A latent bug in `StockfishEngine`.** Both timeout helpers built their deadline
from `GetTickCount`, which wraps to zero after about 49 days of uptime; a
deadline computed across that wrap either expires immediately or never.
`GetTickCount64` throughout.

**Fuzzing, because crafted bad input only covers what somebody thought of.** The
protocol test now ends by sending 1,600 random packets from four Clients at
once, half of them starting with a plausible type byte so the per-message
parsers are reached and fed nonsense, then dropping two of those Clients
mid-stream. The Server survives, its memory and handle count are unchanged, and
it still pairs new Clients and validates real moves afterwards. That is §64 and
§67 tested rather than reasoned about.

**What was checked and found already correct**, so that it is not re-checked
later: every one of the GUI's ten event types is handled; the game-over box is
dismissed by the GUI itself, so `hideGameOver` is not ours to call; the board
loop cannot spin, because the GUI caps itself at 60 frames a second; the display
characters are exactly the set `setBoard` documents; the promotion rank
arithmetic is right for both colours; there is no non-ASCII in any string
literal, so nothing can arrive on the console as mojibake; and all 56 source and
documentation files are clean UTF-8 with no BOM. Both programs compile without a
single warning under `-Wall -Wextra -Wpedantic -Wshadow`, which is stricter than
anything the grader will use.

---

## Final audit against the PDF · README, `client/Game.cpp`, the zip · §39, §82-84

**What it did.** Went through all 89 sections of the PDF itself, in both
directions (anything missing, anything extra), and checked each one against the
code and the package rather than against our own docs. Full results, with
evidence per section, are in `docs/AUDIT.md`.

**How it was verified.** The zip was unpacked into a folder whose path has a
space and Hebrew letters in it, and run from there with MSYS2 removed from
`PATH`. Real Client windows were driven by posting mouse clicks to them at the
coordinates `ChessFrontend.cpp` draws its board and buttons at, and each window
was captured to an image to check what a player would actually see. That
covered a full online game (a move, an out-of-turn click, an illegal move, a
declined draw, a cancelled and then a confirmed resignation), cancelling a
search, a Local game, a Bot game, and an online game connected through the LAN
address instead of `127.0.0.1`.

**The one code fix — and why only a screenshot could find it.** Once a game
ended and the player closed the summary box, the side panel said *Turn: Game
over / Game status: Game in progress*. Our code cleared the status when the game
ended, and the provided GUI draws an empty status as "Game in progress". Nothing
in our code is wrong on its own; the problem only exists where the two meet, so
no test that looks at our code alone could see it. The panel now shows the
result title ("White wins!", "Draw", "Game ended"), all short enough for the
260-pixel panel.

**What left the zip.** `Example/`, `build_example.bat` and `GUI_USAGE.md`. §82
asks for `ProvidedGUI/` only, and a `.bat` file at the top of the submission
reads as the project's build script when it actually builds the lecturer's demo.
They stay in the repo, untouched.

**The README had nine wrong statements and four gaps**, none of them dramatic:
a missing-DLL claim that testing disproved (both programs fail the same way), a
cross-reference to the wrong section, on-screen text quoted differently from
what the screen shows, `./server.exe` offered to `cmd` users, which `cmd`
rejects; and nothing said about `g++` needing to be on `PATH`, whether
double-clicking the programs works, the firewall prompt on a second machine, or
DLLs clashing after a rebuild with another MSYS2 version. Each was checked by running the thing it describes.

---

## No DLLs in the submission · README, the zip · §82, §85, §88-89

**What changed.** The lecturer ruled that the submission must not include any
DLLs, so the 19 that used to sit beside the executables came out of the zip. The
README now runs everything, building and running alike, from the MSYS2 UCRT64
shell.

**Why the build did not change.** The programs still link SFML dynamically,
using exactly the commands §88-89 give. Static linking would have made the DLLs
unnecessary, but it adds flags to those commands, and the commands are the
lecturer's own. So the DLLs have to come from somewhere at run time instead, and
the UCRT64 shell provides them: it puts `C:\msys64\ucrt64\bin`, where pacman
installed SFML, on `PATH`. That is also the environment §85 names, so asking the
grader to use it asks for nothing new.

**The tricky part — what the grader sees if they do it another way.**
Double-clicking the executable, or starting it from `cmd` or PowerShell, uses a
`PATH` that normally lacks that folder. Windows then refuses to start the
program before `main` runs, either with a "was not found" dialog or silently
with exit code `0xC0000135`. README section 3 says so in as many words, because
a program that silently does nothing looks like a crash in our code. The same
applies to a second computer used for §25: it needs the MSYS2 installation too.

**Verified** by unzipping the new zip into a fresh folder. Outside the UCRT64
shell, both programs exit `0xC0000135`, which proves no DLL is hiding in the
package. Inside it, `./server.exe` and `./client.exe` run exactly as README
sections 4 and 5 say. With that shell's `PATH`, an online game, a Local game and
a Bot game all played through; Stockfish ran from the package's own folder and
was gone when its window closed. Both README build commands also work there.
