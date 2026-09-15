# Operating Claude Code on this project

## 1. Install it

Claude Code is separate from VS Code and from GitHub Desktop — having those
does not mean you have it.

1. In VS Code press `Ctrl+Shift+X`, search **Claude Code**, install the one
   published by **Anthropic**.
2. Open it: the **spark icon** in the top-right of the editor (only shows when
   a file is open), or the spark icon in the left Activity Bar, or
   `Ctrl+Shift+P` → type "Claude Code".
3. First open shows a sign-in screen. Sign in with your Claude account — the
   same one you're using now. No API key needed.
4. If the extension doesn't appear after installing: `Ctrl+Shift+P` →
   **Developer: Reload Window**.

Requires VS Code 1.94 or newer. The extension has its own copy of the engine
built in, so you don't need anything else installed.

## 2. Set the permission mode

At the bottom of the prompt box there's a mode indicator. Click it and pick
**Edit automatically** (or leave it on **Auto**, the default).

That's what makes it work without stopping to ask you about every file. If you
leave it on **Manual** it will ask permission for each edit, which is the
opposite of what you want here.

**Plan** mode is worth knowing about for later: it describes what it intends to
do and waits. Useful if you want to see the approach before a big phase.

## 3. The prompts you actually need

**Starting for the first time:**

```
Read CLAUDE.md and docs/PLAN.md, then start Phase 1, one item at a time.
```

**Every time after that** — new session, next day, after usage runs out:

```
Read CLAUDE.md and docs/STATUS.md, then continue.
```

That's the only one you need to remember. It reorients from the files.

**When something is broken** — say what you saw, not that it's broken:

```
I clicked e2 then e4 and nothing happened. The window stayed on White to move.
```

Specific beats vague by about an hour of guessing. Paste error text verbatim.

**Before you stop for the day:**

```
Update docs/STATUS.md and commit anything that builds.
```

## 4. Things only you can do

It writes the code. It cannot see your screen or play chess. These are yours:

- **Run the GUI and look at it.** Every phase that touches the board needs you
  to click pieces and say what happened.
- **Bot difficulty (§20)** must be *demonstrably* different between levels.
  That means you playing easy and hard and confirming they aren't the same.
- **Two clients at once (§25, §57-60)** — start the server, run `client.exe`
  twice, play a game against yourself. The requirement is that it works
  between two different machines, so test that too if you can.
- **Disconnections (§64-66)** — close one client mid-game with the window X and
  report what the other one did.

## 5. When it goes wrong

| Symptom | What to do |
|---|---|
| Build fails | Paste the whole error. Don't summarise it. |
| It changed something in `ProvidedGUI/` | Tell it to revert — those files are submitted unmodified. `git checkout ProvidedGUI/` undoes it. |
| It's drifting from the design | Point at `docs/DESIGN.md`. That file is the authority; it's allowed to change a decision but must rewrite the file and say why. |
| Usage runs out mid-task | Nothing breaks. Come back and use the resume prompt. `main` always builds. |
| Context indicator near full | Type `/compact`, or just start a fresh session and resume — the files carry the state. |
| You want to know your usage | Type `/usage`. |

## 6. GitHub (optional)

Nothing needs GitHub. Git already works locally — three commits so far. But an
off-machine backup is cheap insurance for a project this size.

You have GitHub Desktop, which handles the login so nobody has to type
credentials:

1. GitHub Desktop → **File → Add local repository** →
   `C:\Users\Owner\source\chess`
2. **Publish repository**
3. **Keep this code private.** Leave that box checked. A public repo of a
   graded project is a plagiarism problem waiting to happen — both for you and
   for whoever copies it.

`CLAUDE.md` is gitignored, so it never reaches GitHub. `docs/` is tracked
deliberately, so your design rationale and revision notes are backed up.

Once a remote exists, Claude Code can push after each phase — ask it to.

## 7. Don't

- Don't let it touch `ProvidedGUI/`, `Example/` or `build_example.bat`.
- Don't skip ahead in `docs/PLAN.md`. Phase 4 needs 1-3 actually working, and
  debugging a server against a broken rules engine is miserable.
- Don't accept "it should work" for anything on the checklist. Run it.
- Don't let it commit build output. `*.exe`, `*.dll`, `stockfish*` are ignored
  for a reason.
