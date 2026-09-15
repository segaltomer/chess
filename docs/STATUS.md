# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39)

**In progress:** nothing. Items 1-4 are committed.

**Last completed:** Phase 1 items 2-4, piece movement, captures and blocked
paths (§26-28), in `common/PieceRules.h/.cpp`. 25 assertions pass covering lone
pieces on an empty board, blocking by own and enemy pieces, every pawn case,
promotion generating four choices, and `attacksSquare` differing from move
generation for pawns and blocked rooks.

**Files mid-edit:** none. Working tree clean.

**Next step:** Phase 1 items 5-7 — turn order (§29), check detection (§30) and
rejecting moves that leave your own king in check (§31). New files
`common/Rules.h/.cpp` (`isSquareAttacked`, `isInCheck`, `legalMoves`) and
`common/ChessGame.h/.cpp` (owns the board, the side to move and the result).
A perft test harness lands with them, in `tests/`.

**Deliberately not done yet:** castling and en passant *generation* (items 10
and 11 — `Board` already performs both, nothing yet offers them as moves), and
item 12, which stays unticked until `ChessGame` exposes "this move needs a
promotion choice" for the GUI.

**Anything left broken or unresolved:** no. The last commit builds.

**Note for the next session:** `git` is not on `PATH` in this shell. The working
copy is the one bundled with Visual Studio; prefix commands with

    $env:PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd;" + $env:PATH

and `g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.
