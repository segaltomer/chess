# Project constraints

Final C++ course project: a chess system split into a Client and a Server.
Read `GUI_USAGE.md` before touching anything GUI-related.

## Toolchain — fixed by the course requirements, do not change

- MSYS2 **UCRT64** / MinGW-w64 at `C:\msys64`, C++17
- g++ 16.2.0, SFML 3.0.2, gdb 17.2
- Do **not** propose or introduce MSVC, Visual Studio project files, CMake,
  vcpkg, Conan, or any other build system. The grader compiles with `g++`
  using the command lines in the README.

Build from the project root:

```bash
# example
g++ -std=c++17 Example/main.cpp ProvidedGUI/ChessFrontend.cpp -IProvidedGUI \
    -o gui_example.exe -lsfml-graphics -lsfml-window -lsfml-system
# client
g++ -std=c++17 common/*.cpp client/*.cpp ProvidedGUI/ChessFrontend.cpp \
    -IProvidedGUI -o client.exe \
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
# server
g++ -std=c++17 common/*.cpp server/*.cpp -o server.exe \
    -lsfml-network -lsfml-system
```

## Hard rules

1. **Never modify anything in `ProvidedGUI/`.** Those files are supplied by the
   lecturer and are submitted unchanged. They have been verified byte-identical
   to his zip. The same goes for `Example/` and `build_example.bat`.
2. **No absolute paths anywhere in the source.** The project is graded on the
   lecturer's machine. Resolve everything relative to the executable or the
   working directory.
3. **Stockfish is launched as a separate process by relative path.** The
   binary is already present at `stockfish/stockfish.exe` (Stockfish 19,
   Windows x86-64 universal). Resolve it relative to the running executable;
   never hardcode an absolute path. Stockfish chooses Bot moves only — move
   legality and game state stay in our own code. UCI protocol reference:
   the Stockfish wiki page `UCI-Protocol-and-Stockfish-Commands.md`.
4. Assets are resolved relative to the **current working directory**
   (`ProvidedGUI/assets/`, then `assets/`, then one and two levels up), so
   executables must be launched from the project root.
5. Runtime needs 18 MinGW/SFML DLLs beside the executables. They are
   gitignored but **must be included in the submission zip**.

## Working documents — read these first

- `docs/DESIGN.md` — the design decisions. **This is the source of truth for
  architecture**, and it is complete: all six decisions are made, with the
  reasoning recorded. Build against it. You may change a decision under the
  rules in "Open to change" below — but rewrite the file when you do, so it
  never describes a design the code no longer has.
- `docs/PLAN.md` — build order with requirement numbers. Work one item at a
  time and tick items off as they pass.
- `docs/REQUIREMENTS.md` — all 89 requirements as a checklist, for verification.
- `docs/NOTES.md` — the running explanation of how the code works. You add to
  this as you build; see "Explaining the work as you go" below.
- `docs/REVISION.md` — question-and-answer revision sheet. You append to it at
  the end of every phase. This is what the student reads before the test.
- `docs/CLAUDE_CODE.md` — the student's own operating notes for running you.
  Not instructions for you, but read it once so your advice matches what it
  tells them to do.
- `docs/STATUS.md` — where the work currently stands. You overwrite this when
  starting and finishing every task; it is what makes an interrupted session
  resumable. See "Resuming after an interrupted session" below.
- `docs/requirements.pdf` — **the lecturer's original requirements document**
  (Hebrew). This is the authority. `REQUIREMENTS.md` is a transcription made
  for convenience and could contain a mistake; if the two ever disagree, the
  PDF wins and the transcription gets corrected. Read the PDF directly whenever
  a requirement's exact wording matters.

## Two categories: requirements vs. design choices

Keep these strictly apart. Confusing one for the other is the main way this
project goes wrong.

### Non-negotiable — never change these

These come from the lecturer's requirements document, not from us. They are not
open to improvement, however much better an alternative looks:

