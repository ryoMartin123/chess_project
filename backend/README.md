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
{ "player": "White", "cardId": "disintegrate", "targetSquare": "a2" }
```

`POST /api/card/discard` discards a card during the player's own turn and schedules its replacement draw for the start of their next turn:

```json
{ "player": "White", "cardId": "disintegrate" }
```

## Card Examples

`CardManager.cpp` defines the development card catalog, including `disintegrate`, `charge`, `onslaught`, `knightmare`, `bog`, and `neutrality`. Turn cards such as `charge` and `onslaught` apply their effects and then pass the turn; `disintegrate` is a `BEFORE_OWN_TURN` card and leaves the player free to move afterward. Reaction cards such as `knightmare` and `bog` are played after the opponent's turn and do not spend your turn. Continuing-effect cards such as `neutrality` mark a board piece and keep changing the rules until the piece is captured or a future card removes the effect.

Each `CardDefinition` includes `rulesText`, a longer plain-English description that the frontend can later show in an inspect/details panel.

For development, both players build their starting deck from the current test card catalog, then draw a five-card opening hand from that deck. Discarding schedules one draw from the remaining deck at the start of that player's next turn.

```sh
g++.exe -std=c++17 card_testing.cpp GameState.cpp CardManager.cpp ChessBoard.cpp ChessPiece.cpp Move.cpp -o card_testing.exe
.\card_testing.exe
```
