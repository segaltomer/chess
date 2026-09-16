# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 1 — chess rules in `common/` (§26-39)

**In progress:** nothing. Items 1-11 are committed.

**Last completed:** Phase 1 item 11, en passant (§35), in
`common/PieceRules.cpp`.

**The rules engine is now verified against the full standard perft suite** —
all 26 cases pass, about 15 million positions across the six positions chess
engine authors use to catch each other's bugs. Start position to depth 5 is
4,865,609; Kiwipete to depth 4 is 4,085,603. This is the main correctness
evidence for §26-36 and it should be re-run after any change to `common/`.

Alongside it: 31 assertions on turn order, check, pins and FEN; 40 on checkmate
and stalemate; 37 on castling; 21 on en passant including the capture that is
illegal because it would expose your own king along the rank.

**Files mid-edit:** none. Working tree clean.

**Next step:** Phase 1 item 12, promotion (§36). The rules engine already
generates all four choices — that is what makes the "promotions" perft position
come out right at 422,333. What is missing is the one method the GUI will need
in Phase 2: `ChessGame::needsPromotionChoice(from, to)`, so the Client knows to
show `showPromotionDialog` and wait for the answer before completing the move.

Then item 13, the game result and reason for the GUI (§39), which also brings
in resignation (§37) and draw by agreement (§38). That finishes Phase 1, and
the phase boundary means appending question-and-answer entries to
`docs/REVISION.md`.

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
