import { useEffect, useLayoutEffect, useRef, useState } from "react";
import {
  getGameState, getLegalMoves, makeMove, resetGame,
  getCardDefinitions, playCard, discardCard,
} from "./api/chessApi";
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
  K: "♔", Q: "♕", R: "♖", B: "♗", N: "♘", P: "♙",
  k: "♚", q: "♛", r: "♜", b: "♝", n: "♞", p: "♟",
};

const promotionPieces = ["Queen", "Rook", "Bishop", "Knight"];

const backendLabels = {
  checking: "Connecting",
  connected: "Backend connected",
  reconnecting: "Reconnecting",
  offline: "Backend offline",
};

// Fallback definitions so cards render even before the API responds
const FALLBACK_CARD_DEFS = {
  destroy_pawn: { id: "destroy_pawn", name: "Destroy Pawn", level: 1, timing: "AS_OWN_TURN", targetRequirement: "ENEMY_PAWN",      effect: "DESTROY_TARGET", countsAsOwnTurnCard: true },
  charge:       { id: "charge",       name: "Charge",       level: 6, timing: "AS_OWN_TURN", targetRequirement: "FRIENDLY_KNIGHT", effect: "CHARGE_KNIGHT",  countsAsOwnTurnCard: true },
  onslaught:    { id: "onslaught",    name: "Onslaught",    level: 6, timing: "AS_OWN_TURN", targetRequirement: "FRIENDLY_PAWNS",  effect: "MOVE_PAWNS",     countsAsOwnTurnCard: true },
};

function cardDescription(def) {
  if (!def) return "";
  if (def.effect === "DESTROY_TARGET" && def.targetRequirement === "ENEMY_PAWN") {
    return "Remove one enemy pawn from the board.";
  }
  if (def.effect === "CHARGE_KNIGHT") {
    return "Move a knight twice. First hop must land on an empty square.";
  }
  if (def.effect === "MOVE_PAWNS") {
    return "Move any number of your pawns one square forward.";
  }
  return def.name;
}

function cardTimingLabel(timing) {
  const labels = {
    AS_OWN_TURN:          "Uses your turn",
    ON_OWN_TURN:          "Uses your turn",
    BEFORE_OWN_TURN:      "Before move",
    AFTER_OWN_TURN:       "After move",
    AFTER_OPPONENT_TURN:  "After opponent",
    ANYTIME:              "Anytime",
  };
  return labels[timing] || timing;
}

function CardArt({ def }) {
  if (!def) return null;
  if (def.effect === "DESTROY_TARGET" && def.targetRequirement === "ENEMY_PAWN") {
    return (
      <div className="card-art">
        <span className="card-art-piece">♟</span>
        <span className="card-art-x" aria-hidden="true">✕</span>
      </div>
    );
  }
  if (def.effect === "CHARGE_KNIGHT") {
    return (
      <div className="card-art">
        <span className="card-art-piece card-art-knight">♞</span>
        <span className="card-art-charge" aria-hidden="true">»»</span>
      </div>
    );
  }
  if (def.effect === "MOVE_PAWNS") {
    return (
      <div className="card-art">
        <span className="card-art-piece card-art-pawns">♟♟</span>
        <span className="card-art-charge" aria-hidden="true">↑</span>
      </div>
    );
  }
  return <div className="card-art"><span className="card-art-piece">?</span></div>;
}

const KNIGHT_DELTAS = [[-2,-1],[-2,1],[-1,-2],[-1,2],[1,-2],[1,2],[2,-1],[2,1]];

function isWhitePiece(piece) {
  return piece !== "" && piece === piece.toUpperCase();
}

function areSameSide(pieceA, pieceB) {
  if (!pieceA || !pieceB) return false;
  return isWhitePiece(pieceA) === isWhitePiece(pieceB);
}

function isFriendlyPawn(piece, player) {
  return (player === "White" && piece === "P") || (player === "Black" && piece === "p");
}

function forwardRowForPawn(row, player) {
  return player === "White" ? row - 1 : row + 1;
}