- Every numbered requirement (see `docs/REQUIREMENTS.md`, all 89).
- The toolchain: C++17, MSYS2 UCRT64 / MinGW, SFML 3, `g++`. No MSVC, CMake,
  vcpkg or alternative build system.
- `ProvidedGUI/`, `Example/` and `build_example.bat` stay byte-for-byte
  unmodified.
- Two separate programs, each compiling and running independently (§1, §87).
- Networking via SFML Network (§5).
- Stockfish as a separate process for Bot moves only (§45-47).
- The Online move flow in §22-24: the Server owns the board and validates; the
  Client never applies a move locally or treats its own request as final.
- §60: never assume a single game exists on the Server.
- The rules engine compiles into both binaries (forced by §6 + §17-18 + §22).
- No absolute paths anywhere in the source.

If a requirement seems to conflict with good design, say so and follow the
requirement. If two requirements seem to conflict with each other, stop and ask.

### Open to change — `docs/DESIGN.md`

Everything in `docs/DESIGN.md` is a choice, not a rule. Some of it the student
picked from options they had explained to them; some of it is a default that
seemed reasonable at the time. The student has explicitly said: **if you see a
clearly better way, take it.**

So you may change these. The conditions:

1. **The bar is "clearly better", not "differently structured".** A real
   reduction in complexity, bugs or duplication — not taste, not a refactor for
   its own sake, and not a rewrite of working code.
2. **Say so before or as you do it**, in one or two sentences: what you are
   changing, and what it buys. Never drift silently.
3. **Update `docs/DESIGN.md` in the same commit**, replacing the old decision
   and its reasoning with the new one. The file must always describe the code as
   it actually is — the student has to be able to explain this design, so the
   rationale trail is the point, not the paperwork.
4. **Don't change one to dodge a hard requirement.** If the current design makes
   some requirement awkward, that is usually the design being wrong in an
   interesting way — explain it rather than routing around the requirement.

When the student asks for something that would break a non-negotiable item,
tell them which requirement it breaks instead of doing it.

## Git — maintain it continuously

The repo is already initialised on branch `main`, with user `Tomer
<tomerontomer@gmail.com>` configured locally and one commit of the setup.
Keeping history useful is part of your job here, not an afterthought.

- **Commit every time something works.** A working unit is: it compiles, it
  runs, and it does what the task said. One task from `docs/PLAN.md` is
  usually one commit.
- **Never commit broken code to `main`.** If a change leaves the project not
  compiling, either finish it or say so and leave it uncommitted. A history
  where every commit builds is what makes `git bisect` and "what did I break"
  answerable.
- **Messages:** a short imperative subject line saying what changed, and when
  the reason isn't obvious, a body explaining why. English. Reference the
  requirement where one applies, e.g. `Add en passant capture (§35)`.
- **No AI attribution lines.** No `Co-Authored-By`, no generated-with trailers,
  nothing naming a tool. The student has asked for a clean project and this is
  part of that.
- **Never commit build output.** `*.exe`, `*.dll`, `*.o`, `*.a` and
  `stockfish*` are gitignored — don't `git add -f` past that. `stockfish.exe`
  in particular is 103 MB and exceeds GitHub's 100 MB per-file limit; it would
  be rejected on push. It is recoverable anyway: the README records its version
  and SHA-256.
- **Everything else is tracked on purpose,** including `docs/`, `.vscode/` and
  this file. Git is the off-machine backup, so anything ignored is a thing that
  would be lost if the machine died. The submission zip excludes them manually
  at zip time instead — see `docs/PLAN.md` Phase 7.
- **Update `docs/PLAN.md` and `docs/STATUS.md` in the same commit** as the work
  they describe, so the checklist, the state file and the code never disagree.
- **When you change a design decision**, commit the `docs/DESIGN.md` edit
  together with the code that depends on it, and say why in the commit body.
  That pairing is the rationale trail the student needs when explaining the
  design later.
- **Before each phase boundary**, check `git status` is clean and mention where
  things stand.

