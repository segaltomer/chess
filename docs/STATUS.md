# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 3 — the Bot (§18-20, §43-47). **Done and confirmed at the board.**
Phases 1 and 2 are done. **Next up: Phase 4, the Server and Online play.**

**In progress:** nothing.

**Files mid-edit:** none. Working tree clean.

The student confirmed the Bot replies to every move and that Offer Draw is
absent in Bot mode. The strength difference was deliberately *not* confirmed by
eye — the student does not play chess well enough to feel it, and an impression
would not be evidence. `tests/bot_match.cpp` measures it instead; see below.

## What is done

**Phase 1 — the chess rules, in `common/`.** Complete and verified.

| File | What it holds |
|---|---|
| `ChessTypes.h/.cpp` | `Color`, `PieceType`, `Piece`, square geometry, display characters, FEN square names |
| `Move.h/.cpp` | a from/to/promotion triple, and its UCI spelling |
| `Board.h/.cpp` | the 64 squares, castling rights, en passant target, `applyMove`, FEN reading |
| `PieceRules.h/.cpp` | abstract base + one class per piece type; how each moves and what it attacks |
| `Rules.h/.cpp` | attacked squares, check, legal moves, castling generation, checkmate, stalemate |
| `ChessGame.h/.cpp` | the position, the turn, the result and the reason — the authority |

**Phase 2 — the Client and the Local Game.** Complete and **confirmed at the
board by the student**: ordinary and illegal moves, moving out of turn, check,
promotion, castling, en passant, resign with both Confirm and No, draw offer
with both Accept and Decline, and closing the window mid-game. All correct.

| File | What it holds |
|---|---|
| `server/Server.h/.cpp`, `server/main.cpp` | listener, accepts clients, prints its address (§11) |
| `client/ConsoleInput.h/.cpp` | console prompts with the checking §67 wants |
| `client/ServerConnection.h/.cpp` | the socket, closed by its destructor (§76) |
| `client/ConnectionScreen.h/.cpp` | §7, §9, §10 — the gate in front of the menu |
| `client/MainMenu.h/.cpp` | the four options (§8, §12) |
| `client/Game.h/.cpp` | the shared board loop every mode uses (DESIGN decision 2) |
| `client/LocalGame.h/.cpp` | a two-human game — only what differs (§13, §17) |

**Phase 3 — the Bot.** Code complete.

| File | What it holds |
|---|---|
| `client/StockfishEngine.h/.cpp` | the Stockfish subprocess and UCI (§45-46); the only Windows-specific file |
| `client/BotDifficulty.h/.cpp` | the three levels as data, and the menu that picks one (§43) |
| `client/BotGame.h/.cpp` | a game against the engine, validated by our own rules (§18, §47) |

`tests/perft_main.cpp` is the verification harness. It does not ship.

## Verification status

**The rules engine passes the full standard perft suite — all 26 cases, about
15 million positions.** Start position depth 5 = 4,865,609; Kiwipete depth 4 =
4,085,603. Re-run this after any change under `common/`:

    g++ -std=c++17 -O2 -Icommon common/*.cpp tests/perft_main.cpp -o perft.exe
    ./perft.exe

Alongside it, roughly 180 direct assertions were run while building, covering
piece movement and blocking, turn order, check and pins, checkmate and
stalemate, all six castling conditions, en passant including the capture that
would expose your own king, promotion including under-promotion, and the result
and reason strings. Those lived in the scratch directory — perft is the
regression test that was kept, because it is the one that matters.

The Client and Server were tested together: the connection screen's success
path and all three recovery paths, rubbish typed at every prompt, the menu
gate, and the server surviving several clients connecting, sending nonsense
and disconnecting in any order.

**The Bot was verified against the real engine:**

- FEN round-trips for four positions, including the en passant field.
- Stockfish starts, completes the UCI handshake, and finds a mate in one.
- It honours its time limit (912 ms for a 900 ms budget).
- **120 engine half-moves all accepted by `ChessGame::makeMove`** — the §47
  check, and an independent cross-check of our move generator.
- **The levels were measured, not assumed** — `tests/bot_match.cpp`, kept in
  the repo and re-runnable:

      g++ -std=c++17 -O2 -Icommon -Iclient common/*.cpp client/StockfishEngine.cpp \
          client/BotDifficulty.cpp client/ConsoleInput.cpp tests/bot_match.cpp \
          -o bot_match.exe
      ./bot_match.exe

  Six games, each pairing from both colours. The stronger level won all six by
  checkmate; material at the end as lopsided as 33 against 9. It calls the
  shipped `settingsFor()`, so changing a difficulty constant re-tests it.
- Stockfish runs as a separate process and **is gone once the client exits**,
  so the RAII shutdown leaves nothing orphaned. Confirmed live: with a Bot game
  open, `stockfish.exe` appears in Task Manager's **Details** tab with
  `client.exe` as its parent process.

## Next step — Phase 4, the Server and Online play

The biggest phase, and the one the requirements are strictest about. Before
writing any of it, re-read §22-24 and DESIGN decisions 4 and 5.

The two things that must be true by construction, not by care:

- **§22-24.** The Server owns the board, the turn and the validation. The
  Client never applies an online move itself and never treats its own request
  as final. In practice: the Client's `OnlineGame` will have **no `ChessGame`
  of its own** — only the last board the Server sent. That is what DESIGN
  decision 2 was rearranged for, and it means the Client physically cannot
  cheat rather than merely choosing not to.
- **§60.** Never assume one game exists. Games live in a container keyed by id
  from the first line written, and every message carries its game id so §59 —
  never tell a client about a game it is not in — is checkable in one place.

`OnlineGame` will subclass `Game` like the other two, overriding
`onOpponentTurn` to poll the Server rather than an engine.

Phase 4 cannot be tested alone: it needs **two clients and one server running
at once**, which the student must drive.

## Environment notes for the next session

`git` is **not** on `PATH` in this shell. Two bundled copies exist; the
GitHub Desktop one is newer:

    $env:PATH = "C:\Users\Owner\AppData\Local\GitHubDesktop\app-3.6.5\resources\app\git\cmd;" + $env:PATH

`g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.

Commit messages must be passed with `git commit -F <file>`, not `-m`, because
PowerShell mangles arguments containing double quotes when handing them to a
native executable.

**Never rewrite a UTF-8 doc through PowerShell** (`Get-Content -Raw` then
`WriteAllText`, or `Set-Content`). PowerShell 5.1 reads UTF-8 as ANSI and
writes it back double-encoded, turning every `§` into mojibake. Use the editing
tools instead. This has already happened once to `docs/PLAN.md`.

**`git push` cannot work from this shell, by design of the machine's setup.**
Windows Credential Manager holds no `git:https://github.com` entry; GitHub
Desktop keeps its token in its own private format (`LegacyGeneric:target=GitHub
- https://api.github.com/segaltomer`) which plain `git` cannot read. So a push
from here has to prompt for a username, and this shell has no interactive
input: it fails with `could not read Username`.

**This is not a problem as long as the student pushes from GitHub Desktop**,
which is what `docs/CLAUDE_CODE.md` §6 already tells them to do. Commit
locally, tell them a commit is ready, and they push.

To make pushing work from a shell one day: install Git for Windows (it puts
`git` on `PATH` and ships Git Credential Manager), then run `git push` once in
a normal terminal and sign in. The token then lands in the Windows vault and
any shell can use it.