function canOnslaughtSelectPawn(board, row, col, player) {
  const piece = board[row][col];
  if (!isFriendlyPawn(piece, player)) return false;
  const destinationRow = forwardRowForPawn(row, player);
  return destinationRow >= 0 && destinationRow < 8 && board[destinationRow][col] === "";
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
  const [pendingCardPromotion, setPendingCardPromotion] = useState(null);
  const [legalMoves, setLegalMoves] = useState([]);
  const [isLoadingMoves, setIsLoadingMoves] = useState(false);
  const [activeDrag, setActiveDrag] = useState(null);
  // Card system state
  const [cardState, setCardState] = useState(null);
  const [cardDefinitions, setCardDefinitions] = useState([]);
  // targeting: null | { cardId, step, highlights[], selectedSquares?, fromSquare?, fromRow?, fromCol?, midSquare?, midRow?, midCol? }
  const [targeting, setTargeting] = useState(null);

  const failedPollCount = useRef(0);
  const selectionRequestId = useRef(0);
  const busyRef = useRef(false);
  const dragStateRef = useRef(null);
  const ghostRef = useRef(null);
  const legalMovesRef = useRef([]);
  const isLoadingMovesRef = useRef(false);
  const suppressClickRef = useRef(false);
  const ghostInitialPosRef = useRef(null);

  useEffect(() => { legalMovesRef.current = legalMoves; }, [legalMoves]);
  useEffect(() => { isLoadingMovesRef.current = isLoadingMoves; }, [isLoadingMoves]);

  useLayoutEffect(() => {
    if (!activeDrag || !ghostRef.current || !ghostInitialPosRef.current) return;
    const { x, y, squareSize } = ghostInitialPosRef.current;
    ghostRef.current.style.left = `${x - squareSize / 2}px`;
    ghostRef.current.style.top  = `${y - squareSize / 2}px`;
  }, [activeDrag]);

  // Load card definitions once; fetch game state on mount and poll
  useEffect(() => {
    let isMounted = true;

    async function synchronize(showMessage) {
      if (busyRef.current) return;
      try {
        const [game, defs] = await Promise.all([getGameState(startingBoard), getCardDefinitions()]);
        if (!isMounted) return;
        setBoard(game.board);
        setCurrentTurn(game.turn);
        setGameOver(game.gameOver);
        setBackendStatus("connected");
        if (game.cards) setCardState(game.cards);
        if (defs.length > 0) setCardDefinitions(defs);
        failedPollCount.current = 0;
        if (showMessage) setMessage(game.message);
      } catch {
        if (!isMounted) return;
        failedPollCount.current += 1;
        setBackendStatus(showMessage || failedPollCount.current > 1 ? "offline" : "reconnecting");
        if (showMessage) setMessage("Backend unavailable. Start the C++ server to play.");
      }
    }

    synchronize(true);
    const intervalId = window.setInterval(() => synchronize(false), 8000);
    return () => { isMounted = false; window.clearInterval(intervalId); };
  }, []);

  function applyGameResponse(result) {
    setBoard(result.board);
    setCurrentTurn(result.turn);
    setGameOver(result.gameOver);
    setMessage(result.message);
    if (result.cards) setCardState(result.cards);
  }

  // ── Selection / move ─────────────────────────────────────────────────────

  function clearSelection() {
    selectionRequestId.current += 1;
    setSelectedSquare(null);
    setLegalMoves([]);
    legalMovesRef.current = [];
    setIsLoadingMoves(false);
    isLoadingMovesRef.current = false;
  }

  async function selectPiece(rowIndex, colIndex, piece) {
    const square = convertToChessSquare(rowIndex, colIndex);
    const requestId = selectionRequestId.current + 1;
    selectionRequestId.current = requestId;
    setSelectedSquare({ row: rowIndex, col: colIndex });
    setLegalMoves([]);
    legalMovesRef.current = [];
    setIsLoadingMoves(true);
    isLoadingMovesRef.current = true;
    setMessage(`Checking moves for ${pieceSymbols[piece]} on ${square}...`);

    try {
      const moves = await getLegalMoves(square);
      if (selectionRequestId.current !== requestId) return;
      failedPollCount.current = 0;
      setBackendStatus("connected");
      setLegalMoves(moves);
      legalMovesRef.current = moves;
      setMessage(`Selected ${pieceSymbols[piece]} on ${square}. ${moves.length} legal move${moves.length === 1 ? "" : "s"}.`);
    } catch {
      if (selectionRequestId.current !== requestId) return;
      setBackendStatus("reconnecting");
      setMessage("Could not load legal moves. Checking the backend connection...");
    } finally {
      if (selectionRequestId.current === requestId) {
        setIsLoadingMoves(false);
        isLoadingMovesRef.current = false;
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
      applyGameResponse(result);
      if (result.valid) {
        setLastMove({
          from: { row: moveData.from.row, col: moveData.from.col },
          to:   { row: moveData.to.row,   col: moveData.to.col   },
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

  // ── Card actions ──────────────────────────────────────────────────────────

  async function executePlayCard(cardId, squares) {
    setTargeting(null);
    clearSelection();
    setIsThinking(true);
    busyRef.current = true;

    try {
      const result = await playCard(currentTurn, cardId, squares);
      failedPollCount.current = 0;
      setBackendStatus("connected");
      applyGameResponse(result);
    } catch (error) {
      setBackendStatus("offline");
      setMessage(error.message || "Card play failed.");
    } finally {
      setIsThinking(false);
      busyRef.current = false;
    }
  }

  async function executeDiscardCard(player, cardId) {
    setIsThinking(true);
    busyRef.current = true;

    try {
      const result = await discardCard(player, cardId);
      failedPollCount.current = 0;
      setBackendStatus("connected");
      applyGameResponse(result);
    } catch (error) {
      setBackendStatus("offline");
      setMessage(error.message || "Discard failed.");
    } finally {
      setIsThinking(false);
      busyRef.current = false;
    }
  }

  function startTargeting(def) {
    clearSelection();
    if (def.targetRequirement === "ENEMY_PAWN") {
      setTargeting({ cardId: def.id, step: 1, highlights: [] });
      setMessage("Select an enemy pawn to destroy.");
    } else if (def.targetRequirement === "FRIENDLY_KNIGHT") {
      setTargeting({ cardId: def.id, step: 1, highlights: [] });
      setMessage("Select one of your knights to charge with.");
    } else if (def.targetRequirement === "FRIENDLY_PAWNS") {
      setTargeting({ cardId: def.id, step: 1, highlights: [], selectedSquares: [] });
      setMessage("Select any pawns you want to move one square forward, then press Play Selected.");
    }
  }

  function cancelTargeting() {
    setTargeting(null);
    setMessage("Select a piece or play a card.");
  }

  async function confirmOnslaughtSelection() {
    if (!targeting || targeting.cardId !== "onslaught") return;
    const fromSquares = targeting.selectedSquares ?? [];
    if (fromSquares.length === 0) {
      setMessage("Select at least one pawn for Onslaught.");
      return;
    }

    const hasPromotion = fromSquares.some((square) => {
      const col = square.charCodeAt(0) - "a".charCodeAt(0);
      const row = 8 - Number(square[1]);
      const destinationRow = forwardRowForPawn(row, currentTurn);
      const piece = board[row]?.[col] ?? "";
      return needsPromotion(piece, destinationRow);
    });

    if (hasPromotion) {
      setPendingCardPromotion({ cardId: "onslaught", fromSquares });
      setMessage("Choose a promotion piece for Onslaught.");
      return;
    }

    await executePlayCard("onslaught", { fromSquares, promotion: "Queen" });
  }

  // ── Click handler ─────────────────────────────────────────────────────────

  async function handleSquareClick(rowIndex, colIndex) {
    if (suppressClickRef.current) { suppressClickRef.current = false; return; }
    if (isThinking || pendingPromotion || pendingCardPromotion) return;

    const squareName = convertToChessSquare(rowIndex, colIndex);
    const piece = board[rowIndex][colIndex];

    // Targeting mode
    if (targeting) {
      const { cardId, step, highlights } = targeting;

      if (cardId === "destroy_pawn") {
        const isEnemyPawn = (currentTurn === "White" && piece === "p") || (currentTurn === "Black" && piece === "P");
        if (isEnemyPawn) {
          await executePlayCard(cardId, { targetSquare: squareName });
        } else {
          setMessage("Select an enemy pawn to destroy, or press Cancel.");
        }
        return;
      }

      if (cardId === "charge") {
        if (step === 1) {
          const isFriendlyKnight = (currentTurn === "White" && piece === "N") || (currentTurn === "Black" && piece === "n");
          if (!isFriendlyKnight) { setMessage("Select one of your knights to charge with."); return; }

          // First hop: knight-reachable empty squares only (no capture allowed)
          const emptyHops = [];
          for (const [dr, dc] of KNIGHT_DELTAS) {
            const r = rowIndex + dr, c = colIndex + dc;
            if (r >= 0 && r < 8 && c >= 0 && c < 8 && board[r][c] === "") {
              emptyHops.push(convertToChessSquare(r, c));
            }
          }
          setTargeting({ cardId, step: 2, highlights: emptyHops, fromSquare: squareName, fromRow: rowIndex, fromCol: colIndex });
          setMessage(`Knight at ${squareName} selected. Choose the first hop — must land on an empty square.`);
          return;
        }

        if (step === 2) {
          if (!highlights.includes(squareName)) { setMessage("That square isn't a valid first hop. Choose an empty square the knight can reach."); return; }
          // Compute knight-reachable squares from this intermediate position (can capture on 2nd hop)
          const validLandings = [];
          for (const [dr, dc] of KNIGHT_DELTAS) {
            const r = rowIndex + dr, c = colIndex + dc;
            if (r >= 0 && r < 8 && c >= 0 && c < 8) {
              const p = board[r][c];
              const isOwn = p && ((currentTurn === "White" && isWhitePiece(p)) || (currentTurn === "Black" && !isWhitePiece(p)));
              if (!isOwn) validLandings.push(convertToChessSquare(r, c));
            }
          }
          setTargeting({ ...targeting, step: 3, highlights: validLandings, midSquare: squareName, midRow: rowIndex, midCol: colIndex });
          setMessage(`First hop: ${squareName}. Now choose the final landing square.`);
          return;
        }

        if (step === 3) {
          if (!highlights.includes(squareName)) { setMessage("That square isn't a valid landing. Choose a square the knight can reach from the hop."); return; }
          await executePlayCard(cardId, {
            fromSquare:         targeting.fromSquare,
            targetSquare:       targeting.midSquare,
            secondTargetSquare: squareName,
          });
          return;
        }
      }

      if (cardId === "onslaught") {
        const selectedSquares = targeting.selectedSquares ?? [];
        const alreadySelected = selectedSquares.includes(squareName);
        if (alreadySelected) {
          const nextSelected = selectedSquares.filter((square) => square !== squareName);
          setTargeting({ ...targeting, selectedSquares: nextSelected });
          setMessage(nextSelected.length === 0
            ? "No pawns selected for Onslaught yet."
            : `${nextSelected.length} pawn${nextSelected.length === 1 ? "" : "s"} selected for Onslaught.`);
          return;
        }

        if (!canOnslaughtSelectPawn(board, rowIndex, colIndex, currentTurn)) {
          setMessage("Select one of your pawns that can move one empty square forward.");
          return;
        }

        const nextSelected = [...selectedSquares, squareName];
        setTargeting({ ...targeting, selectedSquares: nextSelected });
        setMessage(`${nextSelected.length} pawn${nextSelected.length === 1 ? "" : "s"} selected for Onslaught. Press Play Selected when ready.`);
        return;
      }
      return;
    }

    if (gameOver) { setMessage("This game is over. Start a new game to play again."); return; }

    if (selectedSquare === null) {
      if (piece === "") { setMessage("That square is empty. Select one of your pieces."); return; }
      if (
        (currentTurn === "White" && !isWhitePiece(piece)) ||
        (currentTurn === "Black" &&  isWhitePiece(piece))
      ) { setMessage(`It is ${currentTurn}'s turn.`); return; }
      await selectPiece(rowIndex, colIndex, piece);
      return;
    }

    const fromSquare    = convertToChessSquare(selectedSquare.row, selectedSquare.col);
    const toSquare      = squareName;
    const selectedPiece = board[selectedSquare.row][selectedSquare.col];

    if (isSameSquare(selectedSquare, rowIndex, colIndex)) {
      clearSelection(); setMessage(`Cleared selection on ${fromSquare}.`); return;
    }
    if (areSameSide(selectedPiece, piece)) {
      await selectPiece(rowIndex, colIndex, piece); return;
    }
    if (isLoadingMoves) { setMessage("Still checking legal moves for the selected piece..."); return; }
    if (!legalMoves.includes(toSquare)) { setMessage(`${toSquare} is not a legal destination for the selected piece.`); return; }

    const moveData = {
      from: { row: selectedSquare.row, col: selectedSquare.col, square: fromSquare },
      to:   { row: rowIndex,           col: colIndex,           square: toSquare   },
    };

    if (needsPromotion(selectedPiece, rowIndex)) {
      clearSelection(); setPendingPromotion(moveData);
      setMessage(`Choose a piece for the pawn on ${toSquare}.`);
      return;
    }

    await submitMove(moveData);
  }

  // ── Drag handlers ─────────────────────────────────────────────────────────

  function handlePointerDown(e, rowIndex, colIndex) {
    if (e.button !== 0) return;
    if (isThinking || pendingPromotion || pendingCardPromotion || gameOver || targeting) return;

    const piece = board[rowIndex][colIndex];
    if (!piece) return;
    if (
      (currentTurn === "White" && !isWhitePiece(piece)) ||
      (currentTurn === "Black" &&  isWhitePiece(piece))
    ) return;

    const rect = e.currentTarget.getBoundingClientRect();
    dragStateRef.current = {
      fromRow: rowIndex, fromCol: colIndex, piece,
      startX: e.clientX, startY: e.clientY,
      squareCenterX: rect.left + rect.width / 2,
      squareCenterY: rect.top  + rect.height / 2,
      squareSize: rect.width,
      isDragging: false,
    };
    e.currentTarget.setPointerCapture(e.pointerId);
  }

  function handlePointerMove(e, rowIndex, colIndex) {
    const ds = dragStateRef.current;
    if (!ds || ds.fromRow !== rowIndex || ds.fromCol !== colIndex) return;

    const dx = e.clientX - ds.startX;
    const dy = e.clientY - ds.startY;

    if (!ds.isDragging && Math.hypot(dx, dy) > 6) {
      ds.isDragging = true;
      suppressClickRef.current = true;
      ghostInitialPosRef.current = { x: e.clientX, y: e.clientY, squareSize: ds.squareSize };
      setActiveDrag({ fromRow: rowIndex, fromCol: colIndex, piece: ds.piece, squareSize: ds.squareSize });
      selectPiece(rowIndex, colIndex, ds.piece);
    }

    if (ds.isDragging && ghostRef.current) {
      ghostRef.current.style.left = `${e.clientX - ds.squareSize / 2}px`;
      ghostRef.current.style.top  = `${e.clientY - ds.squareSize / 2}px`;
    }
  }

  async function handlePointerUp(e, rowIndex, colIndex) {
    const ds = dragStateRef.current;
    if (!ds || ds.fromRow !== rowIndex || ds.fromCol !== colIndex) return;

    if (!ds.isDragging) { dragStateRef.current = null; return; }

    dragStateRef.current = null;

    const elements = document.elementsFromPoint(e.clientX, e.clientY);
    let targetRow = -1, targetCol = -1;
    for (const el of elements) {
      if (el.dataset?.row !== undefined && el.dataset?.col !== undefined) {
        targetRow = parseInt(el.dataset.row, 10);
        targetCol = parseInt(el.dataset.col, 10);
        break;
      }
    }

    const fromSquare  = convertToChessSquare(ds.fromRow, ds.fromCol);
    const toSquare    = targetRow >= 0 ? convertToChessSquare(targetRow, targetCol) : null;
    const targetPiece = targetRow >= 0 ? board[targetRow][targetCol] : null;

    const isValidDrop =
      targetRow >= 0 &&
      !(targetRow === ds.fromRow && targetCol === ds.fromCol) &&
      !areSameSide(ds.piece, targetPiece) &&
      !isLoadingMovesRef.current &&
      legalMovesRef.current.includes(toSquare);

    if (isValidDrop) {
      setActiveDrag(null);
      clearSelection();
      const moveData = {
        from: { row: ds.fromRow, col: ds.fromCol, square: fromSquare },
        to:   { row: targetRow,  col: targetCol,  square: toSquare   },
      };
      if (needsPromotion(ds.piece, targetRow)) {
        setPendingPromotion(moveData);
        setMessage(`Choose a piece for the pawn on ${toSquare}.`);
      } else {
        await submitMove(moveData);
      }
    } else {
      snapBack(ds);
    }
  }

  function handlePointerCancel(e, rowIndex, colIndex) {
    const ds = dragStateRef.current;
    if (!ds || ds.fromRow !== rowIndex || ds.fromCol !== colIndex) return;
    if (ds.isDragging) snapBack(ds);
    else dragStateRef.current = null;
  }

  function snapBack(ds) {
    dragStateRef.current = null;
    clearSelection();
    const ghost = ghostRef.current;
    if (!ghost) { setActiveDrag(null); return; }
    ghost.style.transition = "left 200ms cubic-bezier(0.25,1,0.5,1), top 200ms cubic-bezier(0.25,1,0.5,1)";
    ghost.style.left = `${ds.squareCenterX - ds.squareSize / 2}px`;
    ghost.style.top  = `${ds.squareCenterY - ds.squareSize / 2}px`;
    setTimeout(() => { if (ghostRef.current) ghostRef.current.style.transition = ""; setActiveDrag(null); }, 210);
  }

  // ── Promotion / new game ─────────────────────────────────────────────────

  async function handlePromotionChoice(pieceName) {
    if (pendingCardPromotion) {
      const { cardId, fromSquares } = pendingCardPromotion;
      setPendingCardPromotion(null);
      await executePlayCard(cardId, { fromSquares, promotion: pieceName });
      return;
    }

    const moveData = pendingPromotion;
    setPendingPromotion(null);
    await submitMove(moveData, pieceName);
  }

  function cancelPromotion() {
    const wasCardPromotion = Boolean(pendingCardPromotion);
    setPendingPromotion(null);
    setPendingCardPromotion(null);
    if (wasCardPromotion) {
      setTargeting(null);
      setMessage("Onslaught canceled.");
    } else {
      setMessage("Select a piece.");
    }
  }

  async function handleNewGame() {
    setIsThinking(true);
    busyRef.current = true;
    setPendingPromotion(null);
    setPendingCardPromotion(null);
    setActiveDrag(null);
    setTargeting(null);
    dragStateRef.current = null;
    clearSelection();

    try {
      const result = await resetGame(startingBoard);
      applyGameResponse(result);
      setLastMove(null);
      setGameOver(false);
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

  // ── Derived card helpers ──────────────────────────────────────────────────

  function getPlayerCards(player) {
    return cardState?.[player] ?? { deckCount: 0, hand: [], discardPile: [], cardPlayedOnOwnTurn: false, discardedThisTurn: false };
  }

  function canPlayCardNow(player, def) {
    if (!def || gameOver || isThinking) return false;
    const state = getPlayerCards(player);
    const isMyTurn = currentTurn === player;
    if (!isMyTurn) return false;
    if (def.countsAsOwnTurnCard && state.cardPlayedOnOwnTurn) return false;
    return state.hand.includes(def.id);
  }

  function canDiscardNow(player, cardId) {
    if (gameOver || isThinking) return false;
    const state = getPlayerCards(player);
    return currentTurn === player && !state.discardedThisTurn && state.hand.includes(cardId);
  }

  // ── Render helpers ────────────────────────────────────────────────────────

  function renderCardHand(player) {
    const state = getPlayerCards(player);
    const isActivePlayer = currentTurn === player;
    const playerDefs = state.hand
      .map((id) => cardDefinitions.find((d) => d.id === id) ?? FALLBACK_CARD_DEFS[id])
      .filter(Boolean);

    return (
      <div className={`player-hand${isActivePlayer ? " active-hand" : ""}`}>
        <div className="hand-header">
          <span className={`hand-dot ${player.toLowerCase()}`} aria-hidden="true" />
          <span className="hand-player-name">{player}</span>
          <span className="hand-counts">
            <span className="hand-count-item" title="Cards in deck">
              <span className="hand-count-icon">▤</span>{state.deckCount}
            </span>
            <span className="hand-count-sep">·</span>
            <span className="hand-count-item" title="Cards in discard">
              <span className="hand-count-icon">✕</span>{state.discardPile.length}
            </span>
          </span>
          {isActivePlayer && <span className="hand-turn-marker">Active</span>}
        </div>

        <div className="hand-cards">
          {playerDefs.length === 0 ? (
            <div className="hand-empty">No cards in hand</div>
          ) : (
            playerDefs.map((def) => {
              const playable = canPlayCardNow(player, def);
              const discardable = canDiscardNow(player, def.id);
              const isTargeting = targeting?.cardId === def.id;

              return (
                <div
                  key={def.id}
                  className={`card${playable ? " card-playable" : ""}${isTargeting ? " card-targeting" : ""}${!isActivePlayer ? " card-inactive" : ""}`}
                >
                  <div className="card-level">Lv.{def.level}</div>
                  <CardArt def={def} />
                  <div className="card-name">{def.name}</div>
                  <div className="card-desc">{cardDescription(def)}</div>
                  <div className="card-timing">{cardTimingLabel(def.timing)}</div>
                  <div className="card-actions">
                    {isTargeting ? (
                      <>
                        {def.id === "onslaught" && (
                          <button
                            type="button"
                            className="card-btn card-btn-play"
                            disabled={(targeting.selectedSquares ?? []).length === 0}
                            onClick={confirmOnslaughtSelection}
                          >
                            Play Selected
                          </button>
                        )}
                        <button
                          type="button"
                          className="card-btn card-btn-cancel"
                          onClick={cancelTargeting}
                        >
                          Cancel
                        </button>
                      </>
                    ) : (
                      <>
                        <button
                          type="button"
                          className="card-btn card-btn-play"
                          disabled={!playable || Boolean(targeting)}
                          onClick={() => startTargeting(def)}
                          title={!playable ? (isActivePlayer ? "Already played a card this turn" : "Not your turn") : "Play this card"}
                        >
                          Play
                        </button>
                        <button
                          type="button"
                          className="card-btn card-btn-discard"
                          disabled={!discardable || Boolean(targeting)}
                          onClick={() => executeDiscardCard(player, def.id)}
                          title={!discardable ? (isActivePlayer ? "Already discarded this turn" : "Not your turn") : "Discard to draw next turn"}
                        >
                          Discard
                        </button>
                      </>
                    )}
                  </div>
                </div>
              );
            })
          )}
        </div>
      </div>
    );
  }

  // ── Main render ───────────────────────────────────────────────────────────

  return (
    <div className={`app${activeDrag ? " is-dragging" : ""}`}>
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
            {targeting && (
              <span className="targeting-badge">
                {targeting.cardId === "charge" && targeting.step === 1 && "⚔ Select a knight"}
                {targeting.cardId === "charge" && targeting.step === 2 && "⚔ Choose the first hop"}
                {targeting.cardId === "charge" && targeting.step === 3 && "⚔ Choose the landing square"}
                {targeting.cardId === "destroy_pawn" && "⚔ Select an enemy pawn"}
                {targeting.cardId === "onslaught" && `⚔ ${(targeting.selectedSquares ?? []).length} pawn${(targeting.selectedSquares ?? []).length === 1 ? "" : "s"} selected`}
              </span>
            )}
          </div>
          <p className="status-message">{message}</p>
        </div>
        <div className="game-tools">
          <button type="button" className="new-game" onClick={handleNewGame} disabled={isThinking}>
            <span className="new-game-icon" aria-hidden="true">↺</span>
            New Game
          </button>
          <span className={`backend-status ${backendStatus}`}>
            {backendLabels[backendStatus]}
          </span>
        </div>
      </header>

      <div
        className={`board${isThinking ? " thinking" : ""}${targeting ? " card-targeting-mode" : ""}`}
        aria-label="Chess board"
      >
        {board.map((row, rowIndex) =>
          row.map((piece, colIndex) => {
            const isLightSquare   = (rowIndex + colIndex) % 2 === 0;
            const isSelected      = isSameSquare(selectedSquare, rowIndex, colIndex);
            const isLastMove      = isSameSquare(lastMove?.from, rowIndex, colIndex) || isSameSquare(lastMove?.to, rowIndex, colIndex);
            const squareName      = convertToChessSquare(rowIndex, colIndex);
            const isLegalTarget   = legalMoves.includes(squareName);
            const isCaptureTarget = isLegalTarget && piece !== "";
            const isDragSource    = activeDrag?.fromRow === rowIndex && activeDrag?.fromCol === colIndex;

            // Per-square targeting class for the active card's targeting state
            let targetingClass = "";
            if (targeting) {
              const { cardId, step, highlights = [], selectedSquares = [], fromRow, fromCol, midRow, midCol } = targeting;
              const isFrom = fromRow === rowIndex && fromCol === colIndex;
              const isMid  = midRow  === rowIndex && midCol  === colIndex;
              const inHighlights = highlights.includes(squareName);
              const isOnslaughtSelected = selectedSquares.includes(squareName);

              if (cardId === "destroy_pawn") {
                const isEnemyPawn = (currentTurn === "White" && piece === "p") || (currentTurn === "Black" && piece === "P");
                targetingClass = isEnemyPawn ? "sq-destroy-target" : "sq-card-dim";
              } else if (cardId === "charge") {
                if (step === 1) {
                  const isFriendlyKnight = (currentTurn === "White" && piece === "N") || (currentTurn === "Black" && piece === "n");
                  targetingClass = isFriendlyKnight ? "sq-charge-pick" : "sq-card-dim";
                } else if (step === 2) {
                  if (isFrom)        targetingClass = "sq-charge-origin";
                  else if (inHighlights) targetingClass = "sq-charge-hop";
                  else               targetingClass = "sq-card-dim";
                } else if (step === 3) {
                  if (isFrom)        targetingClass = "sq-charge-origin";
                  else if (isMid)    targetingClass = "sq-charge-mid";
                  else if (inHighlights) targetingClass = "sq-charge-land";
                  else               targetingClass = "sq-card-dim";
                }
              } else if (cardId === "onslaught") {
                if (isOnslaughtSelected) targetingClass = "sq-onslaught-selected";
                else if (canOnslaughtSelectPawn(board, rowIndex, colIndex, currentTurn)) targetingClass = "sq-onslaught-pick";
                else targetingClass = "sq-card-dim";
              }
            }

            return (
              <button
                type="button"
                key={`${rowIndex}-${colIndex}`}
                data-row={rowIndex}
                data-col={colIndex}
                onClick={() => handleSquareClick(rowIndex, colIndex)}
                onPointerDown={(e) => handlePointerDown(e, rowIndex, colIndex)}
                onPointerMove={(e) => handlePointerMove(e, rowIndex, colIndex)}
                onPointerUp={(e) => handlePointerUp(e, rowIndex, colIndex)}
                onPointerCancel={(e) => handlePointerCancel(e, rowIndex, colIndex)}
                className={`square ${isLightSquare ? "light" : "dark"}${isSelected ? " selected" : ""}${isLastMove ? " last-move" : ""}${isLegalTarget ? (isCaptureTarget ? " capture-target" : " legal-target") : ""}${targetingClass ? ` ${targetingClass}` : ""}`}
                aria-label={`${squareName}${piece ? ` ${piece}` : ""}${isLegalTarget ? " legal destination" : ""}${targetingClass === "sq-destroy-target" || targetingClass === "sq-charge-pick" || targetingClass === "sq-charge-hop" || targetingClass === "sq-charge-land" || targetingClass === "sq-onslaught-pick" || targetingClass === "sq-onslaught-selected" ? " targetable" : ""}`}
                disabled={isThinking || Boolean(pendingPromotion) || Boolean(pendingCardPromotion)}
              >
                <span className={`piece${piece ? (isWhitePiece(piece) ? " white-piece" : " black-piece") : " empty"}${isDragSource ? " drag-hidden" : ""}`}>
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

      {cardState && (
        <div className="card-area">
          {renderCardHand("White")}
        </div>
      )}

      {/* Drag ghost */}
      {activeDrag && (
        <div
          ref={ghostRef}
          className={`drag-ghost ${isWhitePiece(activeDrag.piece) ? "white-piece" : "black-piece"}`}
          aria-hidden="true"
          style={{ width: activeDrag.squareSize, height: activeDrag.squareSize }}
        >
          {pieceSymbols[activeDrag.piece]}
        </div>
      )}

      {(pendingPromotion || pendingCardPromotion) && (
        <div className="promotion-backdrop">
          <section className="promotion-dialog" role="dialog" aria-modal="true" aria-label="Promote pawn">
            <h2>Promote pawn</h2>
            <div className="promotion-options">
              {promotionPieces.map((pieceName) => {
                const symbol = promotionSymbol(pieceName, currentTurn === "White");
                return (
                  <button type="button" className="promotion-option" key={pieceName} onClick={() => handlePromotionChoice(pieceName)}>
                    <span className="promotion-piece" aria-hidden="true">{pieceSymbols[symbol]}</span>
                    <span>{pieceName}</span>
                  </button>
                );
              })}
            </div>
            <button type="button" className="cancel-promotion" onClick={cancelPromotion}>Cancel</button>
          </section>
        </div>
      )}
    </div>
  );
}

export default App;
