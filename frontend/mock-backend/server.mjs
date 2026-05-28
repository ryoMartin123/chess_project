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
    id: "disintegrate",
    name: "Disintegrate",
    level: 2,
    timing: "BEFORE_OWN_TURN",
    targetRequirement: "FRIENDLY_NON_KING",
    effect: "DESTROY_TARGET",
    countsAsOwnTurnCard: false,
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
  {
    id: "knightmare",
    name: "Knightmare",
    level: 10,
    timing: "AFTER_OPPONENT_TURN",
    targetRequirement: "NONE",
    effect: "CANCEL_LAST_ACTION",
    countsAsOwnTurnCard: false,
  },
  {
    id: "think_again",
    name: "Think Again!",
    level: 10,
    timing: "AFTER_OPPONENT_TURN",
    targetRequirement: "NONE",
    effect: "CANCEL_LAST_ACTION",
    countsAsOwnTurnCard: false,
  },
  {
    id: "bog",
    name: "Bog",
    level: 4,
    timing: "AFTER_OPPONENT_TURN",
    targetRequirement: "NONE",
    effect: "BOG_MOVE",
    countsAsOwnTurnCard: false,
  },
  {
    id: "neutrality",
    name: "Neutrality",
    level: 9,
    timing: "AFTER_OWN_TURN",
    targetRequirement: "ENEMY_NON_KING_QUEEN",
    effect: "APPLY_NEUTRALITY",
    countsAsOwnTurnCard: false,
    rulesText: "Played after your move. Choose one opponent piece except a king or queen. It becomes neutral and gets a marker. A neutral piece can check either king and can capture pieces of any color when moved.",
  },
  {
    id: "warlord",
    name: "Warlord",
    level: 10,
    timing: "AS_OWN_TURN",
    targetRequirement: "FRIENDLY_KING",
    effect: "APPLY_WARLORD",
    countsAsOwnTurnCard: true,
    rulesText: "Played as your turn. Your King becomes a Warlord — it may move up to 2 squares per turn in any direction or combination. Making a capture ends its move for that turn. This continuing effect lasts until the Warlord is captured or the game ends.",
  },
];

let lastAction = null;
let retryRestriction = null;
let game = createNewGame();

function copyBoard(board) {
  return board.map((row) => [...row]);
}

