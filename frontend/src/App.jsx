import { useEffect, useRef, useState } from "react";
import { getGameState, getLegalMoves, makeMove, resetGame } from "./api/chessApi";
import "./App.css";

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

const pieceSymbols = {
  K: "\u2654",
  Q: "\u2655",
  R: "\u2656",
  B: "\u2657",
  N: "\u2658",
  P: "\u2659",
  k: "\u265a",
  q: "\u265b",
  r: "\u265c",
  b: "\u265d",
  n: "\u265e",
  p: "\u265f",
};

const promotionPieces = ["Queen", "Rook", "Bishop", "Knight"];

const backendLabels = {
  checking: "Connecting",
  connected: "Backend connected",
  reconnecting: "Reconnecting",
  offline: "Backend offline",
};

function isWhitePiece(piece) {
  return piece !== "" && piece === piece.toUpperCase();
}

function areSameSide(pieceA, pieceB) {
  if (!pieceA || !pieceB) {
    return false;
  }

  return isWhitePiece(pieceA) === isWhitePiece(pieceB);
}

function isSameSquare(square, row, col) {
  return square?.row === row && square?.col === col;
}

function convertToChessSquare(row, col) {
  const files = ["a", "b", "c", "d", "e", "f", "g", "h"];
  return `${files[col]}${8 - row}`;
}

function needsPromotion(piece, destinationRow) {
  return piece.toLowerCase() === "p" && (destinationRow === 0 || destinationRow === 7);
}

function promotionSymbol(pieceName, isWhite) {
  const symbol = pieceName === "Knight" ? "N" : pieceName[0];
  return isWhite ? symbol : symbol.toLowerCase();
}

