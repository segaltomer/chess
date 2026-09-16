# Design decisions

Decided by me from explained options. Build against these.

**These are choices, not rules.** Some I picked from options that were
explained to me, some were sensible defaults. I don't know C++ architecture
well, so if there is a clearly better way — simpler, fewer bugs, less
duplication — take it. Tell me what you changed and why, and rewrite the
decision here so this file always describes the code as it actually is.

What is *not* open to change: anything required by the lecturer's document.
`CLAUDE.md` lists those separately, and `docs/REQUIREMENTS.md` has all 89.
A better idea never justifies breaking one of those.

---

## 1. Board and move representation

**Decision:** one flat list of 64 squares, index 0 = a8, index 63 = h1. Each
square holds a small **value** — a `struct Piece { PieceType type; Color
colour; }` where `PieceType::None` means empty — not a piece object.

> **Renamed.** This struct was called `Square` when the decision was written.
> In the code it became `Piece`, because "square" was already doing a second
> job: a square *index* (`int fromSquare`, 0-63) is the thing most of the
> engine passes around, and having a type called `Square` that is not that was
> confusing on sight. `board.pieceAt(fromSquare)` says what it means. Nothing
> about the decision changed — it is still a value, not an object.

**Why the flat 64:** it is exactly the shape `ChessFrontend::setBoard` already
takes (`std::array<char, 64>`, same indexing), so there is no conversion layer
between engine and screen and no chance of an off-by-one flip. Row and column
come from `index / 8` and `index % 8`.

**Why values and not piece objects** (corrected — see below): checking legality
means asking "if I made this move, would my own king be attacked?" (§31), which
in turn is what checkmate and stalemate are defined against (§32-33). The
cheapest way to answer that is to copy the board, apply the move, and test. With
plain values a board copy is a trivial memcpy of 64 small structs. With
`std::unique_ptr<Piece>` per square, every copy needs a deep clone of up to 32
heap objects, on every candidate move — slow, and far more code to get wrong.

> **Correction.** This decision originally said `std::array<Piece, 64>` while
> decision 6 asked for a `Piece` base class with a subclass per piece type.
> Those two cannot both be true: a polymorphic base class cannot be stored by
> value in an array — it would slice off the subclass. The fix keeps both ideas
> by separating the board's *data* from each piece type's *behaviour*; see
> decision 6.

A `Move` carries a from-square, a to-square, and an optional promotion choice.

---

## 2. Who owns a game

**Decision:** the position, the turn and the rules live in one class,
`ChessGame`, in `common/`. The three game modes are a separate `Game` hierarchy
in the Client — `LocalGame`, `BotGame`, `OnlineGame` — and each **holds** a
`ChessGame` where it needs one, rather than inheriting from it.

**Why:** the three modes differ in exactly one thing — who decides a move.
Local asks the human at the same keyboard, Bot asks Stockfish, Online asks the
Server. Everything else (legality, check, end conditions) is identical, per §6.
A base class puts the shared behaviour in one place and confines each mode's
difference to one small file, instead of scattering `if (mode == Online)`
branches through a single class, which §71 penalises.

**Why holding and not inheriting** (changed while building Phase 1): this
decision originally put the board, the rules and the turn *in* the `Game` base
class, so all three modes would inherit them. That is wrong for Online, and
wrong in a way that matters. §23–24 say the Client must never apply an online
move itself and must never treat its own request as final — but a `Game` base
that owns a board and validates moves hands `OnlineGame` exactly the method
those requirements forbid it to use. Inheriting a capability you are required
not to exercise is a trap, not a design.

Separating them means the Client's `OnlineGame` simply has no `ChessGame` to
mis-use: it holds the last board the Server sent and nothing else. The Server
holds the `ChessGame` for that match, because §22 says the Server validates.
Local and Bot games hold their own, because §17–18 say the Client validates
those. Each mode ends up with exactly the authority its requirement gives it.

**As built (Phase 4).** There are three levels, and the middle one exists
precisely to keep the promise above.

```text
Game                  the board window and the shape of an interaction
├── ClientValidatedGame   owns a ChessGame; validates here (§17-18)
│   ├── LocalGame
│   └── BotGame
└── OnlineGame            owns no ChessGame; the Server validates (§22-24)
```

`Game` holds the whole shared loop: reading clicks, turning them into moves, the
promotion dialog, the resign confirmation, the draw offer, and deciding when to
show the result. `play()` is not virtual — subclasses cannot reimplement the
parts that must behave identically in every mode (§6). What `Game` deliberately
does **not** hold is a `ChessGame`.