- **Push after every commit.** A private GitHub remote exists and is the
  off-machine backup. `git push` after each commit, without being asked. If a
  push fails, say so once and carry on committing locally — never stop the work
  over it, and never try to authenticate.

## Resuming after an interrupted session

A session can end at any point — usage limits, a closed window, a new day —
and the next one starts with no memory of this one. Everything needed to
continue must therefore be on disk, not in the conversation.

**First actions in a new session, in this order:**

1. Read this file.
2. Read `docs/STATUS.md` — where things stand, and what was mid-edit.
3. Run `git log --oneline` and `git status`.
4. Read `docs/PLAN.md` and continue from the first unticked item.

If `STATUS.md` says something was mid-edit, check that work before starting
anything new: either finish it or, if it is unsalvageable, `git checkout` the
affected files back to the last commit and redo it. Never start a new task on
top of a half-finished one.

**Keep `docs/STATUS.md` current — this is not optional.** Overwrite it when you
start a task, and again when you finish one. It is the only thing standing
between an interruption and a confused restart. An unticked box in `PLAN.md`
cannot tell the next session whether a task is untouched or half-written;
`STATUS.md` can.

Since every commit builds and leaves `main` working, the worst an interruption
can cost is the uncommitted work in progress that `STATUS.md` describes.

## Working autonomously

**Do not wait for approval to proceed.** The student prefers to review written
records rather than answer questions mid-build. Work straight down
`docs/PLAN.md`, one item at a time, and keep going. When something is
ambiguous, make the reasonable choice, state it in one line, record it in
`docs/DESIGN.md` if it is a design decision, and continue.

Only stop for these, and nothing else:

- A genuine conflict between two requirements.
- Something that needs credentials or an account — you cannot enter those.
- A choice that is expensive to reverse later *and* has no clearly better
  option, where guessing wrong means rewriting a phase.
- A requirement you cannot satisfy and want to flag rather than fake.

Tests, builds, fixes and commits all proceed without asking. Explanations are
written down (below), not delivered as questions.

**When you do stop and ask, write the question so a third party could answer
it.** The student may take it to someone else to think through, so a question
that assumes the conversation so far is useless to them. Include, briefly:

- what you are trying to do, and which requirement drives it;
- the options you see, with the trade-off of each;
- which one you would pick and why, so an answer of "go with yours" works;
- what it costs to change your mind later.

Say plainly whether it blocks you. If it doesn't, say what you are doing
meanwhile — never sit idle waiting for an answer when there is other work in
`docs/PLAN.md` you could be doing.

### Efficiency yes, shortcuts no

Autonomy means "don't stop to ask", not "get it done quickly". There is no time
budget on this work and you are not measured on how fast it finishes.

One test decides every judgement call here: **does this cost a mark?** Being
efficient where nothing is graded is good work. Being efficient where something
is graded is not efficiency, it is a lost mark. When unsure which side a call
falls on, do the fuller thing.

**Be lean here — it costs nothing:**

- Don't gold-plate. A requirement satisfied cleanly is done; keep going rather
  than polishing past it.
- Don't add features nobody asked for. Nothing outside
  `docs/REQUIREMENTS.md` earns a mark, and every extra feature is more surface
  to break and more code the student has to be able to explain.
- Don't build abstractions the design doesn't need. §73 wants polymorphism
  *where it fits*; a layer added for elegance reads as noise against §71 and
  §80.
- Reuse instead of duplicating, and do mechanical work in batches.
- Keep explanatory writing short. `NOTES.md` and `REVISION.md` are revised
  from, not admired.

**Never compress these:**

- Any requirement in `docs/REQUIREMENTS.md`. No stubbing, no simplifying, no
  deferring, no partial version that looks right in a demo.
- No `// TODO` and no "good enough for now" on anything graded. Placeholders
  belong only in code you are actively mid-way through.
