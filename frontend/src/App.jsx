import { useState } from "react";
import { makeMove } from "./api/chessApi";
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

function convertToChessSquare(row, col) {
  const files = ["a", "b", "c", "d", "e", "f", "g", "h"];
  const file = files[col];
  const rank = 8 - row;

  return `${file}${rank}`;
}

function App() {
  const [board, setBoard] = useState(startingBoard);
  const [selectedSquare, setSelectedSquare] = useState(null);
  const [message, setMessage] = useState("Click a piece to begin.");
  const [isThinking, setIsThinking] = useState(false);

  async function handleSquareClick(rowIndex, colIndex) {
    if (isThinking) {
      return;
    }

    const clickedPiece = board[rowIndex][colIndex];

    if (selectedSquare === null) {
      if (clickedPiece === "") {
        setMessage("That square is empty. Click a piece.");
        return;
      }

      const selectedChessSquare = convertToChessSquare(rowIndex, colIndex);

      setSelectedSquare({ row: rowIndex, col: colIndex });
      setMessage(`Selected ${pieceSymbols[clickedPiece]} on ${selectedChessSquare}`);
      return;
    }

    const fromSquare = convertToChessSquare(selectedSquare.row, selectedSquare.col);
    const toSquare = convertToChessSquare(rowIndex, colIndex);

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

    setMessage(`Sending move ${fromSquare} to ${toSquare} to backend...`);
    setIsThinking(true);

    try {
      const result = await makeMove(moveData, board);

      if (result.valid) {
        setBoard(result.board);
        setMessage(result.message);
      } else {
        setMessage(result.message || "Invalid move.");
      }
    } catch (error) {
      setMessage(error.message || "Backend request failed.");
    } finally {
      setIsThinking(false);
      setSelectedSquare(null);
    }
  }

  return (
    <div className="app">
      <h1>Chess Frontend</h1>
      <p>{message}</p>

      <div className="board">
        {board.map((row, rowIndex) =>
          row.map((piece, colIndex) => {
            const isLightSquare = (rowIndex + colIndex) % 2 === 0;
            const isSelected =
              selectedSquare &&
              selectedSquare.row === rowIndex &&
              selectedSquare.col === colIndex;

            return (
              <div
                key={`${rowIndex}-${colIndex}`}
                onClick={() => handleSquareClick(rowIndex, colIndex)}
                className={`square ${isLightSquare ? "light" : "dark"} ${
                  isSelected ? "selected" : ""
                }`}
              >
                {pieceSymbols[piece] ?? ""}
              </div>
            );
          })
        )}
      </div>
    </div>
  );
}

export default App;