> **Changed in Phase 4.** Through Phase 3 the base class *did* hold one, because
> both modes that existed then wanted it. That directly contradicted the
> paragraph above, and the contradiction would have surfaced as a bug: a
> two-level hierarchy would have handed `OnlineGame` an inherited `makeMove`,
> which is the exact call §24 forbids it. Adding the middle class instead of
> writing "remember not to call that" is the whole point — `OnlineGame` now has
> no rules engine to misuse, and the compiler is what enforces it rather than
> care.

The hooks come in two kinds: ones asking the mode what to draw
(`boardForDisplay`, `isOver`, `turnText`, `statusText`, `resultTitle`,
`resultDetail`), and ones telling it what the player asked for (`onMoveChosen`,
`onResignConfirmed`, `onDrawOffered`, `onDrawResponse`, `onOpponentTurn`,
`needsPromotionChoice`). That split is what lets the same loop serve a mode that
decides moves itself and one that may only ask.

`ClientValidatedGame` answers every question of the first kind from its
`ChessGame`, and turns every request of the second kind into a call on it.
`LocalGame` is then 30 lines and `BotGame` about 90.

`OnlineGame` answers the first kind from the last `GameState` the Server sent —
64 characters it replaces wholesale and never edits — and turns the second kind
into messages. A click sends a `MoveRequest` and changes nothing on screen; the
piece moves only when a position in which it has moved comes back.

The base class was deliberately **not** written in Phase 2, when only
`LocalGame` existed, and the split above was not written in Phase 3 either. Each
seam was cut when a second real case made it visible, rather than guessed at
from one.

---

## 3. Where the chess rules live

**Decision:** `common/` holds the board, the piece classes, move generation,
the rules, and the protocol message definitions. `client/` holds the console
screens, the GUI wiring and the client networking. `server/` holds the
listener, matchmaking and online game hosting.

**Why:** forced by the requirements, not a preference. The Client validates
Local and Bot games (§17–18), the Server validates Online games (§22), and §6
says the rules are identical in every mode. So the rules engine compiles into
both binaries. The build commands already reflect this:
`g++ … common/*.cpp client/*.cpp …` and `g++ … common/*.cpp server/*.cpp`.

Writing the rules twice is the failure mode this avoids.

---

## 4. Protocol message format

**Decision:** `sf::Packet`, with a leading `enum class MessageType` followed by
typed fields.

**Why:** networking is not what this course is about, so this layer should cost
as little debugging time as possible. `sf::Packet` handles message framing —
it tracks where one message ends and the next begins — and packs integers and
strings without hand-written parsing. Plain text over TCP would mean buffering
partial reads and splitting on newlines myself, which is a well-known source of
bugs for no benefit here.

Message types, as built (§62). All of it lives in `common/Protocol.h`, written
and read by the same operators so the two ends cannot drift apart:

| Direction | Message | Carries |
|---|---|---|
| C→S | `FindGame` | — |
| C→S | `CancelFindGame` | — |
| C→S | `MoveRequest` | game id, from, to, optional promotion |
| C→S | `OfferDraw` | game id |
| C→S | `DrawResponse` | game id, accepted |
| C→S | `Resign` | game id |
| S→C | `Waiting` | — |
| S→C | `GameStarted` | game id, my colour, opponent name |
| S→C | `GameState` | game id, 64 squares, side to move, in check |
| S→C | `MoveRejected` | game id, reason |
| S→C | `DrawOffered` | game id |
| S→C | `DrawDeclined` | game id |
| S→C | `GameOver` | game id, result title, reason |

Every in-game message carries the game id, so §59 — never send an update to a
client outside that game — is checkable in one place. On the Server that place
is `findGameForPlayer`, which refuses to find a game the asking client is not
playing in.

> **Three changes from the list first written here.**
>
> `Hello` is gone. It carried nothing: the TCP connection *is* the hello, and
> the Server assigns the client its id on accept. An unused message is protocol
> surface that still has to be explained and kept working.
>
> `OpponentDisconnected` is gone, folded into `GameOver`. A disconnection ends
> the game, and `GameOver` already carries a result and a reason — so it says
> "White disconnected" the same way it says "White resigned", and the Client
> needs one ending path rather than two. This did mean adding
> `GameEndReason::Disconnection` to `ChessGame`, because telling the remaining
> player their opponent *resigned* when the network dropped would be a lie.
>
> `DrawDeclined` is new. Without it, a player who offers a draw and is refused
> is told nothing at all and is left wondering whether the offer ever arrived.
>
> `GameState` sends the side to move rather than a finished caption, because the
> two Clients word it differently — "Your move" against "Player 4 to move" — and
> only each Client knows which of the two it is.

