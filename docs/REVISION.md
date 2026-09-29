# Revision sheet

Questions and answers about this project, written as it was built. This is the
file to read before the test — not the code, and not `NOTES.md`.

Each entry is a question someone could ask about the project, with the answer.
Questions are added at the end of every phase, covering what that phase built.
Answers are short enough to hold in your head and specific enough to say out
loud.

Read it by covering the answers first and trying to answer from memory. The
reason the answers are written down is so you can check yourself, not so you
can read them passively — recognising an answer and being able to produce one
are different things, and only the second survives being asked.

---

## Setup and toolchain

**Why MSYS2 / MinGW and not Visual Studio?**
The requirements pin it: §85 says MSYS2 UCRT64 / MinGW, and §88-89 give the
exact `g++` command lines the README has to document. Visual Studio uses MSVC,
a different compiler, and its linker cannot read the MinGW-format SFML
libraries that pacman installs. The lecturer's own `build_example.bat` also
hardcodes `C:\msys64\ucrt64\bin` and calls `g++`.

**What does the provided GUI do, and what does it not do?**
It draws the board and pieces, shows player names, turn and status text, and
reports clicks. It does *not* know the rules of chess, does not validate moves,
does not manage games, does not do networking, and does not decide who won
(§2). It reports intent — `MoveRequested`, `ResignRequested` — and our code
decides what that means.

**Your submission has no DLLs. How do the programs find SFML?**
The executables link dynamically against SFML and the MinGW runtime, and Windows
looks for those DLLs next to the executable and on `PATH`. The lecturer asked for
no DLLs in the submission, so they come from `PATH`: the MSYS2 UCRT64 shell, the
environment §85 names, puts `C:\msys64\ucrt64\bin`, where pacman installed SFML,
on `PATH`. That is why the README runs everything from that shell. Started any
other way, the program dies at launch with "libsfml-graphics-3.dll was not
found". Static linking would remove the dependency, but it would change the
build commands §88-89 give, so the build stayed dynamic.

**Why is the chess rules code in `common/` rather than in the client?**
Because it compiles into both programs. The Client validates Local and Bot
games (§17-18), the Server validates Online games (§22), and §6 says the rules
are identical in every mode. One copy in `common/`, built into both binaries,
is the only way to guarantee that without maintaining two implementations.

---

## Phase 1 — the chess rules

**How is the board stored, and why that way?**
One flat array of 64 `Piece` values, indexed 0 = a8 up to 63 = h1. Two reasons.
That is exactly the indexing the provided GUI's `setBoard` uses, so displaying
the board needs no conversion and cannot come out flipped. And because a
`Piece` is a plain value — a piece type and a colour — copying an entire board
is copying 64 small structs. That matters because deciding whether a move is
legal is done by copying the board and playing the move on the copy, which
happens for every candidate move. Had each square held a `unique_ptr<Piece>`,
every one of those copies would be a deep clone of up to 32 heap objects.

**How do you know a move does not leave your own king in check (§31)?**
Copy the board, play the move on the copy, then ask whether your own king is
attacked in the resulting position. If it is, the move is not legal.

The reason this is worth doing rather than tracking pins directly is that one
test covers every case without any of them being written down: pinned pieces,
the king walking into check, capturing the piece that is giving check, blocking
a check, and the en passant capture that removes two pawns from a rank and
exposes the king along it. All of them are the same question asked about the
position that results.

**Where is the polymorphism, and why there?**
Two places. `PieceRules` is an abstract base with a subclass per piece type,
each knowing how its own piece moves — so fixing a bug in how bishops move
touches one small class instead of one arm of a six-way switch. And the game
modes (Phase 2) will be a `Game` base with `LocalGame`, `BotGame` and
`OnlineGame`, because the three differ in exactly one thing: who decides a
move.

The piece rules objects are **stateless** — they hold no square and no colour,
they answer questions about a board handed to them, and one shared instance of
each exists. That is what lets the board stay a plain array of values:
behaviour is polymorphic, data is not.

