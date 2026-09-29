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
- [x] `tests/protocol_test.cpp` — 81 checks over matchmaking, validation, game
      isolation, draw offers, resignation, disconnection and malformed input;
      103 since the first Phase 7 audit pass (commit `4281b33`) added
      checkmate, promotion and bulk malformed input over the network. Needs a
      running server on the same port.
- [x] **Confirmed by playing it.** Two clients and one server at once: the game
      plays through both windows correctly.
- [x] Side-panel captions measured against the GUI's 260px panel and shortened
      where they overflowed — the panel neither wraps nor truncates
- [x] Board orientation checked against the requirements document: flipping for
      Black is not required, and the provided GUI cannot do it. See DESIGN
      decision 1.
- [ ] Verified between two machines on one network (§25)

## Phase 5 — robustness (§64-70)

- [x] Client disconnect never crashes the Server (§64) — clients dropped in
      every state; the Server keeps pairing new ones afterwards
- [x] Disconnect handled mid-game, while waiting, and on menu screens (§65) —
      the menu case needed `isServerStillThere`, since a dead TCP connection is
      only discovered by using it
- [x] Opponent notified and game ended/updated on disconnect (§66) — worded as
      "disconnected", not "resigned"
- [x] Reject malformed messages and messages wrong for the current state (§67) —
      both directions; unknown type, empty, truncated, server-only messages from
      a client, and six state mismatches
- [x] Bad user input handled everywhere (§67) — every console prompt driven with
      rubbish; each refuses with a specific message and re-asks
- [x] Console status logging for all significant events (§68-70) — checked
      against §69's list item by item

## Phase 6 — quality pass (§71-80)

- [x] Responsibilities cleanly separated (§71) — nothing in `common/` prints to
      the console; sockets, dispatch, matchmaking and one hosted game are four
      separate classes on the Server
- [x] Encapsulation; members private, behaviour exposed (§72) — the two
      protected members are argued for in DESIGN decision 6 rather than left
      to look like an oversight
- [x] Inheritance/polymorphism where it fits, per DESIGN.md §6 (§73) — piece
      rules, and the three-level `Game` hierarchy
- [x] Appropriate STL containers (§74) — `array` for the board, `vector` for
      moves and clients, `map` keyed by game id, `unique_ptr`, `optional`
- [x] `const` correctness (§75) — swept; `describeById` made const, and the two
      by-value `sf::Packet` parameters now say why they are not `const&`
- [x] RAII for sockets, files, the Stockfish process (§76) — demonstrated: the
      Server's handle count is identical before and after six full workloads
- [x] Smart pointers for dynamic ownership; no raw new/delete (§77) — searched;
      there is no `new`, `delete`, `malloc` or `free` anywhere in our source
- [x] No leaks, no double free (§78) — six full protocol workloads against one
      Server process left memory flat at 1,048 KB and handles constant at 94
- [x] .h / .cpp separation throughout (§79) — the only code in a header is
      three empty default hooks
- [x] Readable, modular, sensible error handling (§80) — no absolute paths, no
      TODO/FIXME, no commented-out code, no leftover debug printing; two dead
      methods found and deleted

## Phase 7 — submission (§82-84)

- [ ] **§25 — TEST ON A SECOND COMPUTER. The student asked to be reminded of
      this at the end.** Run `server.exe` and one `client.exe` here, and a
      second `client.exe` on another machine on the same network, connecting to
      the LAN address the server prints. It is the only requirement that cannot
      be checked on one machine, and it is graded. Everything else about online
      play is already verified.
- [x] README covers every item in §83 — rewritten for the checker: compiling,
      running the Server, where the address and port appear, running a Client
      and connecting, a section for each of the three modes, several Clients
      and concurrent games, how Stockfish is integrated, and the environment
      requirements
- [x] `client.exe`, `server.exe`, Stockfish binary, and the 19 DLLs present
- [x] No absolute paths anywhere in the source
- [x] Cleanup pass — the staged tree is built from the whitelist below by
      copying in, and holds nothing else
- [x] **The zip fits, so this needs nothing from the lecturer.** It was flagged
      as a possible blocker and is not: `stockfish.exe` is 98 MB but compresses
      well, and the whole submission comes to **82.3 MB** — under a 100 MB cap
      whichever way a megabyte is counted. 110 MB unpacked.
