# Build order

Derived from the requirements document. Work top to bottom; each phase depends
on the ones above it. `docs/DESIGN.md` is complete, so Phase 1 can start.

## Phase 0 — prerequisites

- [x] `docs/DESIGN.md` filled in
- [x] Stockfish 19 installed at `stockfish/stockfish.exe` (§45, §82)
- [x] First git commit

## Phase 1 — chess rules, in `common/` (§26-39)

- [ ] Board and piece representation, per DESIGN.md §1
- [ ] Legal movement for each piece type (§26)
- [ ] Captures; cannot capture own piece (§27)
- [ ] Blocked paths, per piece type (§28)
- [ ] Turn order (§29)
- [ ] Check detection (§30)
- [ ] Reject moves leaving own king in check (§31)
- [ ] Checkmate (§32)
- [ ] Stalemate (§33)
- [ ] Castling, all conditions (§34)
- [ ] En passant (§35)
- [ ] Promotion, choice of Q/R/B/N (§36)
- [ ] Game result + reason exposed for the GUI (§39)

## Phase 2 — Client, Local Game (§7-17, 48-56)

- [ ] **Minimal Server first:** listen on a port, accept connections, print the
      listening IP and port to the console (§11). Nothing else — matchmaking and
      online games come in Phase 4. Without this there is no server to connect
      to, and §8 makes the main menu unreachable until a connection succeeds,
      so none of the rest of this phase can be tested.
- [ ] Console connection screen: IP + port (§7)
- [ ] Clear errors for bad IP, bad port, server down, failed connect (§9)
- [ ] Retry / change IP / change port / exit after failure (§10)
- [ ] Main menu after successful connect only (§8, §12)
- [ ] GUI event loop wired to the rules engine
- [ ] Local Game: two humans, one client, validated locally (§13, §17)
- [ ] Resign with Confirm/No dialog (§53-56)
- [ ] Draw offer shown to opponent on the same client (§48-49, §51-52)

## Phase 3 — Bot (§18-20, 43-47)

- [ ] Launch Stockfish as a subprocess, relative path (§46)
- [ ] UCI communication to get a move (§46-47)
- [ ] Three difficulty levels, chosen before the game starts (§43-44)
- [ ] Levels clearly distinguishable in play (§19-20)
- [ ] Our code still validates every move and owns game state (§47)
- [ ] Hide the Offer Draw button in Bot mode

## Phase 4 — Server and Online (§11, 21-25, 40-42, 50, 56-63)

- [ ] TCP listener; print listening IP and port to console (§11)
- [ ] Protocol implemented per DESIGN.md §4 (§61-63)
- [ ] Multiple simultaneous clients (§57)
- [ ] Multiple independent concurrent games; no cross-talk (§58-60)
- [ ] Matchmaking: waiting state, pairing, colour assignment, notify both (§40-41)
- [ ] Free-client rules; remove on cancel or disconnect (§42)
- [ ] Server owns board, turn, validation for online games (§22)
- [ ] Client renders server state only, never applies a move locally (§23-24)
- [ ] Resign and draw offers relayed through the Server (§50, §56)
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
- [ ] `client.exe`, `server.exe`, Stockfish binary, and the 18 DLLs present
- [ ] No absolute paths anywhere in the source
- [ ] `.vscode/`, `CLAUDE.md`, `docs/` and `.git/` removed **from the zip**
      (they stay in the repo — they are tracked so they get backed up)
- [ ] Zip named `<your ID>.zip` (§84)
- [ ] **Check the zip fits the upload limit.** `stockfish.exe` alone is 103 MB,
      so the finished zip will exceed 100 MB. §82 requires the binary, so if
      the submission system caps upload size, the lecturer has to say how he
      wants Stockfish delivered. Worth settling early, since it is the one
      item here that cannot be solved by doing more work.
- [ ] Final check: unzip somewhere else and run it from there