function shuffled(arr) {
  const a = [...arr];
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

function createPlayerCards() {
  const deck = shuffled(["disintegrate", "charge", "onslaught", "knightmare", "think_again", "bog", "neutrality", "warlord"]);
  return {
    deck,
    deckCount: deck.length,
    hand: [],
    discardPile: [],
    activePile: [],
    cardPlayedOnOwnTurn: false,
    discardedThisTurn: false,
    drawAtStartOfNextTurn: false,
    drawnThisTurn: false,
  };
}

function createNewGame() {
  lastAction = null;
  retryRestriction = null;
  return {
    board: copyBoard(startingBoard),
    turn: "White",
    gameOver: false,
    pendingCheckmate: "",
    message: "New mock game started. White to move.",
    neutralSquares: [],
    warlordSquares: [],
    continuingEffects: [],
    nextContinuingEffectId: 1,
    cards: { White: createPlayerCards(), Black: createPlayerCards() },
  };
}

function snapshotGame() {
  return {
    board: copyBoard(game.board),
    turn: game.turn,
    gameOver: game.gameOver,
    pendingCheckmate: game.pendingCheckmate ?? "",
    message: game.message,
    neutralSquares: [...game.neutralSquares],
    warlordSquares: [...(game.warlordSquares ?? [])],
    continuingEffects: game.continuingEffects.map((effect) => ({ ...effect })),
    nextContinuingEffectId: game.nextContinuingEffectId,
    cards: copyCards(),
  };
}

function restoreSnapshot(snapshot) {
  game = {
    board: copyBoard(snapshot.board),
    turn: snapshot.turn,
    gameOver: snapshot.gameOver,
    pendingCheckmate: snapshot.pendingCheckmate ?? "",
    message: snapshot.message,
    neutralSquares: [...(snapshot.neutralSquares ?? [])],
    warlordSquares: [...(snapshot.warlordSquares ?? [])],
    continuingEffects: (snapshot.continuingEffects ?? []).map((effect) => ({ ...effect })),
    nextContinuingEffectId: snapshot.nextContinuingEffectId ?? 1,
    cards: {
      White: { ...snapshot.cards.White, deck: [...snapshot.cards.White.deck], hand: [...snapshot.cards.White.hand], discardPile: [...snapshot.cards.White.discardPile], activePile: [...(snapshot.cards.White.activePile ?? [])] },
      Black: { ...snapshot.cards.Black, deck: [...snapshot.cards.Black.deck], hand: [...snapshot.cards.Black.hand], discardPile: [...snapshot.cards.Black.discardPile], activePile: [...(snapshot.cards.Black.activePile ?? [])] },
    },
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

function isNeutralSquare(square) {
  return game.neutralSquares.includes(square);
}

function markNeutral(square) {
  if (!isNeutralSquare(square)) game.neutralSquares.push(square);
}

function addContinuingEffect(cardId, playedBy, targetSquare) {
  game.continuingEffects.push({
    id: game.nextContinuingEffectId,
    cardId,
    playedBy,
    targetSquare,
    active: true,
  });
  game.nextContinuingEffectId += 1;
}

function deactivateContinuingEffectsAt(square) {
  game.continuingEffects = game.continuingEffects.map((effect) => {
    if (effect.active && effect.targetSquare === square) {
      const playerCards = game.cards[effect.playedBy];
      if (playerCards) {
        playerCards.activePile = (playerCards.activePile ?? []).filter((id) => id !== effect.cardId);
        playerCards.discardPile.push(effect.cardId);
      }
      return { ...effect, active: false };
    }
    return effect;
  });
  game.neutralSquares = game.neutralSquares.filter((neutralSquare) => neutralSquare !== square);
}

function moveNeutralMarker(fromSquare, toSquare) {
  const hadMarker = isNeutralSquare(fromSquare);
  game.neutralSquares = game.neutralSquares.filter((square) => square !== fromSquare && square !== toSquare);
  if (hadMarker) game.neutralSquares.push(toSquare);
  game.continuingEffects = game.continuingEffects.map((effect) => (
    effect.active && effect.targetSquare === fromSquare ? { ...effect, targetSquare: toSquare } : effect
  ));
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
  const selectedNeutral = isNeutralSquare(square);
  for (let row = 0; row < 8; row += 1) {
    for (let col = 0; col < 8; col += 1) {
      const destinationSquare = squareForCoordinates(row, col);
      const dest = game.board[row][col];
      const same = row === origin.row && col === origin.col;
      const destIsNeutral = isNeutralSquare(destinationSquare);
      const sameSide = dest && !selectedNeutral && !destIsNeutral && isWhitePiece(dest) === isWhitePiece(selectedPiece);
      const kingTarget = dest && dest.toLowerCase() === "k";
      if (!same && !sameSide && !kingTarget) moves.push(destinationSquare);
    }
  }
  return moves;
}

function copyCards() {
  return {
    White: { ...game.cards.White, deck: [...game.cards.White.deck], deckCount: game.cards.White.deck.length, hand: [...game.cards.White.hand], discardPile: [...game.cards.White.discardPile], activePile: [...(game.cards.White.activePile ?? [])] },
    Black: { ...game.cards.Black, deck: [...game.cards.Black.deck], deckCount: game.cards.Black.deck.length, hand: [...game.cards.Black.hand], discardPile: [...game.cards.Black.discardPile], activePile: [...(game.cards.Black.activePile ?? [])] },
  };
}

function moveWarlordMarker(fromSquare, toSquare) {
  if (!game.warlordSquares) return;
  const hadMarker = game.warlordSquares.includes(fromSquare);
  game.warlordSquares = game.warlordSquares.filter((s) => s !== fromSquare && s !== toSquare);
  if (hadMarker) game.warlordSquares.push(toSquare);
  game.continuingEffects = game.continuingEffects.map((effect) =>
    effect.active && effect.cardId === "warlord" && effect.targetSquare === fromSquare
      ? { ...effect, targetSquare: toSquare }
      : effect
  );
}

function gameResponse(valid, message = game.message) {
  return {
    valid,
    board: copyBoard(game.board),
    turn: game.turn,
    gameOver: game.gameOver,
    pendingCheckmate: game.pendingCheckmate ?? "",
    message,
    neutralSquares: [...game.neutralSquares],
    warlordSquares: [...(game.warlordSquares ?? [])],
    continuingEffects: game.continuingEffects.map((effect) => ({ ...effect })),
    cards: copyCards(),
  };
}

function cardActionResponse(valid, message, cardId, targetSquare, drawnCardId = "") {
  return {
    ...gameResponse(valid, message),
    cardResult: { success: valid, message, cardId, targetSquare, drawnCardId },
  };
}

function cardStateAfterCost(state, cardId) {
  return {
    ...state,
    hand: state.hand.filter((id) => id !== cardId),
    discardPile: [...state.discardPile, cardId],
  };
}

function describeBogMove(player) {
  if (!lastAction || lastAction.player === player || game.turn !== player || lastAction.actionType !== "move") {
    return null;
  }

  const from = coordinatesForSquare(lastAction.fromSquare);
  const to = coordinatesForSquare(lastAction.toSquare);
  if (!from || !to) return null;

  const piece = lastAction.beforeAction.board[from.row][from.col];
  if (!piece || isWhitePiece(piece) !== (lastAction.player === "White")) return null;

  const type = piece.toLowerCase();
  if (!["r", "b", "q"].includes(type)) return null;

  const rowDiff = to.row - from.row;
  const colDiff = to.col - from.col;
  const distance = Math.max(Math.abs(rowDiff), Math.abs(colDiff));
  const straight = rowDiff === 0 || colDiff === 0;
  const diagonal = Math.abs(rowDiff) === Math.abs(colDiff);
  const directionAllowed =
    (type === "r" && straight) ||
    (type === "b" && diagonal) ||
    (type === "q" && (straight || diagonal));
  if (distance < 2 || !directionAllowed) return null;

  const oneStep = {
    row: from.row + Math.sign(rowDiff),
    col: from.col + Math.sign(colDiff),
  };
  oneStep.square = squareForCoordinates(oneStep.row, oneStep.col);
  return { canceledAction: lastAction, from, oneStep };
}

function startTurnForPlayer(player) {
  const state = game.cards[player];
  state.cardPlayedOnOwnTurn = false;
  state.discardedThisTurn = false;
  state.drawAtStartOfNextTurn = false;
  state.drawnThisTurn = false;
}

function autoDrawForPlayer(player) {
  const state = game.cards[player];
  if (state.drawnThisTurn || state.deck.length === 0 || state.hand.length >= 5) return null;
  const drawnCardId = state.deck.pop();
  state.hand.push(drawnCardId);
  state.drawnThisTurn = true;
  state.deckCount = state.deck.length;
  return drawnCardId;
}

function applyMove(move, promotion) {
  const selectedPiece = game.board[move.from.row][move.from.col];
  if (!selectedPiece) return gameResponse(false, "No piece exists on the selected square.");
  const neutral = isNeutralSquare(move.from.square);
  const player = neutral ? game.turn : (isWhitePiece(selectedPiece) ? "White" : "Black");
  if (retryRestriction?.active && retryRestriction.player === player) {
    if (retryRestriction.forbiddenMoveFrom && retryRestriction.forbiddenMoveFrom === move.from.square && retryRestriction.forbiddenMoveTo === move.to.square) {
      return gameResponse(false, `${retryRestriction.cardName || "A card"} prevents repeating the canceled move.`);
    }
  }

  const beforeAction = snapshotGame();
  const fromSquare = move.from.square;
  const toSquare = move.to.square;
  const targetPiece = game.board[move.to.row][move.to.col];
  const promotedSymbols = { Queen: "q", Rook: "r", Bishop: "b", Knight: "n" };
  let placedPiece = selectedPiece;
  if (selectedPiece.toLowerCase() === "p" && (move.to.row === 0 || move.to.row === 7)) {
    const promoted = promotedSymbols[promotion || "Queen"] || "q";
    placedPiece = selectedPiece === selectedPiece.toUpperCase() ? promoted.toUpperCase() : promoted;
  }

  game.board[move.to.row][move.to.col] = placedPiece;
  game.board[move.from.row][move.from.col] = "";
  if (targetPiece) deactivateContinuingEffectsAt(toSquare);
  moveNeutralMarker(fromSquare, toSquare);
  moveWarlordMarker(fromSquare, toSquare);
  const autoDrawnCardId = autoDrawForPlayer(player);
  game.turn = game.turn === "White" ? "Black" : "White";
  startTurnForPlayer(game.turn);
  game.message = autoDrawnCardId
    ? `${player} drew ${autoDrawnCardId} at the end of their turn.`
    : `Mock accepted move: ${move.from.square} to ${move.to.square}`;
  retryRestriction = retryRestriction?.player === player ? null : retryRestriction;
  lastAction = {
    player,
    actionType: "move",
    fromSquare,
    toSquare,
    beforeAction,
  };
  return gameResponse(true);
}

function applyPlayCard(player, cardId, targetSquare, fromSquare, secondTargetSquare, fromSquares = [], promotion = "Queen") {
  const afterOwnTurnCard = cardId === "neutrality";
  if (!afterOwnTurnCard && game.turn !== player) {
    return cardActionResponse(false, "You can only play cards on your own turn.", cardId, targetSquare);
  }
  if (afterOwnTurnCard && (!lastAction || lastAction.player !== player || lastAction.actionType !== "move" || game.turn === player)) {
    return cardActionResponse(false, "That card can only be played after your move.", cardId, targetSquare);
  }
  const state = game.cards[player];
  if (!state.hand.includes(cardId)) {
    return cardActionResponse(false, "That card is not in your hand.", cardId, targetSquare);
  }
  if (state.cardPlayedOnOwnTurn) {
    return cardActionResponse(false, "You have already played your own-turn card this turn.", cardId, targetSquare);
  }
  if (retryRestriction?.active && retryRestriction.player === player && retryRestriction.forbiddenCardId === cardId) {
    return cardActionResponse(false, `${retryRestriction.cardName || "A card"} prevents replaying the canceled card.`, cardId, targetSquare);
  }

  const beforeAction = snapshotGame();

  if (cardId === "disintegrate") {
    const coords = coordinatesForSquare(targetSquare);
    if (!coords) return cardActionResponse(false, "Choose a valid target square.", cardId, targetSquare);
    const targetPiece = game.board[coords.row][coords.col];
    const isFriendlyNonKing =
      targetPiece &&
      targetPiece.toLowerCase() !== "k" &&
      isWhitePiece(targetPiece) === (player === "White");
    if (!isFriendlyNonKing) return cardActionResponse(false, "Disintegrate must target one of your non-king pieces.", cardId, targetSquare);

    game.board[coords.row][coords.col] = "";
    deactivateContinuingEffectsAt(targetSquare);
    state.hand = state.hand.filter((id) => id !== cardId);
    state.discardPile.push(cardId);
    state.cardPlayedOnOwnTurn = true;
    retryRestriction = retryRestriction?.player === player ? null : retryRestriction;
    lastAction = { player, actionType: "card", cardId, beforeAction };
    return cardActionResponse(true, `${player} played Disintegrate on ${targetSquare}.`, cardId, targetSquare);
  }

  if (cardId === "charge") {
    const knightCoords = coordinatesForSquare(fromSquare);
    const midCoords    = coordinatesForSquare(targetSquare);
    const landCoords   = coordinatesForSquare(secondTargetSquare);

    if (!knightCoords) return cardActionResponse(false, "Choose a valid knight square.", cardId, targetSquare);
    if (!midCoords)    return cardActionResponse(false, "Choose a valid intermediate square.", cardId, targetSquare);
    if (!landCoords)   return cardActionResponse(false, "Choose a valid final square.", cardId, targetSquare);

    const expectedKnight = game.board[knightCoords.row][knightCoords.col];
    const neutralKnight = isNeutralSquare(fromSquare) && expectedKnight?.toLowerCase() === "n";
    const friendlyKnight = expectedKnight === (player === "White" ? "N" : "n");
    if (!friendlyKnight && !neutralKnight) {
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
    moveNeutralMarker(fromSquare, secondTargetSquare);

    state.hand = state.hand.filter((id) => id !== cardId);
    state.discardPile.push(cardId);
    const chargeDrawnCardId = autoDrawForPlayer(player);
    game.turn = player === "White" ? "Black" : "White";
    startTurnForPlayer(game.turn);
    retryRestriction = retryRestriction?.player === player ? null : retryRestriction;
    lastAction = { player, actionType: "card", cardId, fromSquare, toSquare: targetSquare, beforeAction };
    const chargeDrawSuffix = chargeDrawnCardId ? ` ${player} drew ${chargeDrawnCardId} at the end of their turn.` : "";
    return cardActionResponse(
      true,
      `${player} played Charge: ${fromSquare} → ${targetSquare} → ${secondTargetSquare}.${chargeDrawSuffix}`,
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
      moveNeutralMarker(squareForCoordinates(move.from.row, move.from.col), squareForCoordinates(move.to.row, move.to.col));
    }

    state.hand = state.hand.filter((id) => id !== cardId);
    state.discardPile.push(cardId);
    const onslaughtDrawnCardId = autoDrawForPlayer(player);
    game.turn = player === "White" ? "Black" : "White";
    startTurnForPlayer(game.turn);
    retryRestriction = retryRestriction?.player === player ? null : retryRestriction;
    lastAction = { player, actionType: "card", cardId, beforeAction };
    const onslaughtDrawSuffix = onslaughtDrawnCardId ? ` ${player} drew ${onslaughtDrawnCardId} at the end of their turn.` : "";
    return cardActionResponse(true, `${player} played Onslaught and moved ${pawnMoves.length} pawn${pawnMoves.length === 1 ? "" : "s"}.${onslaughtDrawSuffix}`, cardId, "");
  }

  if (cardId === "neutrality") {
    const coords = coordinatesForSquare(targetSquare);
    if (!coords) return cardActionResponse(false, "Choose a valid target square.", cardId, targetSquare);
    const targetPiece = game.board[coords.row][coords.col];
    const targetIsNeutral = isNeutralSquare(targetSquare);
    const targetIsEnemy = targetPiece && isWhitePiece(targetPiece) !== (player === "White");
    if (!targetPiece || (!targetIsEnemy && !targetIsNeutral) || ["k", "q"].includes(targetPiece.toLowerCase())) {
      return cardActionResponse(false, "Neutrality must target one opponent non-king, non-queen piece.", cardId, targetSquare);
    }

    markNeutral(targetSquare);
    addContinuingEffect(cardId, player, targetSquare);
    state.hand = state.hand.filter((id) => id !== cardId);
    state.activePile = [...(state.activePile ?? []), cardId];
    state.cardPlayedOnOwnTurn = true;
    lastAction = { player, actionType: "card", cardId, targetSquare, beforeAction };
    return cardActionResponse(true, `${player} played Neutrality on ${targetSquare}.`, cardId, targetSquare);
  }

  if (cardId === "bog") {
    const bogMove = describeBogMove(player);
    if (!bogMove) {
      return cardActionResponse(false, "Bog can only slow an opponent rook, bishop, or queen move of two or more squares.", cardId, "");
    }

    const playerStateAfterCost = cardStateAfterCost(state, cardId);
    const { canceledAction, from, oneStep } = bogMove;

    restoreSnapshot(canceledAction.beforeAction);
    game.cards[player] = playerStateAfterCost;
    game.turn = canceledAction.player;

    const piece = game.board[from.row][from.col];
    game.board[oneStep.row][oneStep.col] = piece;
    game.board[from.row][from.col] = "";
    game.turn = player;
    retryRestriction = null;
    lastAction = null;

    return cardActionResponse(true, `${player} played Bog. ${canceledAction.player}'s ${canceledAction.toSquare} move was slowed to ${oneStep.square}.`, cardId, "");
  }

  if (cardId === "warlord") {
    const coords = coordinatesForSquare(targetSquare);
    if (!coords) return cardActionResponse(false, "Choose a valid target square.", cardId, targetSquare);
    const targetPiece = game.board[coords.row][coords.col];
    const expectedKing = player === "White" ? "K" : "k";
    if (targetPiece !== expectedKing) {
      return cardActionResponse(false, "Warlord must target your King.", cardId, targetSquare);
    }
    if ((game.warlordSquares ?? []).includes(targetSquare)) {
      return cardActionResponse(false, "Your King is already a Warlord.", cardId, targetSquare);
    }

    if (!game.warlordSquares) game.warlordSquares = [];
    game.warlordSquares.push(targetSquare);
    addContinuingEffect(cardId, player, targetSquare);
    state.hand = state.hand.filter((id) => id !== cardId);
    state.activePile = [...(state.activePile ?? []), cardId];
    state.cardPlayedOnOwnTurn = true;
    const warlordDrawnCardId = autoDrawForPlayer(player);
    game.turn = player === "White" ? "Black" : "White";
    startTurnForPlayer(game.turn);
    retryRestriction = retryRestriction?.player === player ? null : retryRestriction;
    lastAction = { player, actionType: "card", cardId, targetSquare, beforeAction };
    const warlordDrawSuffix = warlordDrawnCardId ? ` ${player} drew ${warlordDrawnCardId} at the end of their turn.` : "";
    return cardActionResponse(true, `${player} played Warlord. The ${player} King on ${targetSquare} is now a Warlord!${warlordDrawSuffix}`, cardId, targetSquare);
  }

  if (cardId === "knightmare" || cardId === "think_again") {
    const cardName = cardId === "think_again" ? "Think Again!" : "Knightmare";
    if (!lastAction || lastAction.player === player || game.turn !== player) {
      return cardActionResponse(false, `${cardName} can only cancel the opponent's last action after their turn.`, cardId, "");
    }

    const canceledAction = lastAction;
    const playerStateAfterCost = cardStateAfterCost(state, cardId);

    restoreSnapshot(canceledAction.beforeAction);
    game.cards[player] = playerStateAfterCost;
    game.turn = canceledAction.player;
    retryRestriction = {
      active: true,
      player: canceledAction.player,
      cardName,
      forbiddenMoveFrom: canceledAction.actionType === "move" ? canceledAction.fromSquare : "",
      forbiddenMoveTo: canceledAction.actionType === "move" ? canceledAction.toSquare : "",
      forbiddenCardId: canceledAction.actionType === "card" ? canceledAction.cardId : "",
    };
    lastAction = null;
    return cardActionResponse(true, `${player} played ${cardName}. ${game.turn}'s ${canceledAction.actionType === "card" ? canceledAction.cardId : "move"} was canceled.`, cardId, "");
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

function applyDrawCardAsTurn(player) {
  if (game.turn !== player) {
    return cardActionResponse(false, "You can only draw on your own turn.", "draw", "");
  }
  const state = game.cards[player];
  if (state.drawnThisTurn) {
    return cardActionResponse(false, "You have already drawn a card this turn.", "draw", "");
  }
  if (state.deck.length === 0) {
    return cardActionResponse(false, "Your deck is empty.", "draw", "");
  }
  if (state.hand.length >= 5) {
    return cardActionResponse(false, "Your hand is full (5 cards max).", "draw", "");
  }

  const drawnCardId = state.deck.pop();
  state.hand.push(drawnCardId);
  state.drawnThisTurn = true;
  state.deckCount = state.deck.length;

  const message = `${player} drew ${drawnCardId}.`;
  game.message = message;
  const result = gameResponse(true, message);
  result.cardResult = { success: true, message, cardId: "draw", targetSquare: "", drawnCardId };
  return result;
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

  if (request.method === "POST" && request.url === "/api/claim-checkmate") {
    if (game.pendingCheckmate) {
      const loser = game.pendingCheckmate;
      const winner = loser === "White" ? "Black" : "White";
      game.pendingCheckmate = "";
      game.gameOver = true;
      game.message = `Checkmate! ${winner} wins!`;
    }
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

  if (request.method === "POST" && request.url === "/api/card/draw") {
    try {
      const { player } = await readJson(request);
      sendJson(response, 200, applyDrawCardAsTurn(player));
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
