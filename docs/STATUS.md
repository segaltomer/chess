# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 7 — submission (§82-84). **Done. The zip is built and tested.**
Phases 1-6 are done. **88 of the 89 requirements are complete.**

**`216908467.zip` is in the project root** — 82.3 MB, 89 entries, everything
inside one top-level folder `216908467/`. It is gitignored, so rebuild it from
the whitelist in `docs/PLAN.md` if anything changes.

**The one requirement left is §25 — online play across two computers.** Server
and one Client here, a second Client on another machine on the same network,
using the LAN address the Server prints. The student asked to be reminded of
this at the end; this is that reminder, and it is the only thing between the
project and finished. Nothing in the code assumes one machine, and two Clients
on this one already play each other through the Server, but the requirement asks
for two machines and only a second machine can answer it.

If it turns out the two machines cannot see each other, the usual cause is
Windows Firewall rather than the code: allow `server.exe` on private networks
when prompted, and check both machines are on the same network rather than one
being on a guest or mobile connection.

**In progress:** nothing.

**Files mid-edit:** none. Working tree clean.

Online play has been confirmed at the board through two real client windows.

**The one open item:** §25 — run the Server and one Client on this machine and a
second Client on another machine on the same network. It is the only requirement
that cannot be tested on one computer at all.

**Two findings from that play-test, both now fixed or settled:**

- The GUI's side panel is 260 pixels wide and neither wraps nor truncates, so
  three captions ran off the edge of the window. Every string the Client puts in
  that panel has now been measured in the GUI's own font and sizes; the
  move-rejection reasons are worded on the Server to fit it.
- **The board is not flipped for Black, and should not be.** Checked against the
  requirements document itself, not assumed — see DESIGN decision 1 for the
  evidence. Do not "fix" this later.

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
| `client/ClientValidatedGame.h/.cpp` | the layer that owns a `ChessGame`; Local and Bot only (§17-18) |
| `client/LocalGame.h/.cpp` | a two-human game — only what differs (§13, §17) |

**Phase 3 — the Bot.** Code complete.

| File | What it holds |
|---|---|
| `client/StockfishEngine.h/.cpp` | the Stockfish subprocess and UCI (§45-46); the only Windows-specific file |
| `client/BotDifficulty.h/.cpp` | the three levels as data, and the menu that picks one (§43) |
| `client/BotGame.h/.cpp` | a game against the engine, validated by our own rules (§18, §47) |

**Phase 4 — the Server and Online play.** Code complete.

| File | What it holds |
|---|---|
| `common/Protocol.h/.cpp` | every message, written and read by the same operators (§61-63) |
| `server/HostedGame.h/.cpp` | one online game; the Server's authority over it (§22) |
| `server/Matchmaker.h/.cpp` | the queue of clients waiting to be paired (§40-42) |
| `server/Server.h/.cpp` | sockets, dispatch, the §59 guard, console logging (§57-60, §68-70) |
| `client/OnlineGame.h/.cpp` | draws server state, sends requests, owns no rules engine (§23-24) |

`tests/perft_main.cpp`, `tests/bot_match.cpp` and `tests/protocol_test.cpp` are
the verification harnesses. None of them ships.

## Verification status

