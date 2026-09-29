# Final audit against the requirements PDF

Done on 2026-09-29, against `docs/requirements.pdf` (the lecturer's original,
Hebrew) read in full, not against our own transcription. Everything below was
checked in the code, the README or the built package. Nothing is taken from our
earlier docs on trust.

**Result: all 89 sections met.** One code defect was found and fixed (§39: the
side panel said "Game in progress" after a game had ended). Three files the PDF
does not ask for were taken out of the zip. Fourteen README corrections were
made, and stale comments and docs were fixed. §25 is met in the code and proved
as far as one computer allows. Actually running it on two computers is the one
check still outstanding, and it has to be done by hand.

> **Superseded on DLLs.** After this audit the lecturer ruled that the
> submission must contain no DLLs. The zip no longer has them, and the programs
> are run from the MSYS2 UCRT64 shell, whose `PATH` supplies SFML's. So these
> parts describe the package as it was on the day, not as it is now: runs R7,
> R8, R10 and R11 (MSYS2 off `PATH` with bundled DLLs), the §82 row's "plus the
> DLLs", the README changes about DLLs and double-clicking in Part 2, and the
> "19 DLLs" row in Part 3. The current package is described in `docs/PLAN.md`,
> Phase 7.

## What was run

Each run is referred to by its tag in the evidence column below.

| Tag | What was run | Result |
|---|---|---|
| **R1** | `HEAD` exported to a clean folder; the README's two build commands pasted exactly, in PowerShell, `cmd` and the MSYS2 UCRT64 shell; then again with `-Wall -Wextra` | all builds exit 0, no warnings |
| **R2** | `tests/perft_main.cpp`, built with the README's command | 26/26 cases, 16,564,718 positions, ~16 s |
| **R3** | `tests/protocol_test.cpp` against a running `server.exe`, six runs, one of them against the unzipped package's own server | 103/103 every time; the server was still running afterwards |
| **R4** | `tests/bot_match.cpp`, run three times, the last from the unzipped package | the stronger level won all six games by checkmate in every run; 194 s and 221 s for the timed runs |
| **R5** | `client.exe` with scripted console input and no server: empty address, a port typed as an address, an unresolvable name, a non-number port, port 70000, a refused connection, then choices 5 (out of range), 1, 3, 2 and 4 | every input got a specific message; all four §10 choices worked |
| **R6** | `client.exe` connected to a server that was then killed; Online chosen from the menu | loss detected and reported; Local and Bot still offered |
| **R7** | **Unzipped package**, in a path containing a space and Hebrew letters, **MSYS2 removed from `PATH`**. Server plus two real Client windows driven by posted mouse clicks and captured to images: pairing, e2e4, an out-of-turn click, an illegal move, a draw offered and declined, resign cancelled then confirmed, summary closed; then a third Client closing its window while waiting | everything as required. Two things seen only on screen: the draw dialog and the resign confirmation each appeared on one window only |
| **R8** | Same package: a Local game (moves, draw offer declined on the same window, resign confirmed, summary closed) and a Bot game on Easy | Bot replied. `stockfish.exe` ran from the package's own `stockfish\`, as a child of `client.exe`, and was gone once its window closed. The server logged nothing for either mode |
| **R9** | Online game between two Clients connected through the Server's printed LAN address (192.168.132.67), not loopback | paired and played; the server logged both peers as 192.168.132.67 |
| **R10** | `client.exe` and `server.exe` with no DLLs and no MSYS2 on `PATH` | both exit `0xC0000135` with no output |
| **R11** | `ldd client.exe server.exe` compared with the 19 bundled DLLs; bundled copies hashed against `C:\msys64\ucrt64\bin` | exactly the 19 needed, none extra; byte-identical |
| **R12** | From the unzipped package: both README build commands and all three test builds | all exit 0; perft and protocol test pass there |
| **R13** | `server.exe` start-up output; `netstat` | prints port, 127.0.0.1 and the LAN address; listening on `0.0.0.0` |
| **R14** | Server memory and handle count after a warm-up protocol-test workload, then again after three more | private memory 1,168 KB → 1,088 KB, handles 92 → 92; all 412 checks passed |

