# Requirements checklist (all 89)

Transcribed from the lecturer's requirements document. Every item not marked
optional is mandatory. Use this for the final self-check before zipping.

**Status: 87 of 89 done.** The two outstanding are §25, which needs a second
computer on the same network, and §84, which needs the student's ID number for
the zip filename. Both are marked in place below.

Everything else has been verified rather than assumed — the rules against the
standard perft suite, the Bot's difficulty ordering by playing the levels
against each other, and the Server against a test that connects as several
Clients at once. The staged submission was run on this machine with MSYS2
removed from `PATH`, so the bundled DLLs and the relative Stockfish path are
known to be complete.

## System structure
- [x] 1. Two separate programs: Client and Server
- [x] 2. GUI is display + input only; does not validate moves, manage the engine, do networking, manage online games, or decide the winner
- [x] 3. GUI_USAGE.md explains the provided API
- [x] 4. Screens other than the game screen may be console-based
- [x] 5. Networking via SFML Network
- [x] 6. The same chess rules apply in every game mode

## Startup and connection
- [x] 7. First Client screen is a connection screen taking Server IP and port
- [x] 8. Main menu reachable only after a successful connection
- [x] 9. Invalid IP, invalid port, server down, or failed connection show a clear error
- [x] 10. After a failure: retry, change IP, change port, or exit
- [x] 11. Server prints the IP and port it listens on; clients use those

## Client main menu
- [x] 12. Menu: Local Game, Player vs Bot, Online Game, Exit
- [x] 13. Local Game — two players on one Client
- [x] 14. Player vs Bot
- [x] 15. Online Game — find another player via matchmaking
- [x] 16. Exit

## Game modes
- [x] 17. Local: two humans, one Client; created, managed and validated on the Client; Server not involved
- [x] 18. Bot: created and managed on the Client; engine and bot logic run on the Client
- [x] 19. At least three Bot difficulty levels
- [x] 20. Difficulty differences significant and demonstrable
- [x] 21. Online: two players on two Clients against one Server, paired by matchmaking
- [x] 22. Online: game created on the Server; board, state and turn stored there; every move validated there
- [x] 23. Online: Client only displays received state, takes user action, sends a request
- [x] 24. Client never applies an online move itself and never treats its own request as final; new state shown only after Server approval
- [ ] 25. Online play works between Clients on different machines on the same network; no assumption that all Clients share one machine
      — **the only untested requirement.** Nothing in the code assumes one
      machine: the Server binds all interfaces and prints its LAN address, and
      two Clients on one machine already play each other through it. But it has
      not been run across two computers, and that is what this asks. Needs a
      second machine on the same network.

## Chess rules
- [x] 26. Legal movement for every piece type
- [x] 27. Capturing enemy pieces; capturing your own forbidden
- [x] 28. Movement blocked by pieces in the way, per piece type
- [x] 29. Correct turn order
- [x] 30. Check detection
- [x] 31. Moves leaving your own king in check are forbidden
- [x] 32. Checkmate detection
- [x] 33. Stalemate detection
- [x] 34. Castling, including every condition
- [x] 35. En passant
- [x] 36. Promotion, choosing between Queen, Rook, Bishop, Knight
- [x] 37. Game end by resignation
- [x] 38. Draw by agreement in Local and Online games
- [x] 39. On game end, determine the result and the reason and pass both to the GUI

## Online game and matchmaking
- [x] 40. A Client choosing Online notifies the Server it is searching; with no other free Client it enters a waiting state
- [x] 41. With two free Clients searching, the Server pairs them, creates the game, associates both, assigns colours, and notifies both that it started
- [x] 42. A Client is free if connected, has requested an Online Game, and is not already in one; remove it from matchmaking on cancel or disconnect