**The rules engine passes the full standard perft suite — all 26 cases, about
15 million positions.** Start position depth 5 = 4,865,609; Kiwipete depth 4 =
4,085,603. Re-run this after any change under `common/`:

    g++ -std=c++17 -O2 -Icommon common/*.cpp tests/perft_main.cpp -o perft.exe \
        -lsfml-network -lsfml-system
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
          -o bot_match.exe -lsfml-network -lsfml-system
      ./bot_match.exe

  Six games, each pairing from both colours. The stronger level won all six by
  checkmate; material at the end as lopsided as 33 against 9. It calls the
  shipped `settingsFor()`, so changing a difficulty constant re-tests it.
- Stockfish runs as a separate process and **is gone once the client exits**,
  so the RAII shutdown leaves nothing orphaned. Confirmed live: with a Bot game
  open, `stockfish.exe` appears in Task Manager's **Details** tab with
  `client.exe` as its parent process.

**Phase 5 — robustness (§64-70).** Complete. Most of it was already standing,
since online play could not be written without it; the pass added the menu-level
disconnection check (§65), tightened the Client against messages that do not fit
its current state (§67), and verified §69's list of logged events item by item.

**Online play and robustness were verified by an automated protocol test** — a
scratch program that connects as several clients at once and checks the Server's
answers. **81 checks, all passing**, over: matchmaking and the waiting state;
colour assignment; moves played out of turn and moves that are simply illegal,
both rejected by the Server; two games running at once with no cross-talk; a
client aiming a move at a game it is not in; draw offers accepted and declined;
resignation; a client disconnecting mid-game and a client disconnecting while
waiting to be paired; six messages that are well formed but wrong for the
current state; and malformed input of four kinds, after which the Server still
pairs new clients.

Separately, every console prompt was driven with deliberate rubbish — a bad
address, an address where a port belongs, a port out of range, words where
numbers belong, and menu choices above, below and outside the range. Each is
refused with a specific message and re-asked.

**Phase 6 — the quality pass (§71-80).** Complete. Searched rather than assumed:
no `new`/`delete`/`malloc`/`free` anywhere, no absolute paths, no TODO/FIXME, no
commented-out code, no debug printing, and nothing in `common/` writes to the
console. §78 and §76 were demonstrated on the real binary — six full protocol
workloads through one server process left memory flat at 1,048 KB and the handle
count constant at 94. Two dead methods were found by counting references and
deleted (`HostedGame::hasPendingDrawOffer`, `ChessGame::legalMovesFrom`), one
method was made `const`, and a `-Wcomment` warning was fixed in all three test
harnesses. Both programs build clean under `-Wall -Wextra`.

It is kept, as `tests/protocol_test.cpp`, alongside perft and the Bot match.
Unlike those two it needs a Server to talk to, so it takes two windows:

    g++ -std=c++17 -O2 -Icommon common/*.cpp tests/protocol_test.cpp \
        -o protocol_test.exe -lsfml-network -lsfml-system
    ./server.exe 53100            one window
    ./protocol_test.exe 53100     another

It exits 0 only if all 81 checks passed. Re-run it after any change to the
protocol, the Server, or `client/OnlineGame`.

## How to test it — two clients and one server

Three console windows, all opened at the project root:

    .\server.exe          window 1
    .\client.exe          window 2
    .\client.exe          window 3

Both clients connect to `127.0.0.1` on the port the server printed, then choose
**3) Online Game**. The first sees "Waiting for an opponent..."; when the second
asks, both board windows open with colours assigned. Worth checking: moving out
of turn does nothing, a draw offer appears only on the other window, resigning
ends it on both, and closing one client mid-game ends the other's game with
"disconnected" rather than "resigned".

## Phase 7 — what has been checked

**The submission was staged and run, not just assembled.** Everything on the
whitelist in `docs/PLAN.md` was copied into a clean folder and then run there
**with MSYS2 removed from `PATH`** — which is the real test of whether the 19
bundled DLLs are the right ones:

- `server.exe` starts and prints both addresses and the port.
- `client.exe` connects, reaches the main menu and exits cleanly.
- A Bot game launches `stockfish.exe` **from the staged folder's own
  `stockfish/`**, which is the proof that the path is resolved relative to the
  executable rather than hardcoded. No orphan process is left behind.

**The zip fits after all.** This had been flagged as a possible blocker needing
the lecturer. It is not: `stockfish.exe` is 98 MB but compresses, and the whole
submission comes to **82.3 MB** zipped, 110 MB unpacked.

**`tests/` now ships**, reversing the earlier decision. §20 asks the Bot's
difficulty levels to be *demonstrable*, and `bot_match` is what demonstrates
them; and the README documents all three harnesses, so omitting them would mean
documenting commands for files the checker does not have. Reasoning is in
`docs/PLAN.md`.

## The last step, once the ID is known

Build the zip from the whitelist by copying items in — never by copying the
project folder and deleting. The PDF asks for the files "in a folder named with
your ID", so put everything inside one top-level folder of that name and call
the archive `<ID>.zip`; that satisfies both that wording and §84's.

## Environment notes for the next session

`git` is **not** on `PATH` in this shell. Two bundled copies exist; the
GitHub Desktop one is newer:

    $env:PATH = "C:\Users\Owner\AppData\Local\GitHubDesktop\app-3.6.5\resources\app\git\cmd;" + $env:PATH

`g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.

Commit messages must be passed with `git commit -F <file>`, not `-m`, because
PowerShell mangles arguments containing double quotes when handing them to a
native executable.

**Reading `docs/requirements.pdf` (the authority) from a shell.** There is no
`pdftotext`, no Python, and driving Word through COM hangs. What works: read the
file's bytes, inflate each `stream ... endstream` with
`System.IO.Compression.DeflateStream` (skipping the two-byte zlib header), then
decode the `[...] TJ` runs through the `ToUnicode` CMap built from the
`beginbfchar` and `beginbfrange` blocks. The Hebrew comes out legible but
visually reversed, so searching it means searching for reversed spellings.

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
