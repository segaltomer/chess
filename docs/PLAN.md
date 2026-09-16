# Build order

Derived from the requirements document. Work top to bottom; each phase depends
on the ones above it. `docs/DESIGN.md` is complete, so Phase 1 can start.

## Phase 0 — prerequisites

- [x] `docs/DESIGN.md` filled in
- [x] Stockfish 19 installed at `stockfish/stockfish.exe` (§45, §82)
- [x] First git commit

## Phase 1 — chess rules, in `common/` (§26-39)

- [x] Board and piece representation, per DESIGN.md §1
- [x] Legal movement for each piece type (§26)
- [x] Captures; cannot capture own piece (§27)
- [x] Blocked paths, per piece type (§28)
- [x] Turn order (§29)
- [x] Check detection (§30)
- [x] Reject moves leaving own king in check (§31)
- [x] Checkmate (§32)
- [x] Stalemate (§33)
- [x] Castling, all conditions (§34)
- [x] En passant (§35)
- [x] Promotion, choice of Q/R/B/N (§36)
- [x] Game result + reason exposed for the GUI (§39), with resignation (§37)
      and draw by agreement (§38) at the rules level

## Phase 2 — Client, Local Game (§7-17, 48-56)

- [x] **Minimal Server first:** listen on a port, accept connections, print the
      listening IP and port to the console (§11). Nothing else — matchmaking and
      online games come in Phase 4. Without this there is no server to connect
      to, and §8 makes the main menu unreachable until a connection succeeds,
      so none of the rest of this phase can be tested.
- [x] Console connection screen: IP + port (§7)
- [x] Clear errors for bad IP, bad port, server down, failed connect (§9)
- [x] Retry / change IP / change port / exit after failure (§10)
- [x] Main menu after successful connect only (§8, §12)
- [x] GUI event loop wired to the rules engine
- [x] Local Game: two humans, one client, validated locally (§13, §17)
- [x] Resign with Confirm/No dialog (§53-56)
- [x] Draw offer shown to opponent on the same client (§48-49, §51-52)
- [x] **Confirmed by playing it.** Checked at the board: ordinary and illegal
      moves, moving out of turn, check, promotion, castling, en passant,
      resign with both Confirm and No, draw offer with both Accept and
      Decline, and closing the window mid-game. All correct.

## Phase 3 — Bot (§18-20, 43-47)

- [x] Launch Stockfish as a subprocess, relative path (§46)
- [x] UCI communication to get a move (§46-47)
- [x] Three difficulty levels, chosen before the game starts (§43-44)
- [x] Levels clearly distinguishable in play (§19-20) — head-to-head matches:
      Hard beat Medium, Medium beat Easy and Hard beat Easy from both colours,
      every game by checkmate
- [x] Our code still validates every move and owns game state (§47) — checked
      by playing 120 engine half-moves through `ChessGame::makeMove`
- [x] Hide the Offer Draw button in Bot mode
- [x] `Game` base class extracted, per DESIGN decision 2 (§73)
- [x] **Confirmed by playing it.** Checked at the board: the Bot replies to
      every move, and Offer Draw is absent in Bot mode. The strength difference
      was *not* judged by eye — the student does not play chess well enough to
      feel it, and an impression would not be evidence anyway. It is measured
      instead by `tests/bot_match.cpp`; see below.
- [x] `tests/bot_match.cpp` — the levels play each other, each pairing from
      both colours. The stronger level wins all six by checkmate. This is the
      artifact that demonstrates §19-20, and it re-runs after any change to the
      difficulty values.

## Phase 4 — Server and Online (§11, 21-25, 40-42, 50, 56-63)

- [x] TCP listener; print listening IP and port to console (§11)
- [x] Protocol implemented per DESIGN.md §4 (§61-63) — `common/Protocol`
- [x] `Game` hierarchy split so an online game has no rules engine to misuse;
      DESIGN decision 2 rewritten (§73)
- [x] Multiple simultaneous clients (§57)
- [x] Multiple independent concurrent games; no cross-talk (§58-60) — checked
      with two games running at once, including a client aiming a move at a
      game it is not in
- [x] Matchmaking: waiting state, pairing, colour assignment, notify both (§40-41)
- [x] Free-client rules; remove on cancel or disconnect (§42)
- [x] Server owns board, turn, validation for online games (§22) — out-of-turn
      and illegal moves both rejected by the Server