## Bot
- [x] 43. Before a Bot game the user picks easy, medium or hard
- [x] 44. The game starts once difficulty is chosen
- [x] 45. No self-written move-selection algorithm; use Stockfish
- [x] 46. Stockfish runs as a separate process, communicated with to obtain its move
- [x] 47. Integration is yours; Stockfish selects the Bot move only — validation and game state stay in your system

## Draw offers
- [x] 48. In Local or Online games either player can offer a draw via Offer Draw
- [x] 49. Local: the offer dialog appears on the same Client, for the opponent
- [x] 50. Online: the offer travels through the Server and appears only on the opponent's Client
- [x] 51. The receiving player can Accept or Decline
- [x] 52. Accept ends the game as a draw with a message; Decline continues play

## Resignation
- [x] 53. Any human player can resign during any game
- [x] 54. Resign shows a confirmation dialog — same Client for Local and Bot, only the resigning player's Client for Online
- [x] 55. The dialog offers Confirm and No; resignation happens only on Confirm
- [x] 56. After confirming, the opponent wins and the resigner loses; the result is displayed; in Online games the Server updates both Clients

## Concurrency
- [x] 57. The Server supports many simultaneous Clients and many active Online games
- [x] 58. Every game is independent; an action in one never alters another
- [x] 59. Updates are never sent to Clients outside the game concerned
- [x] 60. The implementation never assumes only one game exists on the Server

## Protocol
- [x] 61. You design the application protocol yourself; none is provided and no format is mandated
- [x] 62. It supports connect and disconnect, entering and leaving the online wait, matchmaking, creating and managing online games, transferring player actions and state updates, resignation, draw offers, and game end
- [x] 63. It is clear, consistent, explainable and maintainable

## Disconnections and error handling
- [x] 64. A Client disconnecting never crashes the Server or affects other online games
- [x] 65. Disconnection is handled mid-game, while waiting for matchmaking, and on system screens
- [x] 66. If a player disconnects mid-game the Server updates or ends it and the other player is told
- [x] 67. Bad user input, invalid network messages, and messages that do not fit the current Client or game state are all handled

## Server console output
- [x] 68. The Server prints meaningful status messages, understandable without a debugger
- [x] 69. Logged events include client connect/disconnect, entering and leaving the online wait, matchmaking, game creation and player association, accepted and rejected moves, draw offers, resignation, and game end
- [x] 70. No mandated format, but clear, readable and useful

## C++ design and implementation
- [x] 71. Logical division into classes and components with separated responsibilities
- [x] 72. Encapsulation
- [x] 73. Inheritance and polymorphism where they suit the design
- [x] 74. Appropriate use of the STL and suitable data structures
- [x] 75. Correct use of const
- [x] 76. RAII and correct resource management
- [x] 77. Smart pointers where ownership is dynamic and they suit the design
- [x] 78. No memory leaks, no double frees
- [x] 79. Orderly separation of .h and .cpp files
- [x] 80. Readable, modular code with reasonable error and edge-case handling

## Submission
- [x] 81. Personal project; each student submits separately
- [x] 82. All Client and Server source including ProvidedGUI, plus both executables and the Stockfish binary used
- [x] 83. A README letting the checker compile and run with no guesswork: compiling, running the Server, where IP and port appear, running a Client and connecting, starting a Local Game, starting a Bot game and choosing difficulty, starting an Online Game and using matchmaking, running several Clients to demonstrate concurrent online games, how Stockfish is integrated, and any other environment requirement
- [ ] 84. All files in a zip named `<ID>.zip` — waiting on the student's ID
      number. Everything else is staged and verified; the zip comes to 82.3 MB.

## Build environment
- [x] 85. C++17 under MSYS2 UCRT64 / MinGW — verified, g++ 16.2.0
- [x] 86. SFML 3 for GUI and networking, compiling without errors — verified, SFML 3.0.2
- [x] 87. Client and Server compile and run separately
- [x] 88. Client compiles with the documented g++ command
- [x] 89. Server compiles with the documented g++ command