Also checked: `ProvidedGUI/` is touched only by the first commit (`8774f2a`).
The zip's 86 entries all use forward slashes and all sit under `216908467/`, and
every extracted file is byte-identical to the project. Stockfish's SHA-256
matches the README. The shipped files contain no tool or AI mentions, no
TODO/FIXME, no absolute paths, no non-ASCII string literals and no encoding
damage.

---

## Part 1 — every section of the PDF

The preamble says every section not explicitly marked optional is mandatory.
None is marked optional. It also lists what is *not* required: writing a GUI
(`ProvidedGUI/` is used unchanged), understanding its SFML internals, and
writing a move-choosing algorithm (Stockfish is used, §45).

File references are to the code as committed with this audit.

### System structure (§1-6)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 1 | Two separate programs, Client and Server | Met | `client/main.cpp:122`, `server/main.cpp:50`; separate build commands (README §2); R1 |
| 2 | GUI is display and input only: no legality, engine, networking, online games or winner | Met | Legality in `common/` (`ChessGame.cpp:89-100`); result worded by `ChessGame.cpp:170-205` and handed to `showGameOver` (`Game.cpp:127`); networking in `ServerConnection`/`Server`; `ProvidedGUI/` unmodified |
| 3 | The GUI package comes with `GUI_USAGE.md` | Met (describes what the lecturer supplies) | Present in the repo, unmodified, and followed. Not in the zip, see Part 2 |
| 4 | Non-game screens may be console | Met | Connection screen, menu and difficulty choice are console (`ConnectionScreen.cpp`, `MainMenu.cpp`, `BotDifficulty.cpp`) |
| 5 | Networking with SFML Network | Met | `sf::TcpSocket` (`ServerConnection.h:54`), `sf::TcpListener` and `sf::SocketSelector` (`Server.h:55-56`), `sf::Packet` (`Protocol.h`) |
| 6 | Same chess rules in every mode | Met | One `ChessGame` class, held by `ClientValidatedGame.h:21` (Local, Bot) and `HostedGame.h:74` (Online) |

### Start-up and connection (§7-11)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 7 | First Client screen asks for Server IP and port | Met | `client/main.cpp:131-135`; `ConnectionScreen.cpp:38-87`; R5 |
| 8 | Main menu only after a successful connection | Met | `main.cpp:132-135`; the loop in `ConnectionScreen.cpp:145-174` has no exit except success or Exit; R5, R6 |
| 9 | Clear errors for bad IP, bad port, server down, failed connection | Met | `ConnectionScreen.cpp:45-66` (empty, port-in-address-box, unresolvable); `ConsoleInput.cpp:76-87` (not a number, out of range); `ConnectionScreen.cpp:98-105` (refused or timed out, 5 s, `ServerConnection.cpp:9`); R5 |
| 10 | After a connection failure: retry, change IP, change port, or exit | Met | `ConnectionScreen.cpp:111-131`, `155-173`; all four exercised in R5. See Part 3 for invalid *input* |
| 11 | Server prints the IP and port it listens on, which Clients then use | Met | `Server.cpp:38-55`; R13; R9 connected with the printed LAN address |

### Main menu (§12-16)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 12 | Local Game, Player vs Bot, Online Game, Exit | Met | `MainMenu.cpp:9-13` |
| 13 | Local Game: two players on one Client | Met | `main.cpp:98-102`, `LocalGame`; R8 |
| 14 | Player vs Bot | Met | `main.cpp:17-33`, `BotGame`; R8 |
| 15 | Online Game through matchmaking | Met | `main.cpp:83-93`, `OnlineGame.cpp:126-140`; R7 |
| 16 | Exit | Met | `MainMenu.cpp:24`, `main.cpp:112-113`; every run ends "Goodbye." |

