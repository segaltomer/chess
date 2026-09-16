# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39). **COMPLETE.**

**In progress:** nothing. All thirteen Phase 1 items are ticked and committed,
and `docs/REVISION.md` has the phase's question-and-answer entries.

**Last completed:** item 13, the game result and reason for the GUI (§39),
together with resignation (§37) and draw by agreement (§38) at the rules level.

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

## Next step

Phase 2, first item: the **minimal Server** — listen on a port, accept
connections, print the listening IP and port to the console (§11). It comes
first because §8 makes the Client's main menu unreachable until a connection
succeeds, so without a server to connect to, nothing else in Phase 2 can be
tested. Matchmaking and online games stay in Phase 4.

Then the console connection screen (§7), its error handling (§9-10), the main
menu (§8, §12), and the GUI event loop wired to `ChessGame` for a Local game.

Phase 2 is the first point where the student has to run it and look at it —
see `docs/CLAUDE_CODE.md` §4.

## Environment notes for the next session

`git` is **not** on `PATH` in this shell. The working copy is the one bundled
with Visual Studio; prefix git commands with:

    $env:PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd;" + $env:PATH

`g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.

Commit messages must be passed with `git commit -F <file>`, not `-m`, because
PowerShell mangles arguments containing double quotes when handing them to a
native executable.

**`git push` does not work from this shell** — there are no credentials here
and it fails with `could not read Username for 'https://github.com'`. Commits
are being made locally and are safe, but **the GitHub backup is not current**.
Push from GitHub Desktop, which has the login (`docs/CLAUDE_CODE.md` §6).
