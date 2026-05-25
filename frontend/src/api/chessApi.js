const API_BASE_URL = (import.meta.env.VITE_CHESS_API_URL || "").replace(/\/$/, "");

function buildMoveUrl(path) {
  return `${API_BASE_URL}${path}`;
}

function applyMoveLocally(moveData, currentBoard) {
  const newBoard = currentBoard.map((row) => [...row]);

  const fromRow = moveData.from.row;
  const fromCol = moveData.from.col;
  const toRow = moveData.to.row;
  const toCol = moveData.to.col;
  const selectedPiece = newBoard[fromRow][fromCol];

  if (!selectedPiece) {
    return {
      valid: false,
      board: currentBoard,
      message: "No piece exists on the selected square.",
    };
  }

  newBoard[toRow][toCol] = selectedPiece;
  newBoard[fromRow][fromCol] = "";

  return {
    valid: true,
    board: newBoard,
    message: `Move accepted: ${moveData.from.square} to ${moveData.to.square}`,
  };
}

function normalizeMoveResponse(response, fallbackBoard) {
  return {
    valid: Boolean(response.valid),
    board: Array.isArray(response.board) ? response.board : fallbackBoard,
    message: response.message || (response.valid ? "Move accepted." : "Invalid move."),
  };
}

export async function makeMove(moveData, currentBoard) {
  const payload = {
    move: moveData,
    board: currentBoard,
  };

  try {
    const response = await fetch(buildMoveUrl("/api/move"), {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(payload),
    });

    if (!response.ok) {
      throw new Error(`Backend responded with HTTP ${response.status}`);
    }

    const result = await response.json();
    return normalizeMoveResponse(result, currentBoard);
  } catch (error) {
    console.warn("Backend unavailable, using local move fallback:", error);

    await new Promise((resolve) => setTimeout(resolve, 200));
    return {
      ...applyMoveLocally(moveData, currentBoard),
      source: "local-fallback",
    };
  }
}
