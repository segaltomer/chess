# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39)

**In progress:** nothing. Items 1-10 are committed.

**Last completed:** Phase 1 item 10, castling with all six conditions (§34),
generated in `common/Rules.cpp`.

Verified so far: perft matches the published counts for the start position at
depths 1-4 (20 / 400 / 8,902 / 197,281) and for all five standard suite
positions at depth 1, including Kiwipete's 48 — which contains both castles;
31 assertions on turn order, check, pins and FEN; 40 on checkmate and
stalemate; 37 on castling, one per condition in both directions plus the
b1-may-be-attacked case and the resulting piece placement for both colours.

**Files mid-edit:** none. Working tree clean.

**Next step:** Phase 1 item 11, en passant (§35). `Board` already performs the
capture and already sets and clears the target square; what is missing is
`PawnRules::appendCaptures` offering the move when `board.enPassantTarget()` is
the square diagonally ahead. That is about six lines.

With it the whole standard perft suite becomes meaningful, so the same commit
should extend `tests/perft_main.cpp` to the full depths — start position to
depth 5 (4,865,609), Kiwipete to 4 (4,085,603), and the other three to 4 or 5.
That is the real proof the rules engine is right; expect it to take a minute or
so to run.

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