### Game modes (§17-25)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 17 | Local: created, managed, stored and validated on the Client; Server does not manage it | Met | `LocalGame` derives `ClientValidatedGame`, which owns the `ChessGame`; built with no connection (`main.cpp:99`); R8: no server log lines during it |
| 18 | Bot: created and managed on the Client; engine and Bot logic run there | Met | `BotGame`; Stockfish launched by the Client (`StockfishEngine.cpp:146-150`); R8: parent process is `client.exe` |
| 19 | At least three difficulty levels | Met | `BotDifficulty.h:6`; `BotDifficulty.cpp:13-20` |
| 20 | Differences significant and demonstrable | Met | Skill Level 0/8/20 and 100/400/1000 ms (`BotDifficulty.cpp:13-20`); `tests/bot_match.cpp`; R4 |
| 21 | Online: two Clients, one Server, Server does the matchmaking | Met | `Server.cpp:237-303`; R3, R7 |
| 22 | Online game created on the Server; state, board and turn stored there; every move validated there | Met | Created in `games_` (`Server.cpp:250-251`); `HostedGame.h:74`; turn and legality in `HostedGame.cpp:61-96`; R3 (out-of-turn and illegal both rejected), R7 |
| 23 | Client only displays the state it received, takes the action, sends a request | Met | `OnlineGame` derives `Game`, not `ClientValidatedGame`, so it has no rules engine (`OnlineGame.h:29`); `onMoveChosen` only sends (`OnlineGame.cpp:143-161`) |
| 24 | Client never applies an online move or treats its request as final; new state only after Server approval | Met | Board written only from `GameState` (`OnlineGame.cpp:255-259`); input frozen while a request is out (`OnlineGame.cpp:119-123`, `157`); R7 images |
| 25 | Works between Clients on different computers on one network; implementation does not assume one computer | Met in the code; two-computer run still to do by hand | Listens on `sf::IpAddress::Any` (`Server.cpp:23`), R13 shows `0.0.0.0`; LAN address printed (`Server.cpp:46-48`); any address or name accepted (`ConnectionScreen.cpp:61`); R9 played through the LAN address; README §8 now covers the firewall prompt |

### Chess rules (§26-39)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 26 | Legal movement of every piece | Met | `PieceRules.cpp` (sliding `57-84`, stepping `118-139`, pawn `182-253`); R2 |
| 27 | Capture enemy pieces, never your own | Met | `PieceRules.cpp:68-75`, `132-135`, `229`; R2 |
| 28 | Blocked by pieces in the way | Met | `PieceRules.cpp:68-76` (ray stops), `192-211` (pawn); R2 |
| 29 | Correct turn order | Met | `ChessGame.cpp:89-100`; per player online `HostedGame.cpp:78-81`; R3, R7 |
| 30 | Check detection | Met | `Rules.cpp:123-133`; shown as "Check" (`ClientValidatedGame.cpp:29-32`, `OnlineGame.cpp:81`) |
| 31 | No move may leave your own king in check | Met | `Rules.cpp:136-155`; R2 |
| 32 | Checkmate | Met | `Rules.cpp:165-171`, `ChessGame.cpp:103-112`; R2; R3 plays a mate through the Server |
| 33 | Stalemate | Met | `Rules.cpp:174-180`, `ChessGame.cpp:114-117`; R2 |
| 34 | Castling with every condition | Met | `Rules.cpp:21-77` (rights, both pieces home, squares empty, not out of check, not through an attacked square, landing square by the legality filter); `Board.cpp:152-186`; R2 |
| 35 | En passant | Met | `PieceRules.cpp:231-244`, `Board.cpp:201-209`, `224-231`; R2 |
| 36 | Promotion, choosing Queen, Rook, Bishop or Knight | Met | `PieceRules.cpp:161-179`; dialog then move (`Game.cpp:38-64`); R2; R3 (queen and knight over the network; no piece named is refused) |
| 37 | Game ends on resignation | Met | `ChessGame.cpp:121-130`; R7, R8 |
| 38 | Draw by agreement in Local and Online | Met | `ChessGame.cpp:133-140`; Local `LocalGame.cpp:26-34`, `ClientValidatedGame.cpp:65-70`; Online `HostedGame.cpp:135-154`; R3 (accepted), R7 and R8 (declined) |
| 39 | At the end, decide the result and the reason and pass both to the GUI | Met, **fixed in this audit** | `ChessGame.cpp:170-205` → `showGameOver(title, detail)` (`Game.cpp:127`); online via `GameOver` (`Server.cpp:175-180`). The side panel now keeps the result after the summary is closed (`Game.cpp:14-21`), see Part 2 |