function App() {
  const [board, setBoard] = useState(startingBoard);
  const [currentTurn, setCurrentTurn] = useState("White");
  const [selectedSquare, setSelectedSquare] = useState(null);
  const [message, setMessage] = useState("Connecting to the chess backend...");
  const [isThinking, setIsThinking] = useState(false);
  const [backendStatus, setBackendStatus] = useState("checking");
  const [lastMove, setLastMove] = useState(null);
  const [gameOver, setGameOver] = useState(false);
  const [pendingPromotion, setPendingPromotion] = useState(null);
  const [legalMoves, setLegalMoves] = useState([]);
  const [isLoadingMoves, setIsLoadingMoves] = useState(false);
  const failedPollCount = useRef(0);
  const selectionRequestId = useRef(0);
  const busyRef = useRef(false);

  useEffect(() => {
    let isMounted = true;

    async function synchronize(showMessage) {
      if (busyRef.current) {
        return;
      }

      try {
        const game = await getGameState(startingBoard);

        if (!isMounted) {
          return;
        }

        setBoard(game.board);
        setCurrentTurn(game.turn);
        setGameOver(game.gameOver);
        setBackendStatus("connected");
        failedPollCount.current = 0;

        if (showMessage) {
          setMessage(game.message);
        }
      } catch {
        if (!isMounted) {
          return;
        }

        failedPollCount.current += 1;
        setBackendStatus(showMessage || failedPollCount.current > 1 ? "offline" : "reconnecting");

        if (showMessage) {
          setMessage("Backend unavailable. Start the C++ server to play.");
        }
      }
    }

    synchronize(true);
    const intervalId = window.setInterval(() => synchronize(false), 8000);

    return () => {
      isMounted = false;
      window.clearInterval(intervalId);
    };
  }, []);

  function clearSelection() {
    selectionRequestId.current += 1;
    setSelectedSquare(null);
    setLegalMoves([]);
    setIsLoadingMoves(false);
  }

  async function selectPiece(rowIndex, colIndex, piece) {
    const square = convertToChessSquare(rowIndex, colIndex);
    const requestId = selectionRequestId.current + 1;
    selectionRequestId.current = requestId;
    setSelectedSquare({ row: rowIndex, col: colIndex });
    setLegalMoves([]);
    setIsLoadingMoves(true);
    setMessage(`Checking moves for ${pieceSymbols[piece]} on ${square}...`);

    try {
      const moves = await getLegalMoves(square);
      if (selectionRequestId.current !== requestId) {
        return;
      }

      failedPollCount.current = 0;
      setBackendStatus("connected");
      setLegalMoves(moves);
      setMessage(`Selected ${pieceSymbols[piece]} on ${square}. ${moves.length} legal move${moves.length === 1 ? "" : "s"}.`);
    } catch {
      if (selectionRequestId.current !== requestId) {
        return;
      }

      setBackendStatus("reconnecting");
      setMessage("Could not load legal moves. Checking the backend connection...");
    } finally {
      if (selectionRequestId.current === requestId) {
        setIsLoadingMoves(false);
      }
    }
  }

  async function submitMove(moveData, promotion = "") {
    setMessage(`Sending move ${moveData.from.square} to ${moveData.to.square}...`);
    setIsThinking(true);
    busyRef.current = true;
    clearSelection();

    try {
      const result = await makeMove(moveData, promotion, board);

      failedPollCount.current = 0;
      setBackendStatus("connected");
      setBoard(result.board);
      setCurrentTurn(result.turn);
      setGameOver(result.gameOver);
      setMessage(result.message);

      if (result.valid) {
        setLastMove({
          from: { row: moveData.from.row, col: moveData.from.col },
          to: { row: moveData.to.row, col: moveData.to.col },
        });
      }
    } catch (error) {
      setBackendStatus("offline");
      setMessage(error.message || "Backend request failed.");
    } finally {
      setIsThinking(false);
      busyRef.current = false;
    }
  }

  async function handleSquareClick(rowIndex, colIndex) {
    if (isThinking || pendingPromotion) {
      return;
    }

    if (gameOver) {
      setMessage("This game is over. Start a new game to play again.");
      return;
    }

    const clickedPiece = board[rowIndex][colIndex];

    if (selectedSquare === null) {
      if (clickedPiece === "") {
        setMessage("That square is empty. Select one of your pieces.");
        return;
      }

      if (
        (currentTurn === "White" && !isWhitePiece(clickedPiece)) ||
        (currentTurn === "Black" && isWhitePiece(clickedPiece))
      ) {
        setMessage(`It is ${currentTurn}'s turn.`);
        return;
      }

      await selectPiece(rowIndex, colIndex, clickedPiece);
      return;
    }

    const fromSquare = convertToChessSquare(selectedSquare.row, selectedSquare.col);
    const toSquare = convertToChessSquare(rowIndex, colIndex);
    const selectedPiece = board[selectedSquare.row][selectedSquare.col];

    if (isSameSquare(selectedSquare, rowIndex, colIndex)) {
      clearSelection();
      setMessage(`Cleared selection on ${fromSquare}.`);
      return;
    }

    if (areSameSide(selectedPiece, clickedPiece)) {
      await selectPiece(rowIndex, colIndex, clickedPiece);
      return;
    }

    if (isLoadingMoves) {
      setMessage("Still checking legal moves for the selected piece...");
      return;
    }

    if (!legalMoves.includes(toSquare)) {
      setMessage(`${toSquare} is not a legal destination for the selected piece.`);
      return;
    }

    const moveData = {
      from: {
        row: selectedSquare.row,
        col: selectedSquare.col,
        square: fromSquare,
      },
      to: {
        row: rowIndex,
        col: colIndex,
        square: toSquare,
      },
    };

    if (needsPromotion(selectedPiece, rowIndex)) {
      clearSelection();
      setPendingPromotion(moveData);
      setMessage(`Choose a piece for the pawn on ${toSquare}.`);
      return;
    }

    await submitMove(moveData);
  }

  async function handlePromotionChoice(pieceName) {
    const moveData = pendingPromotion;
    setPendingPromotion(null);
    await submitMove(moveData, pieceName);
  }

  function cancelPromotion() {
    setPendingPromotion(null);
    setMessage("Select a piece.");
  }

  async function handleNewGame() {
    setIsThinking(true);
    busyRef.current = true;
    setPendingPromotion(null);
    clearSelection();

    try {
      const result = await resetGame(startingBoard);
      setBoard(result.board);
      setCurrentTurn(result.turn);
      setGameOver(false);
      setLastMove(null);
      setMessage(result.message);
      setBackendStatus("connected");
      failedPollCount.current = 0;
    } catch (error) {
      setBackendStatus("offline");
      setMessage(error.message || "Could not start a new game.");
    } finally {
      setIsThinking(false);
      busyRef.current = false;
    }
  }

  return (
    <div className="app">
      <header className="app-header">
        <div className="header-info">
          <div className="app-title">
            <span className="chess-icon" aria-hidden="true">♟</span>
            <h1>Chess</h1>
          </div>
          <div className="turn-row">
            {gameOver ? (
              <span className="game-over-badge">Game Over</span>
            ) : (
              <span className="turn-badge">
                <span className={`turn-dot ${currentTurn.toLowerCase()}`} aria-hidden="true" />
                {currentTurn} to move
              </span>
            )}
          </div>
          <p className="status-message">{message}</p>
        </div>
        <div className="game-tools">
          <button
            type="button"
            className="new-game"
            onClick={handleNewGame}
            disabled={isThinking}
          >
            <span className="new-game-icon" aria-hidden="true">↺</span>
            New Game
          </button>
          <span className={`backend-status ${backendStatus}`}>
            {backendLabels[backendStatus]}
          </span>
        </div>
      </header>

      <div className={`board ${isThinking ? "thinking" : ""}`} aria-label="Chess board">
        {board.map((row, rowIndex) =>
          row.map((piece, colIndex) => {
            const isLightSquare = (rowIndex + colIndex) % 2 === 0;
            const isSelected = isSameSquare(selectedSquare, rowIndex, colIndex);
            const isLastMove =
              isSameSquare(lastMove?.from, rowIndex, colIndex) ||
              isSameSquare(lastMove?.to, rowIndex, colIndex);
            const squareName = convertToChessSquare(rowIndex, colIndex);
            const isLegalTarget = legalMoves.includes(squareName);
            const isCaptureTarget = isLegalTarget && piece !== "";

            return (
              <button
                type="button"
                key={`${rowIndex}-${colIndex}`}
                onClick={() => handleSquareClick(rowIndex, colIndex)}
                className={`square ${isLightSquare ? "light" : "dark"} ${
                  isSelected ? "selected" : ""
                } ${isLastMove ? "last-move" : ""} ${
                  isLegalTarget ? (isCaptureTarget ? "capture-target" : "legal-target") : ""
                }`}
                aria-label={`${squareName}${piece ? ` ${piece}` : ""}${
                  isLegalTarget ? " legal destination" : ""
                }`}
                disabled={isThinking || Boolean(pendingPromotion)}
              >
                <span
                  className={`piece ${
                    piece ? (isWhitePiece(piece) ? "white-piece" : "black-piece") : "empty"
                  }`}
                >
                  {pieceSymbols[piece] ?? ""}
                </span>
                {(rowIndex === 7 || colIndex === 0) && (
                  <span className="coordinate" aria-hidden="true">
                    {colIndex === 0 ? 8 - rowIndex : ""}
                    {rowIndex === 7 ? squareName[0] : ""}
                  </span>
                )}
              </button>
            );
          })
        )}
      </div>

      {pendingPromotion && (
        <div className="promotion-backdrop">
          <section className="promotion-dialog" role="dialog" aria-modal="true" aria-label="Promote pawn">
            <h2>Promote pawn</h2>
            <div className="promotion-options">
              {promotionPieces.map((pieceName) => {
                const symbol = promotionSymbol(pieceName, currentTurn === "White");

                return (
                  <button
                    type="button"
                    className="promotion-option"
                    key={pieceName}
                    onClick={() => handlePromotionChoice(pieceName)}
                  >
                    <span className="promotion-piece" aria-hidden="true">
                      {pieceSymbols[symbol]}
                    </span>
                    <span>{pieceName}</span>
                  </button>
                );
              })}
            </div>
            <button
              type="button"
              className="cancel-promotion"
              onClick={cancelPromotion}
            >
              Cancel
            </button>
          </section>
        </div>
      )}
    </div>
  );
}

export default App;
