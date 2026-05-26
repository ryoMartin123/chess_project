# Chess C++ Backend

This local HTTP server exposes one live chess game and applies moves through the C++ rules engine. `GameState.cpp` owns the active `Board` and starting piece lifetimes; keeping that state alive preserves en passant eligibility, castling rights, promotion results, and game-over state across frontend requests.

## Build And Run

```sh
g++.exe -std=c++17 ApiServer.cpp GameState.cpp CardManager.cpp ChessBoard.cpp ChessPiece.cpp Move.cpp -lws2_32 -o chess_backend.exe
.\chess_backend.exe
```

The server listens on `http://127.0.0.1:8787`.

JSON request parsing and response serialization use the vendored `nlohmann/json` v3.11.3 single-header library in `third_party/nlohmann/`, distributed under its included MIT license.

## Routes

`GET /api/health` reports whether the service is running.

`GET /api/state` returns the current authoritative `board`, `turn`, and `gameOver` state.

`POST /api/reset` starts a fresh standard game and returns its state.

`POST /api/legal-moves` with `{ "square": "e2" }` returns legal destinations for the selected piece without changing game state.

`POST /api/move` submits one move:

```json
{
  "move": {
    "from": { "row": 6, "col": 4, "square": "e2" },
    "to": { "row": 4, "col": 4, "square": "e4" }
  },
  "promotion": "Knight"
}
```

`promotion` is optional and applies only when a pawn reaches the last rank; omitted promotion defaults to `Queen`. The response contains `valid`, the server board, `turn`, `gameOver`, and an engine message.

`GET /api/cards` returns the card catalog and both local players' card state.

`POST /api/card/play` validates and applies a card against the live board:

```json
{ "player": "White", "cardId": "destroy_pawn", "targetSquare": "a7" }
```

`POST /api/card/discard` discards a card during the player's own turn and schedules its replacement draw for the start of their next turn:

```json
{ "player": "White", "cardId": "destroy_pawn" }
```

## Card Examples

`CardManager.cpp` defines the development card catalog, including `destroy_pawn`, `charge`, and `onslaught`. These are `AS_OWN_TURN` cards, so playing one applies its effect and then passes the turn.

For development, both players start with one copy of each test card in hand and one in their deck. This makes the play path and the discard-then-draw path testable before deck building exists.

```sh
g++.exe -std=c++17 card_testing.cpp GameState.cpp CardManager.cpp ChessBoard.cpp ChessPiece.cpp Move.cpp -o card_testing.exe
.\card_testing.exe
```
