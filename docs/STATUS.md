# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 2 — Client, Local Game. Phase 1 is complete.

**In progress:** nothing.

**Last completed:** Phase 2 items 5-9 — the main menu (§8, §12), the GUI event
loop, the Local Game (§13, §17), resignation with its Confirm/No dialog
(§53-56), and the draw offer answered on the same client (§48-49, §51-52). New
files `client/MainMenu.h/.cpp` and `client/LocalGame.h/.cpp`.

Everything in Phase 2 is written and builds. **One item is deliberately left
unticked: nobody has played a game yet.**

**What has been verified, by running both programs together:** the connection
screen's success path and all three recovery paths; rubbish typed at every
prompt; the menu appearing only after a connection succeeds; each menu option
including bad choices and Exit; and the board window opening, staying
responsive, and running its loop at the GUI's own 60fps limit rather than
spinning. The server's log confirms every connection and clean disconnection.

**What has NOT been verified: anything that needs a mouse.** No piece has been
clicked. The move, promotion, resign and draw paths are written against the
tested rules engine and they compile, but they have never been exercised.

**Files mid-edit:** none. Working tree clean.

## What exists now

`common/` holds the whole rules engine and nothing else:

| File | What it holds |
|---|---|
| `ChessTypes.h/.cpp` | `Color`, `PieceType`, `Piece`, square geometry, display characters |
| `Move.h/.cpp` | a from/to/promotion triple, and its UCI spelling |
| `Board.h/.cpp` | the 64 squares, castling rights, en passant target, `applyMove`, FEN reading |
| `PieceRules.h/.cpp` | abstract base + one class per piece type; how each moves and what it attacks |
| `Rules.h/.cpp` | attacked squares, check, legal moves, castling generation, checkmate, stalemate |
| `ChessGame.h/.cpp` | the position, the turn, the result and the reason — the authority |

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
and reason strings. Those live in the scratch directory rather than the repo —
perft is the regression test that matters, and it is the one kept.

**Not yet exercised by a human.** Nothing has been seen on screen yet; there is
no GUI, no Client and no Server. That starts in Phase 2.

## Next step — this one is the student's

**Play a local game and report what happens.** Everything else is blocked on
it: Phase 3 builds the Bot on top of this same board loop, so a fault here
would be inherited.

    ./server.exe          in one console
    ./client.exe          in another, from the project root

Connect to 127.0.0.1 on the port the server printed, choose Local Game, and
try each of these:

- an ordinary move, and an illegal one (the piece should simply stay put)
- moving a piece when it is not that colour's turn
- putting a king in check — "Check" should appear in the status area
- a pawn reaching the far rank — the promotion dialog should appear, and all
  four pieces should work
- castling, both sides
- en passant
- Resign, then "No" — the game must carry on
- Resign, then "Confirm" — the other colour should win, and the summary should
  say who resigned
- Offer Draw, then Decline — play continues
- Offer Draw, then Accept — the game should end as a draw
- closing the board window mid-game — it should return to the console menu

Report anything that looks wrong in the words of what you saw, as
`docs/CLAUDE_CODE.md` §3 describes.

## After that

Phase 3, the Bot. Two things to settle when it starts:

- `LocalGame` and `BotGame` will differ in almost nothing — only in who
  supplies Black's move. The `Game` base class of DESIGN decision 2 should be
  extracted then, when there are two real cases to generalise from, rather than
  guessed at now from one.
- What the Client does if the connection drops during a Local or Bot game.
  Neither mode needs the Server at all (§17-18), so losing it should not
  disturb them. Phase 5 covers it properly.

**This is the first point where a person has to look at the screen.** The
rules engine was verifiable by counting positions; a GUI is not. See
`docs/CLAUDE_CODE.md` §4 — clicking pieces and reporting what happened is the
part only the student can do.

To run the server while testing a client:

    ./server.exe

It prints the IP and port to type into the connection screen.

## Environment notes for the next session

`git` is **not** on `PATH` in this shell. The working copy is the one bundled
with Visual Studio; prefix git commands with:

    $env:PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd;" + $env:PATH

`g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.

Commit messages must be passed with `git commit -F <file>`, not `-m`, because
PowerShell mangles arguments containing double quotes when handing them to a
native executable.

**`git push` cannot work from this shell, by design of the machine's setup.**
Windows Credential Manager holds no `git:https://github.com` entry; GitHub
Desktop keeps its token in its own private format (`LegacyGeneric:target=GitHub
- https://api.github.com/segaltomer`) which plain `git` cannot read. So a push
from here has to prompt for a username, and this shell has no interactive
input: it fails with `could not read Username`.

**This is not a problem as long as the student pushes from GitHub Desktop**,
which is what `docs/CLAUDE_CODE.md` §6 already tells them to do. Commit locally,
tell them a commit is ready, and they push.

To make pushing work from a shell one day: install Git for Windows (it puts
`git` on `PATH` and ships Git Credential Manager), then run `git push` once in
a normal terminal and sign in. The token then lands in the Windows vault and
any shell can use it.
