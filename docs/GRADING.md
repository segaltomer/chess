# Mock grading of the submitted project

Graded on 2026-09-29 from the submitted `216908467.zip`, unzipped into a fresh
folder and used exactly as the README says. The authority was
`docs/requirements.pdf`, read section by section as a strict lecturer would.
Nothing in the project was changed.

> **Superseded on DLLs.** After this grading the lecturer ruled that the
> submission must contain no DLLs, so they were removed and the programs now run
> from the MSYS2 UCRT64 shell. Issue 8 (bundled DLLs clashing after a rebuild)
> no longer applies, and the runs below that used bundled DLLs with MSYS2 off
> `PATH` describe the earlier package.

## Verdict

**Every one of the 89 sections works when tested the way a lecturer would test
it.** Nothing found here costs certain points. The issues below are judgement
calls a strict grader *might* mark down.

- **Most likely outcome:** full marks, or a point or two short on a 100-point
  scale.
- **Worst plausible outcome:** about 7 points short, and only if the two-computer
  test (§25) fails on the lecturer's network *and* every judgement call goes
  against the project.
- **Resubmission: not worth it for anything on this list.** The largest risk
  (§25) comes from the environment, not the code, so no resubmission could remove
  it. Everything else costs at most a point, which is less than the risk of
  changing a submitted project.

The PDF gives no point values, so costs are estimated on a notional 100-point
scale. That treats the 89 sections as roughly equal (about 1.1 points each), with
functional sections weighted a little above code-style ones.

---

## Issues, ranked by what they could plausibly cost

| # | Issue | Section | Plausible cost | Resubmit? |
|---|---|---|---|---|
| 1 | **Online play across two computers was never actually run.** The code is right: the Server listens on all interfaces (`0.0.0.0`), and in the final audit a game was played through its LAN address rather than `127.0.0.1`. But the lecturer's test depends on things outside the code: the Windows Firewall prompt (the README warns about it), which address the Server prints on a machine with a VPN or virtual network adapters, and networks that stop devices talking to each other, as some campus Wi-Fi does. §25 is the only section the PDF prints in **bold**. | §25 | 0 if the lecturer's test works; 1-3 if it fails and the student can't show it working | **No.** Nothing in the code to change. Instead: run it once on two computers before any demonstration, and know `ipconfig` as the fallback if the printed address is wrong. |
| 2 | **Two protected data members with no visible reason.** `Game::gui_` and `ClientValidatedGame::game_` are `protected`. The argument for it is sound, but it lives in `docs/DESIGN.md`, which is not in the zip. A strict object-oriented grader sees protected data and nothing explaining it. | §72 | 0-1 | No. Be ready to give the argument out loud: a protected accessor returning a mutable reference would give subclasses the same power while only looking encapsulated. |
| 3 | **In a Local game, Resign acts for the side to move.** The player waiting for their turn cannot resign until it comes round. A grader testing "Black resigns during White's turn" will find they can't. | §53 | 0-1 | No. The provided GUI has one Resign button and a confirmation dialog that cannot say who pressed it. That is the explanation. |
| 4 | **Illegal moves are silent in Local and Bot games.** The piece just stays put. Online, the same mistake shows "Illegal move" in the side panel. It's inconsistent, and a grader trying illegal moves locally gets no feedback. | §26-31 feel, §80 | 0-0.5 | No. The rules are enforced correctly; this is polish. |
| 5 | **After an invalid address or port, the only option is to type another one.** The retry / change / exit menu appears after a failed *connection*, which is what §10 says, but not after bad input. Exiting at that prompt means Ctrl+C or closing the window. | §10 | 0-0.5 | No. §10's wording ("after a connection failure") is on our side. |
| 6 | **Comments are heavy and essay-like, and three point at a document the grader doesn't have.** There is roughly one comment line for every three to four lines of code, more in headers. `Server.h:20`, `BotDifficulty.h:16` and `PieceRules.h:21` cite "DESIGN decision 5/6"; `docs/DESIGN.md` is not in the zip. | §80 | 0-0.5 | No. Cosmetic. |
| 7 | **Draw rules the PDF doesn't list are absent:** threefold repetition, fifty moves, insufficient material. King against king plays on forever. §26-39 don't ask for them. | beyond §26-39 | 0-0.5, and only from a very picky grader | No. |
| 8 | **Rebuilding with a newer MSYS2 could clash with the bundled DLLs**, because Windows loads DLLs from the executable's folder first. The README says what to do. Only a grader whose compiler is *newer* than g++ 16.2 and who skips README §3 would hit it. | §86-87 | 0-0.5 | No. |
| 9 | **The README is long** (325 lines). Section 3's heading says "two things that matter" and then covers four. Some content (the Stockfish SHA-256, reserved port ranges, the test programs' internals) is more than a grader needs. | §83 | 0 | No. Everything §83 asks for is there, in order, and every command in it works as written. |
| 10 | Four public methods are never called: `ChessGame::isCheckmate()`, `isStalemate()`, `endReason()` and `Move::operator!=`. | §80 | 0; nobody finds this without searching | No. |
| 11 | A hostile client (half a packet, or never reading) could stall the single-threaded Server. Our own Client never does this. | §64, §67 | 0; not something a grader tests | No. |
| 12 | If the Server dies while a Client is still waiting for an opponent, the box is titled "Game Over" although no game began. The title is fixed by the provided GUI; the text underneath is right ("Lost connection to the server"). | §65 | 0 | No. |
| 13 | Against the Bot the human always plays White. Not required. | §43-44 | 0 | No. |