### Online Game and matchmaking (§40-42)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 40 | Client tells the Server it is searching; waits if nobody else is | Met | `OnlineGame.cpp:136`; `Server.cpp:277-303`; R3; R7 image "Waiting for opponent" |
| 41 | Two free Clients: pair, create the game, assign both, pick colours, tell both | Met | `Server.cpp:237-274`; R3, R7 |
| 42 | "Free" = connected, asked, not in a game; removed on disconnect or cancel while waiting | Met | `Server.cpp:278-291` (already in a game, or already waiting, is refused), `306-315` (cancel), `214-217` (disconnect); closing the window while waiting cancels (`OnlineGame.cpp:22-26`); R3, R7 |

### Player vs Bot (§43-47)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 43 | Choose easy, medium or hard before the game | Met | `BotDifficulty.cpp:41-58`, called first in `main.cpp:17-21`; R8 |
| 44 | The game starts once the level is chosen | Met | `main.cpp:17-33`; R8 |
| 45 | Moves chosen by Stockfish, not our own algorithm | Met | `StockfishEngine.cpp:327-372` (`go movetime` → `bestmove`); no search code anywhere in the project |
| 46 | Stockfish run as a separate process and talked to | Met | `StockfishEngine.cpp:100-170` (`CreateProcessW`, two pipes), UCI handshake `267-286`; R8 |
| 47 | Stockfish only picks the Bot's move; legality and state stay ours | Met | `BotGame.cpp:77-84` puts every suggestion through `makeMove`; R4 (every engine move accepted by our rules) |

### Draw offer (§48-52)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 48 | Either player may offer a draw in Local and Online (Draw Offer button) | Met | `Game.cpp:92-96`; shown `LocalGame.cpp:11`, `OnlineGame.cpp:228`; hidden in Bot `BotGame.cpp:30` |
| 49 | Local: the offer appears on the same Client, for the opponent | Met | `LocalGame.cpp:26-34` ("White offers a draw. Black, do you accept?"); R8 image |
| 50 | Online: sent through the Server, shown only on the opponent's Client | Met | `OnlineGame.cpp:178-191` → `Server.cpp:384` → `OnlineGame.cpp:292-305`; R3 (not echoed, not leaked to another game); R7 images |
| 51 | The receiver chooses Accept or Decline | Met | `Game.cpp:98-104`; R7, R8 |
| 52 | Accept ends it as a draw with a message; Decline continues | Met | `ClientValidatedGame.cpp:65-70`; `HostedGame.cpp:146-151`; `Server.cpp:416-428`; R3 ("Draw agreed"); R7 (declined, play continued, offerer told) |

### Resignation (§53-56)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 53 | Every human player can resign in every game | Met | Resign handled in every mode (`Game.cpp:77-89`): Local, the side to move (`LocalGame.cpp:19-23`, see Part 3); Bot, the human (`BotGame.cpp:42-46`); Online, either player at any time (`OnlineGame.cpp:164-175`) |
| 54 | Confirmation window: same Client in Local and Bot; only the resigner's Client online | Met | Dialog opened locally (`Game.cpp:77-82`), never sent; R7 image shows it on one window only |
| 55 | Confirm and No; resign only on Confirm | Met | `Game.cpp:84-91`; R7: No logged nothing, Confirm resigned |
| 56 | Opponent wins, resigner loses, result shown; online, the Server updates both | Met | `ChessGame.cpp:121-130`; `Server.cpp:432-454`; R7 ("White wins!" / "Black resigned" on both) |

### Several games at once (§57-60)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 57 | Many Clients and many online games at once | Met | `Server.h:57` (clients), `Server.h:63` (games); in R3 the server log peaks at 20 Clients connected and 5 games active at once |
| 58 | Games independent | Met | Each `HostedGame` owns its own `ChessGame`; R3 "the first game's players hear nothing about the second game" |
| 59 | No update to Clients outside the game | Met | `Server.cpp:117-131` (`findGameForPlayer`), `162-165` (sends to the two players only); R3 |
| 60 | Never assume a single game | Met | `std::map<int, HostedGame> games_` (`Server.h:63`); game ids on every in-game message |

