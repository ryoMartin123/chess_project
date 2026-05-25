const API_BASE_URL = (import.meta.env.VITE_CHESS_API_URL || "").replace(/\/$/, "");
const REQUEST_TIMEOUT_MS = Number(import.meta.env.VITE_API_TIMEOUT_MS || 5000);

function buildMoveUrl(path) {
  return `${API_BASE_URL}${path}`;
}

async function requestJson(path, options = {}) {
  const controller = new AbortController();
  const timeoutId = window.setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);

  try {
    const response = await fetch(buildMoveUrl(path), {
      ...options,
      signal: controller.signal,
      headers: {
        "Content-Type": "application/json",
        ...options.headers,
      },
    });

    if (!response.ok) {
      throw new Error(`Backend responded with HTTP ${response.status}`);
    }

    return response.json();
  } finally {
    window.clearTimeout(timeoutId);
  }
}

function normalizeGameResponse(response, fallbackBoard = []) {
  return {
    valid: Boolean(response.valid),
    board: Array.isArray(response.board) ? response.board : fallbackBoard,
    turn: response.turn || "White",
    gameOver: Boolean(response.gameOver),
    message: response.message || (response.valid ? "Move accepted." : "Invalid move."),
  };
}

export async function makeMove(moveData, promotion, currentBoard) {
  const payload = {
    move: moveData,
  };

  if (promotion) {
    payload.promotion = promotion;
  }

  const result = await requestJson("/api/move", {
    method: "POST",
    body: JSON.stringify(payload),
  });

  return {
    ...normalizeGameResponse(result, currentBoard),
    source: "backend",
  };
}

export async function getGameState(currentBoard) {
  const result = await requestJson("/api/state");
  return normalizeGameResponse(result, currentBoard);
}

export async function resetGame(currentBoard) {
  const result = await requestJson("/api/reset", {
    method: "POST",
  });
  return normalizeGameResponse(result, currentBoard);
}

export async function getLegalMoves(square) {
  const result = await requestJson("/api/legal-moves", {
    method: "POST",
    body: JSON.stringify({ square }),
  });

  return Array.isArray(result.moves) ? result.moves : [];
}

export async function getBackendHealth() {
  const result = await requestJson("/api/health");

  return {
    ok: Boolean(result.ok),
    service: result.service || "backend",
  };
}