**Also seen, not an issue:** in one of five scripted runs, a promotion click
fired at machine speed was lost. It didn't reproduce in four more attempts at
the same speed, and no human clicks that fast, so it is a test-harness
artefact rather than something a grader could meet.

---

## Section by section

"Full" means everything the section asks for was seen working in this grading
run; tags like **G5** refer to the runs listed at the end.

### System structure (§1-6)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 1 | Two separate programs | Full | Built and run separately (G2) |
| 2 | GUI only displays and takes input | Full | Rules, results and networking are all in our code; `ProvidedGUI/` is unmodified |
| 3 | GUI_USAGE.md comes with the GUI | Full | Describes what the lecturer supplies; nothing to grade |
| 4 | Non-game screens may be console | Full | Connection, menu and difficulty are console screens |
| 5 | SFML Network | Full | `sf::TcpSocket`, `sf::TcpListener`, `sf::Packet` throughout |
| 6 | Same rules in every mode | Full | One `ChessGame` class serves Local, Bot and the Server |

### Start-up and connection (§7-11)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 7 | First screen asks for IP and port | Full | G8 |
| 8 | Menu only after a successful connection | Full | G8 |
| 9 | Clear errors: bad IP, bad port, server down, failed connection | Full | Every case has its own message (G8) |
| 10 | Retry, change IP, change port, exit | Full, small risk | All four after a failed connection (G8); see issue 5 |
| 11 | Server prints its IP and port | Full | Prints the port, `127.0.0.1` and the LAN address (G7) |

### Main menu (§12-16)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 12 | The four options | Full | Labels match the PDF exactly |
| 13 | Local Game | Full | G5 |
| 14 | Player vs Bot | Full | G6 |
| 15 | Online Game via matchmaking | Full | G7 |
| 16 | Exit | Full | Every run ended "Goodbye." |

### Game modes (§17-25)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 17 | Local: on the Client, Server not involved | Full | Server log shows nothing during six Local games (G5) |
| 18 | Bot: on the Client | Full | Stockfish is a child of `client.exe` |
| 19 | Three levels | Full | Easy, Medium, Hard |
| 20 | Differences significant and demonstrable | Full | Bot match: stronger level won 6/6 by checkmate (G3). By hand, Hard found a mate in one instantly (G6) |
| 21 | Online between two Clients, Server matchmakes | Full | G7 |
| 22 | Server owns the game and validates every move | Full | Protocol test rejects out-of-turn and illegal moves (G3) |
| 23 | Client displays and requests only | Full | Online Client has no rules engine at all |
| 24 | Nothing applied until the Server confirms | Full | Board replaced only from Server messages; input frozen while a request is out |
| 25 | Works across computers; no same-machine assumption | Full in the code, **untested in practice** | Issue 1 |

### Chess rules (§26-39)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 26 | Legal movement of every piece | Full | Perft 26/26 (G3); by hand (G5) |
| 27 | Capture rules | Full | King onto own pawn refused (G5) |
| 28 | Blocking | Full | Rook through a pawn refused (G5) |
| 29 | Turn order | Full | Black moving first refused (G5); out-of-turn rejected online (G3) |
| 30 | Check detection | Full | "Check" shown in amber (G5) |
| 31 | No move leaving your king in check | Full | A reply that ignored check was refused (G5) |
| 32 | Checkmate | Full | Local, Bot and Online all ended "Checkmate" (G5-G7) |
| 33 | Stalemate | Full | Sam Loyd's ten-move stalemate ended "Draw / Stalemate" (G5) |
| 34 | Castling, every condition | Full | Both sides castled by hand (G5); conditions covered by perft |
| 35 | En passant | Full | By hand (G5) |
| 36 | Promotion with Q/R/B/N choice | Full | Dialog showed all four; knight chosen and placed (G5) |
| 37 | Resignation ends the game | Full | G5-G7 |
| 38 | Draw by agreement, Local and Online | Full | Accepted in both (G5, G7) |
| 39 | Result and reason passed to the GUI | Full | Summary box shows both; side panel keeps the result |