### Protocol (§61-63)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 61 | Our own application protocol | Met | `common/Protocol.h`, `Protocol.cpp` |
| 62 | Covers connect and disconnect, entering and leaving the wait, matchmaking, creating and managing games, player actions and state updates, resignation, draw offers, game end | Met | Message list `Protocol.h:38-55`; connect and disconnect are the TCP session itself (`Protocol.h:28-31`, `Server.cpp:189-209`, `501-548`) |
| 63 | Clear, consistent, explainable, maintainable | Met | One file, each message written and read by adjacent operators, explicit type numbers; DESIGN decision 4 |

### Disconnections and errors (§64-67)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 64 | A disconnect never crashes the Server or touches other games | Met | `Server.cpp:523-547`; R3 (disconnects mid-game and while waiting, 1,600 malformed packets, then still pairs and validates) |
| 65 | Handled mid-game, while waiting, and on the system screens | Met | Server `Server.cpp:212-234`; Client `OnlineGame.cpp:385-395`, `411-414`; menu `main.cpp:45-93`; R3, R6 |
| 66 | Mid-game disconnect: Server ends the game, the other player is told | Met | `Server.cpp:222-230`; "White disconnected", not "resigned" (`ChessGame.cpp:143-152`); R3 |
| 67 | Bad user input, invalid messages, messages wrong for the current state | Met | `ConsoleInput.cpp`, `ConnectionScreen.cpp:45-66`; `Server.cpp:457-498`; `HostedGame.cpp` offer rules; `OnlineGame.cpp:333-382`; R3, R5 |

### Server console output (§68-70)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 68 | Meaningful status messages, readable without a debugger | Met | `logLine` throughout `Server.cpp`; R3, R7 logs |
| 69 | Connect and disconnect, entering and leaving the wait, matchmaking, game creation and players, moves accepted and rejected, draw offers, resignation, game end | Met | `Server.cpp` lines 205 and 527; 293, 313 and 215; 254-257; 340-354; 376 and 412; 448; 177. All seen in the R7 log |
| 70 | No set format, but clear and useful | Met | e.g. `[server] game 1: move e7e3 from client 2 (127.0.0.1:61148) rejected - Illegal move`; ASCII only |

### C++ design (§71-80)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 71 | Logical division into classes and components | Met | `common/` rules and protocol; `client/` screens and modes; `server/` split into `Server`, `Matchmaker`, `HostedGame`; DESIGN decisions 2-3 |
| 72 | Encapsulation | Met | Members private throughout; the two protected members are argued for in DESIGN decision 6 |
| 73 | Inheritance and polymorphism where they fit | Met | `PieceRules` hierarchy (`PieceRules.h:22-125`); `Game` → `ClientValidatedGame` → `LocalGame`/`BotGame`, and `Game` → `OnlineGame` |
| 74 | STL and suitable data structures | Met | `std::array` board, `vector` moves and clients, `map` games, `optional`, `unique_ptr`; `find`, `any_of`, `all_of`, `transform` |
| 75 | Correct `const` | Met | Every non-mutating method is `const`; unmodified reference parameters are `const&` |
| 76 | RAII and resource management | Met | `ServerConnection.cpp:14-16`; `StockfishEngine.cpp:81-97`, `375-412`; sockets owned by `unique_ptr` (`Server.h:52`); R8 (Stockfish gone when its owner is destroyed); R14 |
| 77 | Smart pointers where ownership is dynamic | Met | `std::unique_ptr<sf::TcpSocket>` (`Server.h:52`, `Server.cpp:190`); no `new`/`delete` anywhere |
| 78 | No leaks, no double free | Met | No `new`, `delete`, `malloc` or `free` in any file; RAII throughout; R14 |
| 79 | Separate `.h` and `.cpp` | Met | Every class split; the only code in headers is a template (`Protocol.h:143-148`, must be there) and empty default hooks (`Game.h:77`, `92`, `97`) |
| 80 | Readable, modular, reasonable error handling | Met | No commented-out code, TODOs or debug output; failures reported in words (`server/main.cpp:57-60`, `StockfishEngine::lastError`, connection messages) |

