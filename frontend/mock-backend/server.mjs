import http from "node:http";

const PORT = Number(process.env.PORT || 8787);

function sendJson(response, statusCode, body) {
  response.writeHead(statusCode, {
    "Content-Type": "application/json",
    "Access-Control-Allow-Origin": "*",
    "Access-Control-Allow-Methods": "GET,POST,OPTIONS",
    "Access-Control-Allow-Headers": "Content-Type",
  });
  response.end(JSON.stringify(body));
}

function isBoard(board) {
  return (
    Array.isArray(board) &&
    board.length === 8 &&
    board.every((row) => Array.isArray(row) && row.length === 8)
  );
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

function applyMove(move, board) {
  const newBoard = board.map((row) => [...row]);
  const selectedPiece = newBoard[move.from.row][move.from.col];

  if (!selectedPiece) {
    return {
      valid: false,
      board,
      message: "No piece exists on the selected square.",
    };
  }

  newBoard[move.to.row][move.to.col] = selectedPiece;
  newBoard[move.from.row][move.from.col] = "";

  return {
    valid: true,
    board: newBoard,
    message: `Backend accepted move: ${move.from.square} to ${move.to.square}`,
  };
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

  if (request.method === "POST" && request.url === "/api/move") {
    try {
      const { move, board } = await readJson(request);

      if (!isBoard(board) || !isSquare(move?.from) || !isSquare(move?.to)) {
        sendJson(response, 400, {
          valid: false,
          message: "Expected { board: 8x8 array, move: { from, to } }.",
        });
        return;
      }

      sendJson(response, 200, applyMove(move, board));
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
