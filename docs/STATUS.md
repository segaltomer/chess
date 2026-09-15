# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39)

**In progress:** nothing. Item 1 is committed.

**Last completed:** Phase 1 item 1, "Board and piece representation" —
`common/ChessTypes.h/.cpp`, `common/Move.h/.cpp`, `common/Board.h/.cpp`.
Compiles clean under `-Wall -Wextra` and passes a smoke test covering the start
position, the en passant target appearing and clearing, the castling rook jump,
an en passant capture, a promotion, and castling rights being lost.

**Files mid-edit:** none. Working tree clean.

**Next step:** Phase 1 item 2, legal movement per piece type (§26), in
`common/PieceRules.h/.cpp` — the `PieceRules` base plus one subclass per piece,
per DESIGN decision 6. Items 2-4 (movement, captures, blocked paths) are the
same code and land together.

**Anything left broken or unresolved:** no. The last commit builds.

**Note for the next session:** `git` is not on `PATH` in this shell. The working
copy is the one bundled with Visual Studio; prefix commands with

    $env:PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd;" + $env:PATH

and `g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.