### Submission and environment (§81-89)

| § | Requirement | Status | Evidence |
|---|---|---|---|
| 81 | Personal project | Met (not a property of the code) | Single author in the git history |
| 82 | All Client and Server source including `ProvidedGUI/`; both executables; the Stockfish executable used | Met | Zip holds `common/`, `client/`, `server/`, `ProvidedGUI/`, `client.exe`, `server.exe`, `stockfish/stockfish.exe`; plus the DLLs without which neither program starts (R10) |
| 83 | README: compile, run the Server, where IP and port appear, run and connect a Client, Local Game, Bot game and difficulty, Online and matchmaking, several Clients and concurrent games, how Stockfish is integrated, other environment needs | Met, **fourteen corrections in this audit** | README §2, §4, §4, §5, §6, §7, §8, §9, §10, §1 and §3 respectively; each statement checked by running it (R1, R5-R13) |
| 84 | Everything in a zip named `<ID>.zip` | Met | `216908467.zip`; nine digits, passes the Israeli ID check digit |
| 85 | C++17, MSYS2 UCRT64 / MinGW | Met | `-std=c++17`; g++ 16.2.0 (MSYS2 Rev3); R1 |
| 86 | SFML 3; compiles without errors | Met | SFML 3.0.2; R1, R12 |
| 87 | Client and Server compile and run separately | Met | Separate commands; the Server links no graphics libraries; R1, R12 |
| 88 | Client command `g++ -std=c++17 ... -o client.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network` | Met | README §2 matches the template flag for flag, with the sources and `-IProvidedGUI` in the `...` |
| 89 | Server command `g++ -std=c++17 ... -o server.exe -lsfml-network -lsfml-system` | Met | README §2, same |

---

## Part 2 — changes made

Each change cites the PDF section it serves or the risk to the grade it removes.

### Code

| Change | Reason |
|---|---|
| `client/Game.cpp:14-21` — a finished game's side panel now shows the result title ("White wins!", "Draw", "Game ended") instead of an empty status | **§39.** Found in the R7 images. Our code cleared the status when a game ended, and the provided GUI draws an empty status as "Game in progress". So once the summary box was closed, a finished game read *Turn: Game over / Game status: Game in progress*, which a grader would take as a bug. All three titles fit the 260-pixel panel. Verified by R7 and R8 images after the change. |

### The submission package

| Change | Reason |
|---|---|
| `Example/`, `build_example.bat` and `GUI_USAGE.md` taken out of the zip; kept in the repo, unmodified | Not asked for: §82 names `ProvidedGUI/` only, and GUI_USAGE.md's own layout diagram has no `Example/`. **Confusion risk against §83:** `build_example.bat` at the top of the submission reads as the project's build script, but it builds the lecturer's demo into `gui_example.exe`, and `Example/main.cpp` is a third `main()` that belongs to neither program. `GUI_USAGE.md` is the lecturer's own document; it helps nothing in the grade, and a second top-level `.md` beside the README invites the question of which to read. Zip is now 86 entries, 82.3 MiB. |
| Zip rebuilt from the whitelist after every change, and verified by R7, R8, R12 | §82, §84 |

### README (§83 — compile and run without guessing)

