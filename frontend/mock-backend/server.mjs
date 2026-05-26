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

const CARD_DEFINITIONS = [
  {
    id: "destroy_pawn",
    name: "Destroy Pawn",
    level: 1,
    timing: "AS_OWN_TURN",
    targetRequirement: "ENEMY_PAWN",
    effect: "DESTROY_TARGET",
    countsAsOwnTurnCard: true,
  },
  {
    id: "charge",
    name: "Charge",
    level: 6,
    timing: "AS_OWN_TURN",
    targetRequirement: "FRIENDLY_KNIGHT",
    effect: "CHARGE_KNIGHT",
    countsAsOwnTurnCard: true,
  },
  {
    id: "onslaught",
    name: "Onslaught",
    level: 6,
    timing: "AS_OWN_TURN",
    targetRequirement: "FRIENDLY_PAWNS",
    effect: "MOVE_PAWNS",
    countsAsOwnTurnCard: true,
  },
];

let game = createNewGame();

function copyBoard(board) {
  return board.map((row) => [...row]);
}

function createPlayerCards() {
  return {
    deckCount: 3,
    hand: ["destroy_pawn", "charge", "onslaught"],
    discardPile: [],
    cardPlayedOnOwnTurn: false,
    discardedThisTurn: false,
    drawAtStartOfNextTurn: false,
  };
}

