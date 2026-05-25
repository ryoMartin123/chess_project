# Chess C++ Backend

This local HTTP server owns one live chess game and applies moves through the C++ rules engine. Keeping the `Board` instance alive preserves en passant eligibility, castling rights, promotion results, and game-over state across frontend requests.

## Build And Run

```sh
g++.exe -std=c++17 ApiServer.cpp ChessBoard.cpp ChessPiece.cpp Move.cpp -lws2_32 -o chess_backend.exe
.\chess_backend.exe
```

The server listens on `http://127.0.0.1:8787`.

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
