# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 2 — Client, Local Game. Phase 1 is complete.

**In progress:** nothing.

**Last completed:** Phase 2's first item, the minimal Server (§11) —
`server/Server.h/.cpp` and `server/main.cpp`. It binds a port, prints the
addresses a Client should use, accepts any number of clients, logs connections
and disconnections, and survives both.

Tested by running it and connecting three real TCP clients: two at once, one
sending an unknown message, disconnecting in a different order than they
connected, and a new client arriving after an earlier one left. Also tested
with a non-numeric port argument, an out-of-range port, and a second server
told to take a port already in use.

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

Phase 2, second item: the Client's **console connection screen** (§7) — ask for
the server IP and port, connect, and on failure give a clear message (§9) and
the choice to retry, change the IP, change the port, or exit (§10). The main
menu (§8, §12) is only reachable once a connection has succeeded.

After that: the GUI event loop wired to `ChessGame`, then the Local Game
itself (§13, §17), resignation with its Confirm/No dialog (§53-56), and the
draw offer shown to the opponent on the same client (§48-49, §51-52).

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

**`git push` does not work from this shell** — there are no credentials here
and it fails with `could not read Username for 'https://github.com'`. Commits
are being made locally and are safe, but **the GitHub backup is not current**.
Push from GitHub Desktop, which has the login (`docs/CLAUDE_CODE.md` §6).