function createNewGame() {
  return {
    board: copyBoard(startingBoard),
    turn: "White",
    gameOver: false,
    message: "New mock game started. White to move.",
    cards: { White: createPlayerCards(), Black: createPlayerCards() },
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
  if (!/^[a-h][1-8]$/.test(square || "")) return null;
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
  if (!origin) return [];
  const selectedPiece = game.board[origin.row][origin.col];
  if (!selectedPiece) return [];

  const moves = [];
  for (let row = 0; row < 8; row += 1) {
    for (let col = 0; col < 8; col += 1) {
      const dest = game.board[row][col];
      const same = row === origin.row && col === origin.col;
      const sameSide = dest && isWhitePiece(dest) === isWhitePiece(selectedPiece);
      if (!same && !sameSide) moves.push(squareForCoordinates(row, col));
    }
  }
  return moves;
}

function copyCards() {
  return {
    White: { ...game.cards.White, hand: [...game.cards.White.hand], discardPile: [...game.cards.White.discardPile] },
    Black: { ...game.cards.Black, hand: [...game.cards.Black.hand], discardPile: [...game.cards.Black.discardPile] },
  };
}

function gameResponse(valid, message = game.message) {
  return {
    valid,
    board: copyBoard(game.board),
    turn: game.turn,
    gameOver: game.gameOver,
    message,
    cards: copyCards(),
  };
}

function cardActionResponse(valid, message, cardId, targetSquare, drawnCardId = "") {
  return {
    ...gameResponse(valid, message),
    cardResult: { success: valid, message, cardId, targetSquare, drawnCardId },
  };
}

function startTurnForPlayer(player) {
  const state = game.cards[player];
  state.cardPlayedOnOwnTurn = false;
  state.discardedThisTurn = false;

  if (state.drawAtStartOfNextTurn && state.deckCount > 0) {
    state.drawAtStartOfNextTurn = false;
    state.deckCount -= 1;
    state.hand.push("destroy_pawn");
  } else {
    state.drawAtStartOfNextTurn = false;
  }
}

function applyMove(move, promotion) {
  const selectedPiece = game.board[move.from.row][move.from.col];
  if (!selectedPiece) return gameResponse(false, "No piece exists on the selected square.");

  const promotedSymbols = { Queen: "q", Rook: "r", Bishop: "b", Knight: "n" };
  let placedPiece = selectedPiece;
  if (selectedPiece.toLowerCase() === "p" && (move.to.row === 0 || move.to.row === 7)) {
    const promoted = promotedSymbols[promotion || "Queen"] || "q";
    placedPiece = selectedPiece === selectedPiece.toUpperCase() ? promoted.toUpperCase() : promoted;
  }

  game.board[move.to.row][move.to.col] = placedPiece;
  game.board[move.from.row][move.from.col] = "";
  game.turn = game.turn === "White" ? "Black" : "White";
  game.message = `Mock accepted move: ${move.from.square} to ${move.to.square}`;

  startTurnForPlayer(game.turn);
  return gameResponse(true);
}

function applyPlayCard(player, cardId, targetSquare, fromSquare, secondTargetSquare, fromSquares = [], promotion = "Queen") {
  if (game.turn !== player) {
    return cardActionResponse(false, "You can only play cards on your own turn.", cardId, targetSquare);
  }
  const state = game.cards[player];
  if (!state.hand.includes(cardId)) {
    return cardActionResponse(false, "That card is not in your hand.", cardId, targetSquare);
  }
  if (state.cardPlayedOnOwnTurn) {
    return cardActionResponse(false, "You have already played your own-turn card this turn.", cardId, targetSquare);
  }

  if (cardId === "destroy_pawn") {
    const coords = coordinatesForSquare(targetSquare);
    if (!coords) return cardActionResponse(false, "Choose a valid target square.", cardId, targetSquare);
    const targetPiece = game.board[coords.row][coords.col];
    const isEnemyPawn =
      targetPiece &&
      targetPiece.toLowerCase() === "p" &&
      isWhitePiece(targetPiece) !== (player === "White");
    if (!isEnemyPawn) return cardActionResponse(false, "Destroy Pawn must target one enemy pawn.", cardId, targetSquare);

    game.board[coords.row][coords.col] = "";
    state.hand = state.hand.filter((id) => id !== cardId);
    state.discardPile.push(cardId);
    game.turn = player === "White" ? "Black" : "White";
    startTurnForPlayer(game.turn);
    return cardActionResponse(true, `${player} played Destroy Pawn on ${targetSquare}.`, cardId, targetSquare);
  }

  if (cardId === "charge") {
    const knightCoords = coordinatesForSquare(fromSquare);
    const midCoords    = coordinatesForSquare(targetSquare);
    const landCoords   = coordinatesForSquare(secondTargetSquare);

    if (!knightCoords) return cardActionResponse(false, "Choose a valid knight square.", cardId, targetSquare);
    if (!midCoords)    return cardActionResponse(false, "Choose a valid intermediate square.", cardId, targetSquare);
    if (!landCoords)   return cardActionResponse(false, "Choose a valid final square.", cardId, targetSquare);

    const expectedKnight = player === "White" ? "N" : "n";
    if (game.board[knightCoords.row][knightCoords.col] !== expectedKnight) {
      return cardActionResponse(false, "Charge must start from one of your knights.", cardId, targetSquare);
    }
    if (game.board[midCoords.row][midCoords.col] !== "") {
      return cardActionResponse(false, "Charge cannot capture on the first hop.", cardId, targetSquare);
    }

    // Execute the two-hop move
    game.board[midCoords.row][midCoords.col] = expectedKnight;
    game.board[knightCoords.row][knightCoords.col] = "";
    game.board[landCoords.row][landCoords.col] = expectedKnight;
    game.board[midCoords.row][midCoords.col] = "";

    state.hand = state.hand.filter((id) => id !== cardId);
    state.discardPile.push(cardId);
    game.turn = player === "White" ? "Black" : "White";
    startTurnForPlayer(game.turn);
    return cardActionResponse(
      true,
      `${player} played Charge: ${fromSquare} → ${targetSquare} → ${secondTargetSquare}.`,
      cardId,
      targetSquare
    );
  }

  if (cardId === "onslaught") {
    if (!Array.isArray(fromSquares) || fromSquares.length === 0) {
      return cardActionResponse(false, "Onslaught must choose at least one pawn.", cardId, "");
    }

    const seen = new Set();
    const pawnMoves = [];
    for (const square of fromSquares) {
      if (seen.has(square)) return cardActionResponse(false, "Onslaught cannot choose the same pawn twice.", cardId, "");
      seen.add(square);

      const coords = coordinatesForSquare(square);
      if (!coords) return cardActionResponse(false, "Onslaught must choose valid pawn squares.", cardId, "");

      const pawn = game.board[coords.row][coords.col];
      const expectedPawn = player === "White" ? "P" : "p";
      if (pawn !== expectedPawn) {
        return cardActionResponse(false, "Onslaught may only choose your pawns.", cardId, "");
      }

      const destinationRow = player === "White" ? coords.row - 1 : coords.row + 1;
      if (destinationRow < 0 || destinationRow >= 8 || game.board[destinationRow][coords.col] !== "") {
        return cardActionResponse(false, "Onslaught pawns must move one square forward without capturing.", cardId, "");
      }

      pawnMoves.push({ from: coords, to: { row: destinationRow, col: coords.col } });
    }

    const promotedSymbols = { Queen: "q", Rook: "r", Bishop: "b", Knight: "n" };
    for (const move of pawnMoves) {
      let placedPiece = game.board[move.from.row][move.from.col];
      if (move.to.row === 0 || move.to.row === 7) {
        const promoted = promotedSymbols[promotion || "Queen"] || "q";
        placedPiece = player === "White" ? promoted.toUpperCase() : promoted;
      }
      game.board[move.to.row][move.to.col] = placedPiece;
      game.board[move.from.row][move.from.col] = "";
    }

    state.hand = state.hand.filter((id) => id !== cardId);
    state.discardPile.push(cardId);
    game.turn = player === "White" ? "Black" : "White";
    startTurnForPlayer(game.turn);
    return cardActionResponse(true, `${player} played Onslaught and moved ${pawnMoves.length} pawn${pawnMoves.length === 1 ? "" : "s"}.`, cardId, "");
  }

  return cardActionResponse(false, "Unknown card effect.", cardId, targetSquare);
}

function applyDiscardCard(player, cardId) {
  if (game.turn !== player) {
    return cardActionResponse(false, "You may discard only on your own turn.", cardId, "");
  }

  const state = game.cards[player];
  if (!state.hand.includes(cardId)) {
    return cardActionResponse(false, "That card is not in your hand.", cardId, "");
  }
  if (state.discardedThisTurn) {
    return cardActionResponse(false, "You have already discarded a card this turn.", cardId, "");
  }

  state.hand = state.hand.filter((id) => id !== cardId);
  state.discardPile.push(cardId);
  state.discardedThisTurn = true;
  state.drawAtStartOfNextTurn = true;

  return cardActionResponse(
    true,
    `${player} discarded ${cardId} and will draw at the start of their next turn.`,
    cardId,
    ""
  );
}

async function readJson(request) {
  const chunks = [];
  for await (const chunk of request) chunks.push(chunk);
  return JSON.parse(Buffer.concat(chunks).toString("utf8") || "{}");
}

const server = http.createServer(async (request, response) => {
  if (request.method === "OPTIONS") { sendJson(response, 204, {}); return; }

  if (request.method === "GET" && request.url === "/api/health") {
    sendJson(response, 200, { ok: true, service: "chess-mock-backend" });
    return;
  }

  if (request.method === "GET" && request.url === "/api/state") {
    sendJson(response, 200, gameResponse(true));
    return;
  }

  if (request.method === "GET" && request.url === "/api/cards") {
    sendJson(response, 200, { definitions: CARD_DEFINITIONS, players: copyCards() });
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
      sendJson(response, 200, { valid: true, square, moves: mockDestinations(square) });
    } catch (error) {
      sendJson(response, 400, { valid: false, message: `Invalid JSON: ${error.message}` });
    }
    return;
  }

  if (request.method === "POST" && request.url === "/api/move") {
    try {
      const { move, promotion = "" } = await readJson(request);
      if (!isSquare(move?.from) || !isSquare(move?.to)) {
        sendJson(response, 400, { valid: false, message: "Expected { move: { from, to } }." });
        return;
      }
      sendJson(response, 200, applyMove(move, promotion));
    } catch (error) {
      sendJson(response, 400, { valid: false, message: `Invalid JSON: ${error.message}` });
    }
    return;
  }

  if (request.method === "POST" && request.url === "/api/card/play") {
    try {
      const { player, cardId, targetSquare, fromSquare, secondTargetSquare, fromSquares, promotion } = await readJson(request);
      sendJson(response, 200, applyPlayCard(player, cardId, targetSquare, fromSquare, secondTargetSquare, fromSquares, promotion));
    } catch (error) {
      sendJson(response, 400, { valid: false, message: `Invalid JSON: ${error.message}` });
    }
    return;
  }

  if (request.method === "POST" && request.url === "/api/card/discard") {
    try {
      const { player, cardId } = await readJson(request);
      sendJson(response, 200, applyDiscardCard(player, cardId));
    } catch (error) {
      sendJson(response, 400, { valid: false, message: `Invalid JSON: ${error.message}` });
    }
    return;
  }

  sendJson(response, 404, { valid: false, message: "Route not found." });
});

server.listen(PORT, () => {
  console.log(`Mock chess backend listening on http://localhost:${PORT}`);
});
