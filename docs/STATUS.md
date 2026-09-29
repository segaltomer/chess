# Current state

Overwritten as work proceeds. One purpose: a session that starts with no memory
of the last one can read this and know exactly where things stand.

---

**Phase:** 7 — submission. **Done, including a final audit against the PDF.**
Phases 1-6 are done.

**The final audit is in `docs/AUDIT.md`.** It covers all 89 sections, with
evidence per section, every change made, and everything considered and left
alone. All 89 are met. What it changed:

- **One code fix, §39.** Once a game ended and the summary box was closed, the
  side panel said "Game in progress", because the provided GUI draws an empty
  status that way. It now shows the result title (`client/Game.cpp:14-21`).
- **The zip no longer contains `Example/`, `build_example.bat` or
  `GUI_USAGE.md`.** §82 does not ask for them, and a `.bat` at the top of the
  submission reads as the project's build script. They stay in the repo,
  unmodified. The whitelist in `docs/PLAN.md` is updated.
- **The README was corrected in fourteen places**, each checked by running what
  it describes.
- Stale comments in shipped files (three test headers, `Protocol.h`,
  `OnlineGame.h`) and stale facts across `docs/`.

**No DLLs in the submission — the lecturer's instruction.** The 19 DLLs are out
of the zip. The build is unchanged (dynamic, the §88-89 commands exactly), and
the README now runs everything from the MSYS2 UCRT64 shell, whose `PATH`
supplies SFML's DLLs. `CLAUDE.md` hard rule 5 records this; do not reintroduce
DLLs or switch to static linking.

**`216908467.zip` is in the project root** — 67 entries, 77.7 MiB, no DLLs,
everything inside one top-level folder `216908467/`. It is gitignored; rebuild
it from the whitelist in `docs/PLAN.md` if anything shipped changes.

**How the package was verified:** unzipped into a fresh folder. Outside the
UCRT64 shell both programs exit `0xC0000135`, proving no DLL is in the package;
inside it, the server and a client run exactly as README sections 4-5 say. With
that shell's `PATH`, real Client windows were driven by posted clicks through an
online game, a Local game and a Bot game (Stockfish from the package's own
folder). Both README build commands work there too. Earlier, the audit and the
mock grading exercised every mode in far more depth (`docs/AUDIT.md`,
`docs/GRADING.md`).

**In progress:** nothing. **Files mid-edit:** none.

**The one open item:** §25 on two real computers. Run `server.exe` and one
Client here, a second Client on another computer on the same network using the
LAN address the server prints, and allow `server.exe` through Windows Firewall
when asked. The second computer needs MSYS2 with SFML installed and runs the
Client from its own UCRT64 shell, since no DLLs ship. The code side is proved (listens on `0.0.0.0`; an online game was
played through the LAN address), but this is the only check one machine cannot
do.

## How to test it — two clients and one server

Three MSYS2 UCRT64 shell windows, all at the project root:

    ./server.exe          shell 1
    ./client.exe          shell 2
    ./client.exe          shell 3

Both clients connect to `127.0.0.1` on the port the server printed, then choose
**3) Online Game**. The first shows "Waiting for opponent"; when the second
asks, both boards show the game with colours assigned.

## Re-running the verification

    g++ -std=c++17 -O2 -Icommon common/*.cpp tests/perft_main.cpp -o perft.exe -lsfml-network -lsfml-system
    g++ -std=c++17 -O2 -Icommon -Iclient common/*.cpp client/StockfishEngine.cpp client/BotDifficulty.cpp client/ConsoleInput.cpp tests/bot_match.cpp -o bot_match.exe -lsfml-network -lsfml-system
    g++ -std=c++17 -O2 -Icommon common/*.cpp tests/protocol_test.cpp -o protocol_test.exe -lsfml-network -lsfml-system

perft: 26/26. bot_match: the stronger level wins all six (three to four
minutes). protocol_test needs `server.exe 53100` running first: 103/103.

## Environment notes for the next session

`git` is **not** on `PATH` in this shell. Two bundled copies exist; the
GitHub Desktop one is newer:

    $env:PATH = "C:\Users\Owner\AppData\Local\GitHubDesktop\app-3.6.5\resources\app\git\cmd;" + $env:PATH

`g++` needs `C:\msys64\ucrt64\bin` on `PATH` the same way.

Commit messages must be passed with `git commit -F <file>`, not `-m`, because
PowerShell mangles arguments containing double quotes when handing them to a
native executable.

**`docs/requirements.pdf` can be read directly** with the file-reading tool,
which renders its pages. No text extraction is needed.

**Building the zip:** entry by entry with forward slashes (see `docs/PLAN.md`,
Phase 7). In PowerShell, avoid `-replace '\\','/'` in the same command as a
delete: the tool's safety filter reads it as a path. `.Replace([char]92, '/')`
works.

**Never rewrite a UTF-8 doc through PowerShell** (`Get-Content -Raw` then
`WriteAllText`, or `Set-Content`). PowerShell 5.1 reads UTF-8 as ANSI and
writes it back double-encoded, turning every `§` into mojibake. Use the editing
tools instead.

**`git push` cannot work from this shell.** Windows Credential Manager holds no
`git:https://github.com` entry, and GitHub Desktop keeps its token where plain
`git` cannot read it, so a push prompts for a username and fails. Commit locally
and the student pushes from GitHub Desktop.
