# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39)

**In progress:** nothing. Items 1-7 are committed.

**Last completed:** Phase 1 items 5-7 — turn order (§29), check detection (§30)
and rejecting moves that leave your own king in check (§31). New files
`common/Rules.h/.cpp`, `common/ChessGame.h/.cpp`, and the perft harness at
`tests/perft_main.cpp`.

Verified: perft from the starting position matches the published counts at
depths 1-4 (20 / 400 / 8,902 / 197,281), plus 31 direct assertions on turn
order, blocked and unblocked checks, pinned pieces, king safety, kings not
standing adjacent, and FEN parsing.

**Files mid-edit:** none. Working tree clean.

**Next step:** Phase 1 items 8-9, checkmate (§32) and stalemate (§33). Both are
one question — "no legal moves" — split by whether the side to move is in
check, so they belong on `ChessGame` alongside the result and reason of item 13.

**Deliberately not done yet:** castling and en passant *generation* (items 10
and 11 — `Board` already performs both, nothing yet offers them as moves), and
item 12, which stays unticked until `ChessGame` exposes "this move needs a
promotion choice" for the GUI. The perft suite is trimmed to the depths those
gaps do not affect; the castling and en passant commits each extend it, and the
full standard suite should be live by the end of the phase.

**How to run the tests** (from the project root, with `C:\msys64\ucrt64\bin` on
`PATH`):

    g++ -std=c++17 -O2 -Icommon common/*.cpp tests/perft_main.cpp -o perft.exe
    ./perft.exe

**Anything left broken or unresolved:** no. The last commit builds.

**Note for the next session:** `git` is not on `PATH` in this shell. The working
copy is the one bundled with Visual Studio; prefix commands with

    $env:PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd;" + $env:PATH

and `g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.