Bot difficulty is deliberately *not* a class hierarchy. The three levels differ
only in the numbers passed to Stockfish, not in behaviour, so classes there
would be decoration. §73 asks for polymorphism where it suits the design.

**Why is there a separate `attacksSquare` instead of just checking whether a
piece has a move to that square?**
Because for a pawn those are different squares. A pawn moves straight ahead but
captures diagonally. Defining "attacks" as "has a move there" would say a pawn
attacks the empty square in front of it — which it does not — and that it does
not attack the diagonals, which it does. Check detection is built on
`attacksSquare`, so getting this wrong would give wrong answers about check in
exactly the positions where it matters.

**What is pseudo-legal, and why generate moves that are not legal?**
A pseudo-legal move is one that obeys how the piece moves but has not been
checked for leaving your own king exposed. Move generation produces those, and
a second pass filters them.

They are separate because otherwise generation would have to call itself.
Deciding "does this move leave me in check?" means asking whether the *enemy*
could then capture my king — which is a question about enemy moves. If move
generation filtered for check, it would need move generation to answer.
Splitting it in two breaks the loop.

**Why is castling generated in `Rules` rather than with the king's other
moves?**
Two of castling's conditions are about attacked squares: you may not castle out
of check, and the king may not cross a square the enemy attacks. Answering that
needs `isSquareAttacked`, which lives in `Rules`. If `KingRules` called into
`Rules`, the two would depend on each other in a circle. Generating it one
level up keeps the dependency pointing one way — and is the honest place for
it, since castling is a fact about a king, a rook and the check state together,
rather than about how a king moves.

**What are castling's conditions, and which one is easy to get wrong?**
The right must still exist (neither the king nor that rook has moved, and that
rook has not been captured on its corner), the squares between king and rook
must be empty, the king must not be in check, must not cross an attacked
square, and must not land on one.

The trap is treating "must be empty" and "must not be attacked" as one list.
Castling queen's side needs b1, c1 and d1 empty because the rook crosses all
three — but the king only goes e1 to d1 to c1, so b1 may be attacked and the
castle is still legal.

**What makes en passant harder than it looks?**
Two things. The right lasts exactly one move, which is handled by clearing the
target square at the start of every move so it can only survive the one that
set it. And it is the only move in chess that removes two pieces from the same
rank at once — the capturing pawn leaves it and the captured pawn disappears
from it — so it can expose your own king along that rank to a rook or queen in
a way no other capture can. Our engine gets that right without any code for it,
because legality is decided by looking at the position that results.

It is also the only capture that lands on an empty square: the pawn being taken
is beside the capturer, not on the square it moves to.

**How is the rules engine tested? Why trust it?**
With **perft**. Perft plays out every legal move to a given depth and counts
the positions reached. Published counts exist for standard positions and every
chess engine in the world agrees on them, so matching is evidence rather than
optimism — it is not "the cases I thought to test pass".

All 26 cases pass, about 16.5 million positions over the six standard positions,
which were chosen by engine authors precisely because they are nasty: pins,
discovered check, castling rights lost to a rook capture, under-promotion, and
the en passant capture that is illegal because of the rank it clears. From the
starting position, depth 5 is 4,865,609. When a count disagrees, the harness's
`divide` mode prints the count under each individual move, which narrows the
bug to one move and then one position.

**A game has ended. What does the GUI get told, and where does that come
from?**
Two things, because §39 asks for both: the result and the reason.
`ChessGame::resultTitle()` gives `"White wins!"`, `"Black wins!"` or `"Draw"`,
and `resultDetail()` gives `"Checkmate"`, `"Stalemate"`, `"White resigned"` or
`"Draw agreed"` — exactly the two strings `showGameOver` takes. One without the
other is not enough: "Draw" does not say whether it was agreed or forced.