---

## 5. What the Server holds

**Decision:** one thread, using `sf::SocketSelector`. State: a container of
connected clients, a container of active games keyed by game id, a matchmaking
queue of client ids, and a map from client id to the game it is in.

**Why:** a single thread with a selector means there is no shared state between
threads, therefore no locks, therefore an entire category of crash cannot
happen. §64 says a client disconnecting must never bring the Server down, and
thread-per-client with unlocked shared game state is the classic way to fail
exactly that requirement. Again: networking is not the focus here, so the
version with fewer ways to go wrong wins.

Games live in a container from the first line of code, never as single
variables on the Server — §60 forbids assuming one game exists, and retrofitting
that means rewriting the Server.

---

## 6. Where polymorphism is used

**Decision:** two places, both load-bearing.

1. **Piece movement rules** — a `PieceRules` abstract base with `PawnRules`,
   `RookRules`, `KnightRules`, `BishopRules`, `QueenRules`, `KingRules`. Each
   knows how its own piece moves, so fixing one piece touches one file instead
   of one arm of a six-way switch.

   These objects are **stateless**: they hold no position and no colour, they
   answer questions *about* a board ("from this square, on this board, as this
   colour, what are your target squares?"). One instance of each exists, looked
   up by `PieceType`. That is what lets the board itself stay a cheap array of
   values — behaviour is polymorphic, data is not.
2. **Game modes** — `Game` base with `LocalGame`, `BotGame`, `OnlineGame`, as
   in decision 2.

Bot difficulty is **not** a class hierarchy — it is three sets of Stockfish
parameters, because the three levels differ only in values passed to the engine,
not in behaviour. §73 asks for polymorphism where it suits the design; forcing
it here would be decoration.

---

---

## 7. Positions are built from FEN

**Decision:** `Board::fromFen` and `ChessGame::fromFen` read FEN, the standard
one-line text notation for a chess position, and `ChessGame::toFen` writes it.

> The writer was deliberately left out until something needed it. Phase 3 is
> when that happened: `position fen ...` is how Stockfish is told where the
> game stands. It also gave the reader a free test — every position now
> round-trips, text to board to text.
>
> The half-move and full-move counters are written as a fixed `"0 1"` rather
> than tracked. They exist for the fifty-move rule, which is not among the
> required rules (§26-39) and is not implemented, so keeping them would be
> state nothing ever reads.

**Why:** two things need it, and one of them is the reason the rules engine can
be trusted at all.

The move generator is verified with **perft** — play every legal move to a
given depth, count the resulting positions, and compare against the published
count for that position. The numbers are agreed on across every chess engine in
existence, so matching them is real evidence rather than "the tests I thought
to write pass". The test positions are published *as FEN*, and they are dense,
deliberately awkward positions. Transcribing five of those by hand into
`setPiece` calls would add a second thing that can be wrong: when a count then
disagreed, there would be no way to tell a bug in the engine from a typo in the
position. Forty lines of parser removes that whole class of doubt.

Second, Phase 3 has to tell Stockfish where the game stands, and `position fen
<...>` is how UCI does that. The alternative, `position startpos moves ...`,
would mean `ChessGame` keeping a full move history purely for the engine's
benefit.

**Where it lives:** the board fields are parsed by `Board` and the side to move
by `ChessGame`, each parsing the part it owns, so neither needs public setters
for state that should stay private.

---

## 8. The rules are free functions, not a class

**Decision:** `namespace Rules` holds `isSquareAttacked`, `isInCheck`,
`pseudoLegalMoves`, `legalMoves` and `isLegalMove` as free functions.

**Why:** none of them has any state. Every answer depends only on the board
passed in, so there is nothing for an object to own, no constructor that would
mean anything and no invariant to protect. A class whose members are all static
is a namespace with extra ceremony, and §71–72 are about putting
responsibilities where they belong, not about making everything a class. The
piece rules *are* classes, because there the polymorphism is load-bearing
(decision 6); these are not, because there is nothing to dispatch on.

---

## Consequences to hold to while building

- **§24, by construction:** the board the Client draws for an online game lives
  in a variable that is only ever *overwritten wholesale from a `GameState`
  message* — never edited by the Client. A local click sends a `MoveRequest`
  and changes nothing on screen. Then §24 is true by design rather than by
  remembering to be careful. Use `setBoardInputEnabled(false)` while a request
  is in flight.
- **Stockfish** is reached through one small class wrapping the subprocess and
  its pipes, in its own `.h`/`.cpp`. That keeps the only Windows-specific code
  in the project confined to a single file.
- **No absolute paths anywhere** — the project is graded on another machine.
