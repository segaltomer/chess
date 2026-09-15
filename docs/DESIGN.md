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
square holds a small **value** — a `struct Square { PieceType type; Color
colour; }` where `PieceType::None` means empty — not a piece object.

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

**Decision:** a base `Game` class holding the board, the rules and the turn,
with three subclasses: `LocalGame`, `BotGame`, `OnlineGame`.

**Why:** the three modes differ in exactly one thing — who decides a move.
Local asks the human at the same keyboard, Bot asks Stockfish, Online asks the
Server. Everything else (legality, check, end conditions) is identical, per §6.
A base class puts the shared 90% in one place and confines each mode's
difference to one small file, instead of scattering `if (mode == Online)`
branches through a single class, which §71 penalises.

This matters most for Online: §22–24 make it a genuinely different object — it
does not own the board of record, it mirrors one.

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

Message types needed (§62):

| Direction | Message | Carries |
|---|---|---|
| C→S | `Hello` | — |
| C→S | `FindGame` | — |
| C→S | `CancelFindGame` | — |
| S→C | `Waiting` | — |
| S→C | `GameStarted` | game id, my colour, opponent name |
| C→S | `MoveRequest` | game id, from, to, optional promotion |
| S→C | `GameState` | game id, 64 squares, side to move, status text |
| S→C | `MoveRejected` | game id, reason |
| C→S | `OfferDraw` | game id |
| S→C | `DrawOffered` | game id |
| C→S | `DrawResponse` | game id, accepted |
| C→S | `Resign` | game id |
| S→C | `GameOver` | game id, result, reason |
| S→C | `OpponentDisconnected` | game id |

Every message carries the game id, so §59 — never send an update to a client
outside that game — is checkable in one place.

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