They are produced in `common/` rather than in the Client so that a Local game,
a Bot game and a game running on the Server all word the same outcome the same
way.

**Why are the result and reason stored rather than worked out on demand?**
Because two of the four reasons are events, not positions — you cannot look at
a board and see that a player resigned or that both agreed a draw. Storing them
also makes the outcome permanent: once a game is over nothing can overwrite the
result, and `makeMove` refuses every move from then on.

**When a checkmate happens, how do you know who won?**
The side *to move* is the side that has been mated, so the winner is the other
one. The check runs after the turn has already changed hands, so the player who
just moved is the winner. Getting this backwards reports every mate as a win
for the wrong colour, and it is easy to miss, because the game does end
correctly — only the name on it is wrong.

---

## Phase 2 — the Client, the connection and the Local Game

**Why was the Server written before the Client's menu, when the menu has
nothing to do with the Server?**
Because §8 says the main menu is reachable only after a successful connection.
That makes the connection a gate in front of everything else in the Client, so
with no server to connect to, none of the Client could be reached to be tested
— not even Local Game, which never speaks to the Server at all. The server
built at that point does only what §11 needs: listen, accept, and print where
it is listening.

**What does the Server print when it starts, and why two addresses?**
The port, and two addresses: `127.0.0.1` for a Client on the same machine, and
the machine's LAN address for a Client on another. §11 requires the Server to
report where it is listening and §25 requires play between two machines on one
network — and a player cannot type an address nobody has told them.

**How does the connection screen handle a mistake, and what are the three
kinds?** §9 wants a clear message and §10 wants a way forward, so after any
failure the screen offers: try again, change the address, change the port, or
exit. The three failures are caught in three different places:

- a bad address is caught the moment it is typed, by trying to resolve it
  there — otherwise a typo looks exactly like the server being switched off;
- a bad port is caught by the number reader, which refuses letters, blanks,
  out-of-range values and trailing rubbish;
- nothing answering is caught by the attempt itself, and the message lists
  what to check.

**What is the trap with reading a number from the console?**
`std::stoi("53000abc")` returns 53000 and silently ignores the rest, so a typo
passes as a valid port. The reader checks that the number consumed the *whole*
line.

There is a second trap: bad input and the input stream *ending* have to be
told apart. Bad input should be asked again; an ended stream must not be, or
the program spins at full processor asking a stream that can never answer.
That happens for real when the console window is closed.

**Something subtle: why is typing the port into the IP box a problem?**
Because it does not look like an error. Winsock reads a bare number as a packed
32-bit address, so resolving "53000" succeeds and gives 0.0.207.8. The player is
then told that address did not answer — true, and useless. It is the easiest
mistake to make on that screen, so an all-digits address is refused by name.

**In the Local Game, how does a click on the board become a move?**
The GUI reports a click as a row and a column, with row 0 meaning rank 8 and
column 0 meaning file a. The engine numbers its 64 squares the same way, so the
conversion is `row * 8 + col` and nothing more. That was the reason for adopting
the GUI's indexing in the first place: there is no conversion layer to get
wrong, and no way for the board to come out flipped.

**What happens when a player tries an illegal move?**
Nothing — and nothing needs to. `makeMove` checks the move first and changes
nothing at all unless it was legal, so the board is redrawn from a position
that never changed and the piece simply appears not to have moved. There is no
"undo" step, because nothing was ever done.

**Why is resigning two steps rather than one?**
§54-55. The GUI reports only that the Resign button was clicked — it decides
nothing. Our code responds by showing the confirmation dialog, and the player
does not actually resign until `ResignConfirmed` arrives. `ResignCancelled`
does nothing at all and the game carries on.

**Why does the Local Game's draw dialog name both colours?**
Because both players are at the same screen. In an online game the offer
appears on the opponent's own machine and there is no doubt who is being asked;
here, "Opponent offers a draw" would not say which of the two people sitting
there should answer. So it reads "White offers a draw. Black, do you accept?"
(§49).

