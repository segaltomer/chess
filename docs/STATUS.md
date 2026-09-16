# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39)

**In progress:** nothing. Items 1-9 are committed.

**Last completed:** Phase 1 items 8-9, checkmate (§32) and stalemate (§33), in
`common/Rules.h/.cpp` with thin forwarders on `ChessGame`.

Verified so far: perft from the starting position matches the published counts
at depths 1-4 (20 / 400 / 8,902 / 197,281); 31 assertions on turn order, check
detection, pins, king safety and FEN parsing; 40 assertions across three real
checkmates (fool's, back rank, smothered), two stalemates, and two in-check
positions that are *not* mate.

**Files mid-edit:** none. Working tree clean.

**Next step:** Phase 1 item 10, castling with all its conditions (§34).

Note on where it goes: castling is generated in `Rules`, **not** in `KingRules`
with the other king moves. It has to be, because two of its conditions are
about attacked squares — the king may not castle out of check, nor through a
square the enemy attacks — and answering that needs `isSquareAttacked`, which
lives in `Rules`. Having `PieceRules` call back into `Rules` would make the two
mutually dependent. Generating castling one level up keeps the dependency
one-way, and is honest besides: castling is a fact about a king, a rook and the
check state together, not about how a king moves.

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