- [x] Client renders server state only, never applies a move locally (§23-24)
- [x] Resign and draw offers relayed through the Server (§50, §56)
- [x] Automated protocol test: 67 checks over matchmaking, validation, game
      isolation, draw offers, resignation, disconnection and malformed input
- [x] **Confirmed by playing it.** Two clients and one server at once: the game
      plays through both windows correctly.
- [x] Side-panel captions measured against the GUI's 260px panel and shortened
      where they overflowed — the panel neither wraps nor truncates
- [x] Board orientation checked against the requirements document: flipping for
      Black is not required, and the provided GUI cannot do it. See DESIGN
      decision 1.
- [ ] Verified between two machines on one network (§25)

## Phase 5 — robustness (§64-70)

- [ ] Client disconnect never crashes the Server (§64)
- [ ] Disconnect handled mid-game, while waiting, and on menu screens (§65)
- [ ] Opponent notified and game ended/updated on disconnect (§66)
- [ ] Reject malformed messages and messages wrong for the current state (§67)
- [ ] Bad user input handled everywhere (§67)
- [ ] Console status logging for all significant events (§68-70)

## Phase 6 — quality pass (§71-80)

- [ ] Responsibilities cleanly separated (§71)
- [ ] Encapsulation; members private, behaviour exposed (§72)
- [ ] Inheritance/polymorphism where it fits, per DESIGN.md §6 (§73)
- [ ] Appropriate STL containers (§74)
- [ ] `const` correctness (§75)
- [ ] RAII for sockets, files, the Stockfish process (§76)
- [ ] Smart pointers for dynamic ownership; no raw new/delete (§77)
- [ ] No leaks, no double free (§78)
- [ ] .h / .cpp separation throughout (§79)
- [ ] Readable, modular, sensible error handling (§80)

## Phase 7 — submission (§82-84)

- [ ] README covers every item in §83
- [ ] `client.exe`, `server.exe`, Stockfish binary, and the 19 DLLs present
- [ ] No absolute paths anywhere in the source
- [ ] Cleanup pass — see the whitelist below. Nothing in the zip that the
      requirements do not ask for.
- [ ] Zip named `<your ID>.zip` (§84)
- [ ] **Check the zip fits the upload limit.** `stockfish.exe` alone is 103 MB,
      so the finished zip will exceed 100 MB. §82 requires the binary, so if
      the submission system caps upload size, the lecturer has to say how he
      wants Stockfish delivered. Worth settling early, since it is the one
      item here that cannot be solved by doing more work.
- [ ] Final check: unzip somewhere else and run it from there

### What the zip contains — a whitelist, not a blacklist

Build the zip by copying these in, not by copying the folder and deleting
things. Anything not on this list does not ship.

```text
README.md                 §83
ProvidedGUI/              §82 - sources + assets/, unmodified
Example/                  his demo, unmodified
GUI_USAGE.md              his file
build_example.bat         his file
common/  client/  server/ our sources - .h and .cpp only
client.exe  server.exe    §82
stockfish/                stockfish.exe + Copying.txt (GPL - required)
*.dll                     the 19 runtime DLLs
```

### Explicitly not in the zip

- `.git/` — the repo. Keep it on disk, keep pushing it; it just isn't submitted.
- `.vscode/`, `.claude/` — editor and tool local state.
- `CLAUDE.md`, `docs/` — our working notes. They stay in the repo.
- `tests/` — the perft harness that verifies the rules engine, and the Bot
  match that verifies the difficulty levels. Not Client or Server source, each
  has its own `main()`, and neither is picked up by the `common/*.cpp`,
  `client/*.cpp` or `server/*.cpp` build commands. They stay in the repo
  because they are how a rules or difficulty change gets re-checked.
- `.gitignore` — meaningless without `.git/`.
- `gui_example.exe` — his demo's binary, not asked for.
- `*.o`, `*.a`, any build intermediates.
- Anything left over from setting the project up.

### Verify before zipping

- [ ] `grep -rniE "claude|generated by|co-authored|TODO|FIXME|HACK"` across the
      shippable files returns nothing
- [ ] No commented-out or dead code, no leftover debug printing
- [x] `client/.gitkeep` and `server/.gitkeep` deleted — real sources exist now
- [ ] No absolute path anywhere: `grep -rn "C:\\" common/ client/ server/`
- [ ] Every `.h` has a matching `.cpp` that something includes; nothing orphaned
- [ ] The unzipped copy builds and runs on a different machine or drive