- Robustness (§64-70) and the quality pass (§71-80). They look skippable
  because they add no features. They are sixteen graded requirements.
- Testing. Two real clients against the Server for anything online, deliberate
  disconnections, deliberate bad input. Assuming it works is not testing it.
- The order in `docs/PLAN.md`, which is by dependency — reordering to finish
  sooner means building on something unfinished.

When one approach is more work but clearly better against the requirements,
take the more work. If something genuinely cannot be done, say so and explain
why rather than quietly shipping a weaker version.

If the student asks for a shortcut, say which requirement it costs and let them
decide with that in front of them. If it costs nothing, just do it.

## Explaining the work as you go

The student has to be able to explain this project out loud, and is learning
C++ project design while it is being built. Treat that as part of the
deliverable, not a favour.

All of it is written to files rather than delivered in conversation, so it
survives being read long after the code was written.

### At each commit

Two or three sentences in chat, in plain language: what this code does, and why
it is shaped this way. Name the alternative you did not take when there was
one. Skip the jargon or define it in passing — assume no prior knowledge of
C++ project architecture, and none of SFML or sockets beyond what this project
has already covered.

### In `docs/NOTES.md`

Add an entry per piece of work, in the format that file specifies: what it
does, why it is built that way, and the tricky part. Write it at the time, not
at the end of a phase. Keep entries short — this file gets revised from, and
nobody rereads ten pages.

Do not duplicate `docs/DESIGN.md`: that holds the architecture decisions and
their rationale. `NOTES.md` explains how the code actually works.

### At each phase boundary — write to `docs/REVISION.md`

**Do not stop to quiz the student.** Instead, at the end of every phase append
four to six question-and-answer entries to `docs/REVISION.md`, in that file's
format, covering what the phase built.

Write real questions — the ones an examiner would actually ask about that
phase, not trivia. Favour "why is it built this way" over "what is the name
of". Answers short enough to memorise, specific enough to say out loud.

This file is what the student revises from, and it may be read long after the
code was written. Write every answer so it stands alone and assumes nothing
about what they remember.

Notes and explanations in English by default; switch to Hebrew if the student
asks. Identifiers and code comments stay English either way.

## Requirements summary

A rough orientation only — **not authoritative and not a checklist.** Verify
against `docs/REQUIREMENTS.md`, and against `docs/requirements.pdf` when exact
wording matters.

- Two separate programs; each must compile and run independently.
- Client menu: Local Game, Player vs Bot, Online Game, Exit.
- Local Game and Player vs Bot: created, managed and validated on the Client.
- Online Game: created on the Server, which owns board, turn and validation.
  The Client only renders server state and sends requests — it never applies
  an online move locally or treats its own request as final.
- Matchmaking pairs two waiting Clients; the Server supports many concurrent
  Clients and independent concurrent games.
- Full chess rules: legal moves, blocking, turn order, check, checkmate,
  stalemate, castling, en passant, promotion, resign, draw by agreement.
- Bot: at least three clearly distinguishable difficulty levels via Stockfish.
- Networking via SFML Network. Protocol format is ours to design.
- Server prints meaningful status messages to the console.
- Disconnections must never crash the Server or affect other games.

## Leave no traces in shippable files

Everything that ships is the student's work and must read that way. In any
source file, the README, or a commit message:

- **Never** mention Claude, an AI, an assistant, a model, or a tool, and never
  write "generated", "auto-generated" or a co-author trailer.
- No commented-out code, no dead code, no debug `cout` left behind, and no
  `// TODO` / `// FIXME` / `// HACK`.
- No placeholder or scratch files. Delete `client/.gitkeep` and
  `server/.gitkeep` once those folders hold real sources — they exist only so
  git tracked the empty directories.
- No file that nothing uses. If a header or source stops being needed, delete
  it rather than leaving it in the tree.
- Comments explain the chess or the protocol, never the process of writing the
  code. `// pawns may capture en passant only immediately after the double
  step` is useful; `// added this to fix the build` is noise.