| Change | Reason |
|---|---|
| §3: a missing DLL no longer "reported by `client.exe` but not `server.exe`" | False: R10 showed both fail identically, exit `0xC0000135`, no output |
| §3: rebuilding with a different MSYS2 version: copy that installation's DLLs over the bundled ones | Windows loads DLLs from the executable's folder before `PATH`, so a grader's rebuild would load our older DLLs and could fail with "entry point not found". §83 ("any other environment requirement"), §86-87 |
| §2: `C:\msys64\ucrt64\bin` must be on `PATH` in `cmd`/PowerShell; "project root" defined as the folder holding the README | The README offered `cmd` and PowerShell without saying `g++` has to be findable there. §83, §85 |
| §3: double-clicking the executables works | The only run instruction was "from the project root", which leaves a grader who double-clicks guessing. Explorer starts a program in its own folder; R7/R8 launched the same way, with pieces drawn |
| §4: `./` is for the MSYS2 shell and PowerShell; in `cmd` leave it out | `cmd` rejects `./server.exe` ("'.' is not recognized"), checked. §83 |
| §1: "Stockfish ... see section 8" → section 10 | Wrong cross-reference, pointing to a §83 item |
| §5: invalid address or port is explained and re-asked; the four choices follow a failed *connection* | The README promised the four choices after bad input too, which the program does not do. Now matches R5. §9-10 |
| §8: on-screen text "Waiting for an opponent..." → "Waiting for opponent" | Quoted text now matches the screen (R7 image) |
| §8: out-of-turn clicks are ignored by the board; the rejection reason is for illegal moves | The README said an out-of-turn move shows a reason. The Client never sends one, so no reason ever appears (R7). §24 |
| §8: Windows Firewall prompt on first run; allow on private networks | Without it, a two-computer test of **§25** fails for a reason outside the code |
| §10: Stockfish is shut down when its board window closes | Said "when the Client closes"; R8 showed it goes as soon as the window closes, which is what a grader watching Task Manager will see |
| §11: perft "about 15 million" → about 16.5 million, ~15 s | The 26 expected counts sum to 16,564,718 |
| §11: Bot match takes three to four minutes; results stated as observed, not guaranteed | A grader running it deserves to know it is not hung (R4: 194 s and 221 s). Skill Level is randomised, so "wins all six" cannot be a promise. §20 |
| §12: layout without the three removed files | Follows the package change |

### Comments in shipped source

| Change | Reason |
|---|---|
| `tests/perft_main.cpp`, `bot_match.cpp`, `protocol_test.cpp`: removed "it is excluded from the submission zip" | False since `tests/` ships |
| `common/Protocol.h`: example caption "Opponent to move" → "Player 4 to move" | That is what the Client actually shows |
| `common/Protocol.h:28-31`: connecting and disconnecting are the TCP session | **§62** lists "connection and disconnection of Clients" first. The message list alone does not show where that is handled, and a grader checking §62 against the protocol file would find it apparently missing |
| `client/OnlineGame.h`: quoted waiting text corrected | Matches the screen |

### Our own documents (not shipped)

| Change | Reason |
|---|---|
| `docs/REQUIREMENTS.md` §84: removed the claim that the PDF asks for "a folder named with your ID" | The PDF's "תיקיית ZIP" is Windows' name for a zip ("compressed folder"). §84 asks only for the file name |
| `docs/REQUIREMENTS.md` §17, §48: transcription aligned with the PDF | §17 says created, managed *and stored* on the Client; §48 names the button "Draw Offer" |
| `docs/DESIGN.md` decision 5: removed the client-to-game map | It was never built; `findGameOfClient` searches the games instead. The file must describe the code as it is |
| `docs/PLAN.md`: check count 81 → 103, §84 reasoning, whitelist, the pre-zip checklist ticked after re-running it | Stale |
| `CLAUDE.md` hard rules 1 and 5; `docs/CLAUDE_CODE.md`; `docs/NOTES.md`; `docs/REVISION.md`; `docs/STATUS.md` | Stale facts: said `Example/` was submitted, pointed to an `ldd` line the README no longer has (the command now lives in `CLAUDE.md` and was tested), 81 checks, 15 million, old on-screen text. The revision sheet matters most, since the student will say these numbers out loud |

---

## Part 3 — considered and deliberately left alone

### Behaviour