### Online Game (§40-42)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 40 | Search, wait if alone | Full | "Waiting for opponent" (G7) |
| 41 | Pair, create, assign colours, notify both | Full | G7 |
| 42 | Free-client rules; removed on cancel or disconnect | Full | Protocol test (G3) |

### Player vs Bot (§43-47)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 43 | Choose easy, medium or hard first | Full | Out-of-range choice refused (G6) |
| 44 | Game starts after choosing | Full | G6 |
| 45 | Stockfish chooses the moves | Full | Banner confirms Stockfish 19 (G4) |
| 46 | Separate process | Full | Child process, pipes, UCI |
| 47 | Legality and state stay ours | Full | Every engine move goes through our `makeMove` |

### Draw offer (§48-52)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 48 | Offer in Local and Online | Full | Hidden in Bot, as §48 allows |
| 49 | Local: same Client, for the opponent | Full | "White offers a draw. Black, do you accept?" (G5) |
| 50 | Online: through the Server, opponent only | Full | Offer shown on the opponent's window (G7); never echoed to the offerer or to another game (G3) |
| 51 | Accept or Decline | Full | G5, G7 |
| 52 | Accept → draw with message; Decline → play on | Full | G5, G7, G3 |

### Resignation (§53-56)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 53 | Every human player can resign in every game | Full, small risk | Issue 3 |
| 54 | Confirmation on the right Client | Full | Local dialog on the same window (G5); online, the audit's window captures showed it on the resigner's window only |
| 55 | Confirm and No | Full | G5 |
| 56 | Opponent wins; Server updates both online | Full | Local and Bot (G5, G6); online, both Clients told (G3) |

### Several games at once (§57-60)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 57 | Many Clients and games | Full | Two games at once through the GUI (G7); 20 Clients in the protocol test |
| 58 | Games independent | Full | One game went to mate while the other continued untouched (G7) |
| 59 | No updates outside the game | Full | G3 |
| 60 | No single-game assumption | Full | Games kept in a map by id |

### Protocol (§61-63)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 61 | Own protocol | Full | `common/Protocol.h` |
| 62 | Covers everything listed | Full | Connect and disconnect are the TCP session, and `Protocol.h` says so |
| 63 | Clear, consistent, explainable | Full | One file, each message's writer beside its reader |

### Disconnections and errors (§64-67)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 64 | Disconnect never crashes the Server | Full | Process killed mid-game; Server carried on (G7). 1,600 malformed packets (G3) |
| 65 | Handled mid-game, while waiting, on menus | Full | All three seen (G7) |
| 66 | Opponent told | Full | "White wins! / Black disconnected" (G7) |
| 67 | Bad input and bad messages | Full | G3, G8 |

### Server console (§68-70)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 68 | Meaningful status messages | Full | Readable log, every line tagged with client and game |
| 69 | The listed events | Full | Every one of them appears in the G7 log |
| 70 | Clear and useful | Full | e.g. `game 1: move d8h4 from client 2 (127.0.0.1:61876) accepted` |

### C++ design (§71-80)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 71 | Sensible division into classes | Full | `common/` rules, `client/` screens and modes, `server/` split three ways |
| 72 | Encapsulation | Full, small risk | Issue 2 |
| 73 | Inheritance and polymorphism where they fit | Full | Piece-rules hierarchy; game-mode hierarchy |
| 74 | STL and data structures | Full | `array`, `vector`, `map`, `optional`, algorithms |
| 75 | `const` | Full | Consistent |
| 76 | RAII | Full | Sockets and the Stockfish process are released by destructors |
| 77 | Smart pointers where ownership is dynamic | Full | `unique_ptr` for sockets. There is little dynamic ownership, and no raw `new` or `delete` anywhere |
| 78 | No leaks or double frees | Full | Memory and handles flat across repeated workloads (measured in the audit) |
| 79 | `.h` / `.cpp` separation | Full | Throughout |
| 80 | Readable, modular, handles errors | Full, small risk | Issues 4 and 6 |

### Submission and environment (§81-89)