- [x] Final check: the staged copy was run **with MSYS2 removed from `PATH`**,
      which is the real test of the bundled DLLs. The Server starts and prints
      its addresses, a Client connects and reaches the menu, and a Bot game
      launches `stockfish.exe` from the staged folder's own `stockfish/` —
      proving the path is relative to the executable — leaving no orphan behind.
- [x] Zip named `<your ID>.zip` (§84) — `216908467.zip`, everything inside one
      top-level folder `216908467/`. §84 asks only for the archive's name (its
      "תיקיית ZIP" is Windows' "compressed folder", not a folder inside it); the
      top-level folder is kept so "Extract here" does not scatter the project.
      Extracted elsewhere and run from there with MSYS2 off `PATH`: both
      programs work, Stockfish launches from the unzipped copy, the supplied
      files are byte-identical after the round trip, and Stockfish's SHA-256
      matches the README.
- [x] Entry names use forward slashes. Worth a line because the obvious way to
      build the archive gets this wrong: .NET Framework's
      `ZipFile.CreateFromDirectory` writes the platform separator, producing
      entries like `216908467\client.exe`. Windows Explorer copes; the ZIP
      format does not allow it, and other tools extract such an archive as flat
      files with backslashes in their names. The zip is built entry by entry
      instead.
- [x] **Final audit against the PDF, in both directions** — `docs/AUDIT.md`.
      All 89 sections met. It fixed one visible defect (§39: a finished game's
      side panel said "Game in progress" once the summary was closed), took
      `Example/`, `build_example.bat` and `GUI_USAGE.md` out of the zip (86
      entries now), and corrected the README in fourteen places. The unzipped
      package was run with real Client windows driven by scripted clicks.

### What the zip contains — a whitelist, not a blacklist

Build the zip by copying these in, not by copying the folder and deleting
things. Anything not on this list does not ship.

```text
README.md                 §83
ProvidedGUI/              §82 - sources + assets/, unmodified
common/  client/  server/ our sources - .h and .cpp only
tests/                    the three verification programs - see below
client.exe  server.exe    §82
stockfish/                stockfish.exe + Copying.txt + AUTHORS (GPL - required)
*.dll                     the 19 runtime DLLs
```

**`tests/` ships, which reverses the earlier decision to leave it out.** Two
reasons. §20 asks that the Bot's difficulty levels be *demonstrable*, and
`bot_match` is the thing that demonstrates them — shipping the demonstration is
directly responsive to the word the requirement uses. And the README describes
all three harnesses, so leaving them out would mean documenting commands that
refer to files the checker does not have, which is worse than the alternative.

They cannot interfere with anything: each has its own `main()`, and none is
picked up by `common/*.cpp`, `client/*.cpp` or `server/*.cpp`, so the two
documented build commands are unaffected.

### Explicitly not in the zip

- `.git/` — the repo. Keep it on disk, keep pushing it; it just isn't submitted.
- `.vscode/`, `.claude/` — editor and tool local state.
- `CLAUDE.md`, `docs/` — our working notes. They stay in the repo.
- `perft.exe`, `bot_match.exe`, `protocol_test.exe` — the built test binaries.
  The `tests/` sources ship; their executables do not, since §82 asks only for
  the Client and the Server, and the README gives the commands to build them.
- `.gitignore` — meaningless without `.git/`.
- `Example/`, `build_example.bat`, `GUI_USAGE.md` — the lecturer's demo, its
  build script and his API notes. Shipped until the final audit, then taken
  out: §82 asks for `ProvidedGUI/` only, and `build_example.bat` sitting at the
  top level reads as the project's build script but builds his demo instead.
  They stay in the repo, unmodified. See `docs/AUDIT.md`.
- `gui_example.exe` — his demo's binary, not asked for.
- `*.o`, `*.a`, any build intermediates.
- Anything left over from setting the project up.

### Verify before zipping

- [x] `grep -rniE "claude|generated by|co-authored|TODO|FIXME|HACK"` across the
      shippable files returns nothing — only "generated" in the chess sense
- [x] No commented-out code, no leftover debug printing. Four small public
      methods are never called; left in deliberately, see `docs/AUDIT.md`
- [x] `client/.gitkeep` and `server/.gitkeep` deleted — real sources exist now
- [x] No absolute path anywhere: `grep -rn "C:\\" common/ client/ server/`
- [x] Every `.h` has a matching `.cpp` that something includes; nothing orphaned
- [ ] The unzipped copy builds and runs on a different machine or drive