**Why is there a flag for having shown the game-over summary?**
Because the summary's close button hides the overlay and reports no event at
all. If the result were shown whenever the game is over, it would be put back
on the very next frame and could never be dismissed. Showing it once, then
disabling board input and hiding the Offer Draw button, leaves the final
position readable but untouchable.

---

## Phase 3 — the Bot

**How does your program talk to Stockfish?**
Stockfish is a separate program, not a library. We start it with
`CreateProcess`, having first made two pipes — one to write its standard input,
one to read its standard output — and then speak **UCI**, the plain-text
protocol chess engines use. We send lines like `position fen ...` and `go
movetime 500`; it replies with lines, ending in `bestmove e2e4`. Nothing is
shared but those two pipes, which is what makes it genuinely a separate process
as §46 requires.

**Stockfish is far stronger than your code. What stops it running the game?**
It only ever returns a *string*. `chooseMove` hands back something like
"e2e4"; `BotGame` converts that to a `Move` and puts it through
`ChessGame::makeMove` — the identical call a human's mouse click makes. If the
suggestion were illegal it would be refused by the ordinary legality check, with
no special case for it. §47 requires exactly this split: Stockfish selects the
Bot's move, our code owns validation and game state.

It was tested by playing 120 engine half-moves through our own rules. Every one
was accepted, which also cross-checks our move generator against Stockfish's.

**How do the three difficulty levels differ, and why not make them classes?**
Two numbers: Stockfish's own `Skill Level` (0-20) and how long it may think.
`Skill Level` is the one that matters — below 20 Stockfish deliberately plays
moves it has judged inferior, so a low setting *blunders* like a weak human.
Just giving it less time would not do that; a full-strength engine with a tenth
of a second still plays far above club level.

They are not classes because they differ only in values, not behaviour — a
class per level would be decoration. §73 asks for polymorphism where it suits
the design, and here it does not.

**How do you know the levels are actually different, as §20 demands?**
They were made to play each other, by a test that is kept in the repo:
`tests/bot_match.cpp`. It starts two Stockfish processes at different settings
and plays a full game, then repeats with the colours swapped — six games, every
pairing from both sides. The stronger level won all six by checkmate, with final
material as lopsided as 33 points against 9. That is a measurement, not an
assumption, and swapping colours matters because otherwise a win could just be
the advantage of moving first.

The test calls the same `settingsFor()` the Bot uses rather than repeating the
numbers, so if someone changed a difficulty constant the test would re-check it.
A test carrying its own copy of the values would keep passing after the real
ones changed — worse than having no test.

**You can't play chess well. How can you claim the hard level is hard?**
That is precisely why it is measured rather than played. A human impression of
engine strength is an anecdote, and one from a weak player is not even that —
every level beats me, so I cannot tell them apart by feel. Making the levels
play *each other* removes me from the judgement entirely: the question becomes
"which setting checkmates the other", which has an objective answer.

