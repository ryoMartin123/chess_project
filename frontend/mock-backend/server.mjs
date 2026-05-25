import http from "node:http";

const PORT = Number(process.env.PORT || 8787);
const startingBoard = [
  ["r", "n", "b", "q", "k", "b", "n", "r"],
  ["p", "p", "p", "p", "p", "p", "p", "p"],
  ["", "", "", "", "", "", "", ""],
  ["", "", "", "", "", "", "", ""],
  ["", "", "", "", "", "", "", ""],
  ["", "", "", "", "", "", "", ""],
  ["P", "P", "P", "P", "P", "P", "P", "P"],
  ["R", "N", "B", "Q", "K", "B", "N", "R"],
];

let game = createNewGame();

function copyBoard(board) {
  return board.map((row) => [...row]);
}

function createNewGame() {
  return {
    board: copyBoard(startingBoard),
    turn: "White",
    gameOver: false,
    message: "New mock game started. White to move.",
  };
}

function sendJson(response, statusCode, body) {
  response.writeHead(statusCode, {
    "Content-Type": "application/json",
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Methods": "GET,POST,OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type",
  });
  response.end(JSON.stringify(body));
}

function isSquare(square) {
  return (
    square &&
    Number.isInteger(square.row) &&
    Number.isInteger(square.col) &&
    square.row >= 0 &&
    square.row < 8 &&
    square.col >= 0 &&
    square.col < 8
  );
}

function coordinatesForSquare(square) {
  if (!/^[a-h][1-8]$/.test(square || "")) {
    return null;
  }

  return {
    row: 8 - Number(square[1]),
    col: square.charCodeAt(0) - "a".charCodeAt(0),
  };
}

function squareForCoordinates(row, col) {
  return `${String.fromCharCode("a".charCodeAt(0) + col)}${8 - row}`;
}

function isWhitePiece(piece) {
  return piece && piece === piece.toUpperCase();
}

function mockDestinations(square) {
  const origin = coordinatesForSquare(square);
  if (!origin) {
    return [];
  }

  const selectedPiece = game.board[origin.row][origin.col];
  if (!selectedPiece) {
    return [];
  }

  const moves = [];
  for (let row = 0; row < 8; row += 1) {
    for (let col = 0; col < 8; col += 1) {
      const destinationPiece = game.board[row][col];
      const samePiece = row === origin.row && col === origin.col;
      const sameSide = destinationPiece && isWhitePiece(destinationPiece) === isWhitePiece(selectedPiece);

      if (!samePiece && !sameSide) {
        moves.push(squareForCoordinates(row, col));
      }
    }
  }

  return moves;
}

function gameResponse(valid, message = game.message) {
  return {
    valid,
    board: copyBoard(game.board),
    turn: game.turn,
    gameOver: game.gameOver,
    message,
  };
}

function applyMove(move, promotion) {
  const selectedPiece = game.board[move.from.row][move.from.col];

  if (!selectedPiece) {
    return gameResponse(false, "No piece exists on the selected square.");
  }

  const promotedSymbols = {
    Queen: "q",
    Rook: "r",
    Bishop: "b",
    Knight: "n",
  };
  let placedPiece = selectedPiece;

  if (selectedPiece.toLowerCase() === "p" && (move.to.row === 0 || move.to.row === 7)) {
    const promoted = promotedSymbols[promotion || "Queen"] || "q";
    placedPiece = selectedPiece === selectedPiece.toUpperCase() ? promoted.toUpperCase() : promoted;
  }

  game.board[move.to.row][move.to.col] = placedPiece;
  game.board[move.from.row][move.from.col] = "";
  game.turn = game.turn === "Black" ? "White" : "Black";
  game.message = `Mock accepted move: ${move.from.square} to ${move.to.square}`;

  return gameResponse(true);
}

async function readJson(request) {
  const chunks = [];

  for await (const chunk of request) {
    chunks.push(chunk);
  }

  return JSON.parse(Buffer.concat(chunks).toString("utf8") || "{}");
}

const server = http.createServer(async (request, response) => {
  if (request.method === "OPTIONS") {
    sendJson(response, 204, {});
    return;
  }

  if (request.method === "GET" && request.url === "/api/health") {
    sendJson(response, 200, {
      ok: true,
      service: "chess-mock-backend",
    });
    return;
  }

  if (request.method === "GET" && request.url === "/api/state") {
    sendJson(response, 200, gameResponse(true));
    return;
  }

  if (request.method === "POST" && request.url === "/api/reset") {
    game = createNewGame();
    sendJson(response, 200, gameResponse(true));
    return;
  }

  if (request.method === "POST" && request.url === "/api/legal-moves") {
    try {
      const { square } = await readJson(request);
      sendJson(response, 200, {
        valid: true,
        square,
        moves: mockDestinations(square),
      });
    } catch (error) {
      sendJson(response, 400, {
        valid: false,
        message: `Invalid JSON request: ${error.message}`,
      });
    }
    return;
  }

  if (request.method === "POST" && request.url === "/api/move") {
    try {
      const { move, promotion = "" } = await readJson(request);

      if (!isSquare(move?.from) || !isSquare(move?.to)) {
        sendJson(response, 400, {
          valid: false,
          message: "Expected { move: { from, to } }.",
        });
        return;
      }

      sendJson(response, 200, applyMove(move, promotion));
    } catch (error) {
      sendJson(response, 400, {
        valid: false,
        message: `Invalid JSON request: ${error.message}`,
      });
    }
    return;
  }

  sendJson(response, 404, {
    valid: false,
    message: "Route not found.",
  });
});

server.listen(PORT, () => {
  console.log(`Mock chess backend listening on http://localhost:${PORT}`);
});