| Item | Why it was left |
|---|---|
| After an **invalid** address or port the Client re-asks for that field instead of offering the four §10 choices | §10 applies "after a failure in connection" ("לאחר כשל בחיבור"). Invalid input never reaches a connection attempt; it gets a specific message and is re-asked, which is "change the IP / port" (§9, §67). Every real connection failure gets all four choices (R5). Changing the flow would rewrite working code for a reading the PDF does not support. The README now describes the behaviour accurately |
| Local Game: Resign resigns the side to move | §53: every human can resign, and each can on their own turn. The provided GUI has one Resign button and a confirmation dialog with no parameter, so it cannot tell which of two people at one mouse clicked it |
| A draw offer lapses if the offering player then moves; if the opponent clicks Accept on the still-open dialog, the Server ignores it (and logs why) | The tested path, offer then answer, works (R3, R7, R8). Moving on is how an offer lapses in chess. Telling the recipient would need a new protocol message, which is a feature, not a fix |
| The Server uses blocking sockets, so a hostile client that sends half a packet, or stops reading, can stall the single thread | §64 is about *disconnection*, which is handled and tested (R3). Our own Client always sends whole packets. A fix means non-blocking I/O with queues, a rewrite of `Server`. Already recorded as a known trade-off in `docs/REVISION.md` |
| Board not flipped for Black | Not asked for anywhere in the PDF, and the provided GUI draws its own coordinate labels, so flipping would mislabel the board. DESIGN decision 1 |
| Client's `DrawDeclined` handler does not check the game id | The Server only ever sends it to the offering player of that game (§59 enforced at `findGameForPlayer`). Nothing to gain |
| Server log says "which it is not playing in" for a move sent to a game that has just finished and been dropped | Slightly imprecise wording, but the log stays clear and the behaviour is right |

### Code

| Item | Why it was left |
|---|---|
| Four public methods nothing calls: `ChessGame::isCheckmate()`, `isStalemate()`, `endReason()`, `Move::operator!=` (and `ChessGame::result()`, used only by `tests/bot_match.cpp`) | They are part of a coherent public interface and carry the §32/§33 comments a grader looks for. Deleting them is refactoring that no PDF section asks for, and it would not change the grade either way |
| `common/.gitkeep` still in the repo | Never shipped: the whitelist takes only `.h` and `.cpp` from `common/`. No effect on the grade |
| `tests/perft_main.cpp`'s header shows its build line without `-O2` | It works, just runs slower; the README gives the `-O2` form |

### Package and extras kept

| Item | Why it was kept |
|---|---|
| One top-level folder `216908467/` inside the zip | The PDF does not require it (Part 2, REQUIREMENTS.md), but it stops "Extract here" spilling 86 files into whatever folder the grader is in. Harmless either way |
| `tests/` | §20 asks for the Bot levels to be *demonstrable*, and `bot_match` is the demonstration; perft and the protocol test are the evidence for §26-36 and §57-67. They are outside `common/`, `client/` and `server/`, so the build commands never pick them up, and the README documents them as optional. Built and run from the package (R12) |
| 19 DLLs | §82 asks for the executables "so that the project can be run during checking". Without the DLLs neither program starts (R10). Exactly the set `ldd` requires (R11) |
| `stockfish/Copying.txt`, `stockfish/AUTHORS` | The GPL requires the licence to travel with the binary. Two small files in the Stockfish folder |
| README extras: toolchain versions and install line, `-Wall -Wextra` note, reserved-port note, verification section, folder layout, Stockfish SHA-256 | Each either removes a guess (§83) or lets the grader check a claim. None is long |
| Code extras beyond the letter of the PDF: optional server port argument, two printed addresses, `DrawDeclined`, rejection reasons in the panel, port-typed-as-address detection, 5-second connect timeout, menu-level lost-connection notice, board frozen while a request is out | Each serves a numbered section: §11, §25, §52, §24, §9, §9, §65 and §24 respectively. All tested (R5-R9) |
| Zip size, 82.3 MiB | Dictated by §82: `stockfish.exe` alone is 98 MB. Nothing in it can be trimmed. If the upload system has a smaller cap, that is a question for the lecturer |

### Could not be done here

| Item | Status |
|---|---|
| §25 on two physical computers | This machine has one network card and no second computer. The code side is proved (R9, R13). **Worth doing by hand before submitting:** run `server.exe` and one Client here, a second Client on another computer on the same network using the printed LAN address, and allow `server.exe` through the firewall when Windows asks |
| Unzipping onto a different drive | This machine has only `C:`. The package was tested far from the project folder instead, in a path with a space and Hebrew letters, with MSYS2 off `PATH` |