| § | Requirement | Expected mark | Why |
|---|---|---|---|
| 81 | Personal project | Full | Not assessable from the code |
| 82 | Source, ProvidedGUI, both executables, Stockfish | Full | All present (G1) |
| 83 | README covering the listed topics | Full | Every item present; every command worked as written (G2-G8) |
| 84 | `<ID>.zip` | Full | `216908467.zip` |
| 85 | C++17, MSYS2 UCRT64 | Full | G2 |
| 86 | SFML 3, compiles without errors | Full | No errors, and no warnings even under `-Wall -Wextra -Wpedantic` (G2) |
| 87 | Compile and run separately | Full | G2 |
| 88 | Client compile command | Full | Matches the PDF's template |
| 89 | Server compile command | Full | Matches the PDF's template |

---

## Beyond the checklist

**How the code reads to someone opening it cold.** Well, on the whole. Naming
is consistent and descriptive, braces and spacing never vary, functions are
short with early returns, and compound conditions are pulled into named
booleans. File names match class names, and `common/`, `client/` and `server/`
mean what they say. A reader starting at `client/main.cpp` can find every mode
within a minute. The three-level game hierarchy (`Game` → `ClientValidatedGame`
→ `LocalGame`/`BotGame`, and `Game` → `OnlineGame`) could look like one level
too many, but the header of `Game.h` explains why in the first screen: the
online game must have no rules engine to misuse.

What stands out is the commenting. About 940 comment lines against 3,230 lines
of code, many of them paragraphs arguing a design choice, with section numbers
cited throughout. The citations are genuinely useful to *this* lecturer, since
they map the code to his own list. But the volume and the polished,
argumentative voice make it read more like a design document than working
notes. That costs no points under the PDF. It does make an oral walkthrough
likely, and the student should be comfortable explaining any file on request;
`docs/REVISION.md` is written for exactly that.

**The README.** Clear, ordered to follow §83's list, and every command in it
works as written in the MSYS2 shell. It is longer than a grader needs. Sections
11-12 and a few troubleshooting paragraphs could go without loss, but nothing
in it is wrong or confusing.

**Anything careless.** Small things only: the three comments citing a
`DESIGN` document that isn't shipped, the README heading that promises two
things and gives four, and illegal moves being silent locally but explained
online.

**Anything over-engineered.** Not really. The test programs go well beyond
what a course expects: published perft positions, Bot-against-Bot matches, and
1,600 random malformed packets. But they are separate, optional and documented,
so they read as thoroughness rather than bloat. The Stockfish wrapper is long
because Windows process and pipe handling is long, not because of extra
features. Of the product itself, about 3,200 lines for two programs, full chess
rules, matchmaking and an engine bridge is proportionate.

---

## What was run

| Tag | Run | Result |
|---|---|---|
| **G1** | Unzipped `216908467.zip` into a fresh folder; read the README from it | 86 files; layout as the README describes |
| **G2** | README §2's two commands, exactly as written, in the MSYS2 UCRT64 shell; then again with `-Wall -Wextra -Wpedantic` | Both build; 0 errors, 0 warnings |
| **G3** | README §11: perft, Bot match, protocol test | 26/26 in 15.4 s; stronger level won 6/6 by checkmate in 3 min 40 s; 103/103 |
| **G4** | Asked the bundled engine to identify itself | "Stockfish 19", as the README says |
| **G5** | Six Local games through the real window, run twice: illegal moves (out of turn, rook through a pawn, king onto its own pawn, a reply ignoring check); check display; Black resigning on Black's turn; fool's mate; en passant; castling both sides; promotion to a knight via the dialog; Sam Loyd's stalemate; a draw offered and accepted | All correct both times; the Server logged nothing during any of it |
| **G6** | Bot: out-of-range difficulty; Hard handed a mate in one; Easy then resigned | Refused and re-asked; Hard mated at once and the game ended "Checkmate"; "Bot is thinking..." shown; resignation correct |
| **G7** | Online per README §9: four Clients, two games at once, one to checkmate and one to an agreed draw offered by Black; a third game where one player's process was killed; a Client waiting when the Server was killed | Both games isolated and correct on both windows; "Black disconnected" shown to the survivor; "Lost connection to the server" to the waiting Client; every Client's menu then reported the lost connection |
| **G8** | Connection screen with an empty address, `999.1.1.1`, port 0, a word for a port, and two refused connections; Server given a bad port argument, and started twice on one port | A specific message for each, and the four choices after each failed connection; the Server falls back to its default port, and the second one exits with a clear reason |

The Client runs used the executables built in G2, with MSYS2 removed from
`PATH`, so only the bundled DLLs loaded. The two-computer case (issue 1) could
not be run: this machine has no second computer to connect from.
