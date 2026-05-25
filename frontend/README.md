# Chess Frontend

This React/Vite board plays a live game against the local C++ rules engine. The backend owns game state, and the frontend loads its current position, submits moves, supports promotion choice, and can reset the game.

## Run

Start the C++ server:

```sh
cd ..\backend
g++.exe -std=c++17 ApiServer.cpp ChessBoard.cpp ChessPiece.cpp Move.cpp -lws2_32 -o chess_backend.exe
.\chess_backend.exe
```

Start Vite from `frontend`:

```sh
npm run dev
```

Vite proxies `/api` to `http://127.0.0.1:8787`. To use a backend on another origin, create `.env.local`:

```sh
VITE_CHESS_API_URL=http://127.0.0.1:8787
VITE_API_TIMEOUT_MS=1200
```

## Development Mock

`npm run backend:mock` runs a lightweight stateful transport mock for frontend interaction work. It does not enforce chess legality; use the C++ backend to play a valid game.

## API Contract

The frontend uses:

- `GET /api/health`
- `GET /api/state`
- `POST /api/reset`
- `POST /api/legal-moves` with `{ "square": "e2" }`
- `POST /api/move` with `{ "move": { "from": { "square": "e2" }, "to": { "square": "e4" } }, "promotion": "Queen" }`

Move responses contain the authoritative `board`, `turn`, `gameOver`, `valid`, and `message` values.
