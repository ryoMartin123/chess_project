# Chess Frontend

This is a React/Vite chess frontend with a small mock backend for testing the frontend request shape before the real C++ backend is ready.

## Run the frontend

```sh
npm run dev
```

## Run the mock backend

In a second terminal:

```sh
npm run backend:mock
```

Vite proxies `/api` to `http://localhost:8787` during development. If the backend is not running, the frontend falls back to local move simulation so basic board testing still works.

## Backend contract

`POST /api/move`

Request:

```json
{
  "board": [
    ["r", "n", "b", "q", "k", "b", "n", "r"],
    ["p", "p", "p", "p", "p", "p", "p", "p"],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["P", "P", "P", "P", "P", "P", "P", "P"],
    ["R", "N", "B", "Q", "K", "B", "N", "R"]
  ],
  "move": {
    "from": { "row": 6, "col": 4, "square": "e2" },
    "to": { "row": 4, "col": 4, "square": "e4" }
  }
}
```

Response:

```json
{
  "valid": true,
  "board": [
    ["r", "n", "b", "q", "k", "b", "n", "r"],
    ["p", "p", "p", "p", "p", "p", "p", "p"],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["", "", "", "", "P", "", "", ""],
    ["", "", "", "", "", "", "", ""],
    ["P", "P", "P", "P", "", "P", "P", "P"],
    ["R", "N", "B", "Q", "K", "B", "N", "R"]
  ],
  "message": "Backend accepted move: e2 to e4"
}
```

The C++ backend should implement this same route and response shape.