This is about the submitted artifact, not secrecy — `CLAUDE.md`, `docs/` and the
git history stay as they are, and they are excluded from the zip.

## Submission

- Zip named `<ID>.zip` containing all source, `ProvidedGUI/`, `client.exe`,
  `server.exe`, the Stockfish binary, the required DLLs, and a README that
  lets the grader compile and run every mode without guessing.
- Exclude `.vscode/`, `docs/`, `.git/` and this file from the submission zip.
  They are tracked in git for backup, so removing them is a zip-time step —
  never delete them from the repo.

## Code style

Derived from a sample of the student's own code. Keep the naming instinct,
normalise the formatting.

### Naming

- Identifiers in English. `PascalCase` for types, `camelCase` for functions,
  methods and locals, trailing underscore for private members — matching
  `ProvidedGUI` so the whole submission reads consistently.
- **Descriptive names over short ones.** The student writes
  `close_candle_left`, not `ccl` or `tmp`. Preserve that: `nearestRookLeft`,
  `piecesBetween`, `isSquareAttacked` — never `n`, `p`, `flag`, `data`, `temp`.
  Loop counters may be `i` / `row` / `col`.
- Booleans read as questions: `isInCheck`, `hasCastled`, `canPromote`.
- Constants in `ALL_CAPS`, matching `ProvidedGUI`'s own
  `static constexpr int BOARD_N = 8`.

### Formatting

- 4 spaces, no tabs.
- Opening brace on the same line: `if (isCheck) {`
- **Braces always**, even for single-statement bodies. The sample omits them
  on some `else` branches; do not reproduce that.
- Spaces around binary operators and after semicolons in `for`:
  `for (int i = 1; i < size; ++i)` — not `for(int i=1;i<size;i++)`.
- Space after `if`, `for`, `while`, `switch`.
- Soft line limit ~100 columns.
- One blank line between logical blocks inside a function; two between
  function definitions.

### Readability — the main thing to fix

The student's code is correct but gets dense. Avoid nested subscripting and
compound conditions; name the intermediate value instead.

Avoid:

```cpp
if (a[b[q[i][1]]] == -1 || a[b[q[i][0]]] < c[q[i][0]]) { res[i] = 0; }
```

Prefer:

```cpp
const int rangeStart = queries[i].start;
const int rangeEnd   = queries[i].end;

const int leftRook  = nearestRookLeft[rangeEnd];
const int rightRook = nearestRookRight[rangeStart];

if (leftRook == NONE || rightRook == NONE) {
    results[i] = 0;
    return;
}
```

- Extract a compound condition into a named `const bool` or a small
  predicate function.
- One responsibility per function; if it needs a comment to explain *what* it
  does, split it.
- Prefer early return over deep nesting.

### Comments

- Comment *why*, not *what*. `// pawn can capture en passant only on the move
  immediately after the double step` is useful; `// loop over rows` is noise.
- Every non-obvious public method gets a one-line comment in the header
  saying what it guarantees.
- Either English or Hebrew is fine, but be consistent across the project, and
  keep files UTF-8.

### C++ specifics the requirements grade (§71-80)

- Separate `.h` and `.cpp`. Declarations in headers, definitions in sources.
  No logic in headers beyond trivial inline accessors.
- `const` on every method that does not mutate, and on parameters passed by
  reference that are not modified: `bool isLegal(const Move& move) const;`
- Pass non-trivial types by `const&`, not by value.
- Members private by default; expose behaviour, not raw data.
- Smart pointers (`unique_ptr`, and `shared_ptr` only where ownership is
  genuinely shared) where ownership is dynamic. No raw `new` / `delete`.
- RAII for sockets, files and the Stockfish process — acquire in the
  constructor, release in the destructor.
- Prefer STL containers and algorithms over hand-rolled loops where it does
  not hurt clarity.
- `enum class`, not plain `enum`.
- Initialise members in the declaration or the constructor's init list; never
  leave them indeterminate.