I did first try to find a way to see it by hand — a fixed four-move attack
(Scholar's Mate) that I could click without understanding it, hoping Easy would
fall for it and Hard would not. Over eight games per level, every level defended
it and I lost my queen every time, so it separated nothing. The lesson is that
avoiding one specific well-known trap is not a measure of strength; it is the
kind of thing even a deliberately weakened engine does by accident.

**Does that test check anything besides the difficulty ordering?**
Yes — every move either engine produces is played through `ChessGame::makeMove`,
and a refusal ends the game as a reported failure rather than being worked
around. So a run is several hundred more half-moves of our move generator being
cross-checked against a real engine, and it is §47 restated as a test: the
engine proposes, our code disposes.

**Something subtle bit you when testing the engine. What?**
An engine reading from a pipe treats **end-of-file as `quit`**. Feeding
Stockfish a file of commands, it kept answering `bestmove a1b1` in a position
that is mate in one, reporting `nodes 1` — it was not searching at all. On
reaching the end of the file it took that as `quit` and abandoned the search
immediately. The fix in the real code: our end of the pipe stays open for as
long as the engine is wanted, and nothing is sent after `go` until `bestmove`
comes back.

**Why poll the pipe instead of just reading from it?**
A plain `ReadFile` on a pipe blocks until something arrives. If the engine died
mid-search, that would hang the whole board window, with no way even to close
it. `PeekNamedPipe` asks whether anything is waiting first, so every read has a
deadline and a silent engine becomes a reported error rather than a freeze.

**Where does the Bot's window freeze, and what did you do about it?**
The engine blocks while searching — up to a second on Hard. So the loop renders
the human's move *before* asking the Bot, and shows "Bot is thinking..." with
board input disabled while it waits. Done the other way round, the player's own
piece would not appear to move until the Bot replied, and the program would
look like it had frozen after every move.

**You now have three game modes sharing one loop. How is that arranged?**
A `Game` base class holds everything common: reading clicks, turning them into
moves, the promotion dialog, resign confirmation, draw offers, and showing the
result. `play()` is deliberately **not** virtual, so a mode cannot reimplement
the parts §6 requires to behave identically. Subclasses fill in four small
hooks — the player names, how a non-human opponent takes its turn, whose Resign
button it is, and what a draw offer does. `LocalGame` is about 30 lines.

The base class was written in Phase 3, not Phase 2. With only `LocalGame` in
existence the seams would have been a guess; `BotGame` provided a second real
case to generalise from.

**Where is the only Windows-specific code in the project, and why is it
contained?**
`client/StockfishEngine.cpp` — process creation and pipes. The handles are
declared `void*` in the header, so `<windows.h>` never leaks into anything that
includes it. Everything else in the project is portable C++ and SFML.

**A program that builds and links can still fail to start. How?**
A missing DLL. `server.exe` built cleanly and then died instantly with no
message at all, exit code `0xC0000135` — Windows kills a process for a missing
DLL *before* `main` runs, so there is nothing on the console and the symptom
looks like a crash in code that has not executed. The cause was that the
project's runtime DLLs had been collected from the GUI example, which does no
networking, so `libsfml-network-3.dll` was never among them. The submission now
ships no DLLs at all, so the same failure is what anyone sees who starts the
programs outside the MSYS2 UCRT64 shell.


## Phase 4 — the Server and online play

**In an online game, what stops the Client cheating?**
It has nothing to cheat with. `OnlineGame` has no `ChessGame` in it at all —
no board it can edit, no move generator, no `makeMove` to call. All it holds is
the last 64 characters the Server sent, which it replaces wholesale and never
edits. A click sends a `MoveRequest` and changes nothing on screen; the piece
moves only when a position in which it has moved comes back from the Server.
That is §24 turned into something the compiler enforces rather than something
the programmer remembers.

**You had to change the class hierarchy to achieve that. What was wrong with it?**
Until Phase 4 the `Game` base class owned a `ChessGame`, because the only two
modes then written — Local and Bot — both wanted one. But that meant any new
subclass inherited `makeMove`, and `OnlineGame` is required never to use it.
Inheriting a capability you are forbidden to exercise is a trap, not a design.
So a middle class was added: `ClientValidatedGame` holds the rules engine and is
inherited by `LocalGame` and `BotGame` (§17-18 put validation on the Client for
those), while `OnlineGame` inherits the bare `Game` and gets no engine.
`Game` itself kept only the board window and the shape of an interaction.

**Why build the protocol on `sf::Packet` rather than sending text?**
TCP is a stream of bytes with no message boundaries. Two messages sent quickly
can arrive stuck together, and one message can arrive in halves. Anything
hand-written therefore has to buffer partial reads and work out where each
message ends — a well-known source of bugs, and networking is not what this
project is being marked on. `sf::Packet` does that framing itself and packs
integers and strings with no parsing code.

**How do you stop one game's messages reaching another game's players (§59)?**
Every in-game message carries its game id, and on the Server every handler
starts by calling one function: `findGameForPlayer(clientId, gameId)`. It
returns the game only if that client is one of its two players, so a client
asking about somebody else's game is refused in one place rather than at each
of a dozen sends. A client sending a move for a game it is not in gets a log
line and no reply — which is tested.

**Why does the Server run on one thread instead of one per client?**
Because a thread per client means shared game state across threads, which means
locks, and §64 says one client disconnecting must never bring the Server down.
A single thread with `sf::SocketSelector` has no shared state, so an entire
family of crash cannot happen. `wait()` sleeps until some socket has something
to say, so an idle server with many clients still uses no processor time.

**Something about a disconnected socket can spin the Server at 100% CPU. What?**
A closed socket reports itself *ready* to the selector forever. If a client that
has gone is removed from the list but left in the selector, `wait()` returns
immediately every time, the loop finds nothing to read, and it spins. So a
disconnecting client has to leave the selector as well as the list. The order
also matters: the connection is dropped first and only then is its game ended,
otherwise the Server tries to send the result to the player who just vanished.

**A player disconnects mid-game. What does the other one see, and why not just
"resigned"?**
The Server ends that game and sends the remaining player a `GameOver` saying,
for example, "Black wins!" and "White disconnected". It is deliberately not
worded as a resignation, because telling somebody their opponent resigned when
in fact the network dropped would be false. That meant adding
`GameEndReason::Disconnection` to `ChessGame` alongside resignation and
agreed draws.

**How can a player cancel a search for an opponent, given the console is not
watching the keyboard?**
The board window opens straight away, before there is an opponent, showing
"Waiting for opponent". Closing it sends `CancelFindGame`. Waiting at the
console instead would mean watching the keyboard while blocked on a socket,
which needs platform-specific console code; this reuses the loop that already
exists. The same mechanism covers leaving a live game — closing the window then
sends `Resign`, because walking away from a game in progress concedes it rather
than leaving the opponent waiting forever.

**Most chess programs turn the board round for the player with Black. Why
doesn't yours?**
Two reasons, and neither is laziness. It is not asked for: the requirements
document says nothing about orientation anywhere — it lists the chess rules and
says where the position is stored, never which way up it is drawn. And the
provided GUI cannot do it. `ChessFrontend` draws its own coordinate labels from
the row and column it is handed — files as `'a' + col`, ranks as `8 - row` — and
has no orientation setting. Reversing the 64 characters before passing them in
would move the pieces but leave the labels alone, giving a board with Black at
the bottom still labelled rank 8 at the top. That is worse than not flipping,
and correcting it would mean editing `ProvidedGUI/`, which the project may never
do. The lecturer's own component is built assuming an unflipped board.

**Why are the move-rejection messages so short?**
Because the GUI's side panel is 260 pixels wide and neither wraps nor shortens
what it is given — a long string simply runs off the edge of the window, which
is what "Rejected: that move is not legal" did. The reasons are now "Illegal
move", "Not your turn", "Game already over" and "Not your game", each measured
against that width in the GUI's own font and size rather than guessed at. They
are worded on the Server, so they read correctly in its log too.

**Your client is sitting on the main menu and the server dies. How does it find
out?**
By asking, because it cannot be told. A TCP connection that has gone is only
discovered by trying to use it — the "connected" flag is just the result of the
last read or write, so it stays true indefinitely after a server quietly
disappears. Nothing on the menu talks to the server, so `isServerStillThere`
does one non-blocking read before the menu is shown and again if Online is
chosen. The loss is reported once and the program carries on: Local and Bot
games never needed the server, and quitting would remove two working modes to
punish the loss of a third.

**Is there anything a hostile client could still do to your server?**
Yes, and it is a deliberate trade-off rather than an oversight. The server's
sends are blocking, so a client that stopped reading but kept its socket open
could stall the single thread. Fixing it properly means an output queue per
client and non-blocking writes. What §64 actually asks — that a client
*disconnecting* never crashes the server or disturbs other games — is met, since
a dropped socket fails the send immediately instead of blocking, and that is
tested. The simpler design was chosen knowingly.

**§67 talks about invalid messages. Is that just checking the format?**
No, there are two separate problems. A message can be malformed — an unknown
type byte, a missing or truncated body — which is caught by `isKnownMessageType`
and by checking that every field actually read. But a message can also be
perfectly well formed and still be wrong for the moment it arrived in: a move
for a game you are not in, an answer to a draw offer nobody made, answering your
own offer, a second offer while one is outstanding, asking for a new game while
already playing. Those need the current state to detect, and both ends check
them.

**How was any of this tested without two people and two computers?**
With a program that connects as several clients at once and checks the Server's
answers — 103 checks in all. It covers matchmaking and the waiting state, moves
played out of turn and moves that are illegal, two games running at once with no
cross-talk, a client aiming a move at a game it is not in, draw offers accepted
and declined, resignation, a client disconnecting mid-game, a game played to
checkmate, promotion including to a knight, four kinds of malformed message and
then 1,600 random ones, after which the Server is checked to still be pairing
new clients. The console prompts were driven separately with deliberate rubbish at
every one. What it cannot check is §25, two machines on one network; that needs
a second computer.


## Phase 6 — the quality pass

**How do you know there are no memory leaks?**
It was measured rather than argued. The easy answer is that there is no `new`,
`delete`, `malloc` or `free` anywhere in the project — the only owning pointer
is a `unique_ptr<sf::TcpSocket>` and everything else is a value or an STL
container — but that is reasoning, not evidence. AddressSanitizer is not built
into this MinGW toolchain, so the real binary was watched instead: one server
process, six complete runs of the protocol test through it, about seventy
connections and two dozen games. Memory finished flat at 1,048 KB and the
handle count never moved off 94. The handle count is the telling one, because
that is what would climb if a socket were ever left unclosed.

**Where is RAII in this project, and what would go wrong without it?**
Three places. `ServerConnection` closes its socket in its destructor;
`StockfishEngine` starts the engine process in its constructor and shuts it down
in its destructor; the Server holds each client's socket in a `unique_ptr`.
Without it, every path out of a function would have to remember to clean up —
including the ones that throw — and the failure is invisible at first: sockets
and a 103 MB engine process quietly accumulating. Both classes are also
non-copyable, because two objects each believing they own the same socket or the
same process would try to close it twice.

**Why is `gui_` protected rather than private? Isn't that against encapsulation?**
It is a deliberate choice. The usual repair is a protected accessor returning a
reference to the same member — but that hands a subclass exactly the same power
while merely looking more encapsulated, so it buys the appearance rather than
the fact. Subclasses genuinely need the GUI: they set player names, hide the
Offer Draw button, show dialogs and render. What §72 really turns on is that it
is protected rather than public, so nothing outside the hierarchy can touch it.

**You have two functions taking `sf::Packet` by value. Why not `const&`?**
Because SFML's own `send` takes a *non-const* reference and reads the packet as
it goes, so it cannot take a `const&`; and every caller builds its message
inline and passes the temporary, which a plain reference cannot bind to. Taking
it by value gives each recipient its own copy, which is exactly what sending the
same message to two players needs. It is the one place in the project where
by-value is right, and both say so in a comment.

**§25 asks that online play work between two computers. Did you test that?**
The requirement is subtler than it first reads. Its wording asks that the
*implementation* not assume every Client runs on one computer, and it then says
explicitly that during checking it is also acceptable to test with several
Clients on the same machine. So what has to be true is a property of the code,
and it is provable: `netstat` shows the Server listening on `0.0.0.0` rather
than `127.0.0.1`, meaning it accepts connections on every network interface, and
the source says `listen(port, sf::IpAddress::Any)` rather than leaving that to a
default. The Server also prints its LAN address for the purpose, and `127.0.0.1`
appears nowhere in the code except in help text. A server bound to the loopback
address would pass every test done on one machine and fail on two, which is
exactly the trap the requirement is set to catch.

**How do you know your build commands are the ones the project is graded on?**
They were taken from the requirements document and compared flag by flag. It
gives them as templates with the source files left blank — `g++ -std=c++17 ...
-o client.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network`, and
the server one with just `-lsfml-network -lsfml-system`. Ours match exactly,
including the order of the link flags, and the `-IProvidedGUI` we add sits where
the lecturer's own `build_example.bat` puts it. The README gives each as a
single line rather than using backslash continuations, because those work in the
MSYS2 shell but not in `cmd` or PowerShell, and both were pasted into both
shells to confirm.

**Your crafted bad-input tests only cover what you thought of. What about the
rest?**
That is why the protocol test ends with fuzzing: 1,600 random packets from four
Clients at once, half of them beginning with a plausible type byte so the
per-message parsers are actually reached and fed nonsense rather than being
turned away at the first check, and two of those Clients dropped mid-stream so
the Server also has to handle a disconnection with unread rubbish still queued.
Afterwards it is checked to still pair new Clients and still validate a real
move, with its memory and handle count unchanged. The seed is fixed, so a
failure could be reproduced exactly.

**Did the quality pass actually change anything, or just confirm things?**
It changed four things. Two methods were found to be declared, defined and
called from nowhere — `HostedGame::hasPendingDrawOffer`, and
`ChessGame::legalMovesFrom`, which had been written to highlight a piece's legal
moves before it turned out the provided GUI does that itself. Both were deleted,
because an unused public method is still code that has to be explained. One
method that changed nothing was made `const`. And a warning I had introduced in
all three test harnesses — a line-continuation backslash at the end of a `//`
comment, which `-Wcomment` objects to — was fixed.


## Phase 7 — the submission and the final audit

**What is in your submission zip, and what did you deliberately leave out?**
What §82 asks for: the source of both programs (`common/`, `client/`,
`server/`), `ProvidedGUI/` unchanged, `client.exe`, `server.exe` and the
Stockfish binary, plus the README (§83). Also the three test programs, because
§20 asks for the Bot levels to be *demonstrable* and the Bot match is the
demonstration. Left out: DLLs, because the lecturer asked for none (the programs
run from the UCRT64 shell, which supplies them); and the lecturer's `Example/`,
`build_example.bat` and `GUI_USAGE.md`. Nothing asks for those, and a `.bat` file
at the top of the submission looks like the project's build script when it
actually builds his demo.

**Where is a game's result decided, and how does it reach the screen?**
In our code, never the GUI's (§2, §39). `ChessGame` decides the result and the
reason and words them — "Black wins!" and "White resigned". In an online game
the Server's `ChessGame` does it and sends both strings in `GameOver`, so both
Clients show identical text. The Client passes them to `showGameOver`, and also
puts the result in the side panel, because the GUI shows an empty status as
"Game in progress", which is what a finished game used to say once its summary
box was closed.

**How did you test the package itself, rather than your development folder?**
By unzipping it into a fresh folder and following the README from there, in the
MSYS2 UCRT64 shell as it says. Client windows were driven by posting mouse clicks
to them, and captured to images to check what a player sees. Both programs were
also rebuilt from the unzipped copy with the README's exact commands, and the
test programs run there.

**A grader double-clicks `client.exe` and nothing happens. Why?**
Windows could not find SFML's DLLs. None are in the submission, so they have to
come from `PATH`, and a double-click in Explorer uses the system `PATH`, which
normally does not include `C:\msys64\ucrt64\bin`. Windows then refuses to start
the program before `main` runs, with a "was not found" dialog or silently with
exit code `0xC0000135`. Run from the MSYS2 UCRT64 shell, as README section 3
says, it starts normally.
