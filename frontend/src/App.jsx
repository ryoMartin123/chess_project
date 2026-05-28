import { useCallback, useEffect, useLayoutEffect, useRef, useState } from "react";
import {
  getGameState, getLegalMoves, makeMove, resetGame,
  getCardDefinitions, playCard, discardCard, drawCardAsTurn, claimCheckmate,
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
  disintegrate: { id: "disintegrate", name: "Disintegrate", level: 2, timing: "BEFORE_OWN_TURN", targetRequirement: "FRIENDLY_NON_KING", effect: "DESTROY_TARGET", countsAsOwnTurnCard: false },
  charge:       { id: "charge",       name: "Charge",       level: 6, timing: "AS_OWN_TURN", targetRequirement: "FRIENDLY_KNIGHT", effect: "CHARGE_KNIGHT",  countsAsOwnTurnCard: true },
  onslaught:    { id: "onslaught",    name: "Onslaught",    level: 6, timing: "AS_OWN_TURN", targetRequirement: "FRIENDLY_PAWNS",  effect: "MOVE_PAWNS",     countsAsOwnTurnCard: true },
  knightmare:   { id: "knightmare",   name: "Knightmare",   level: 10, timing: "AFTER_OPPONENT_TURN", targetRequirement: "NONE", effect: "CANCEL_LAST_ACTION", countsAsOwnTurnCard: false },
  think_again:  { id: "think_again",  name: "Think Again!", level: 10, timing: "AFTER_OPPONENT_TURN", targetRequirement: "NONE", effect: "CANCEL_LAST_ACTION", countsAsOwnTurnCard: false },
  bog:          { id: "bog",          name: "Bog",          level: 4, timing: "AFTER_OPPONENT_TURN", targetRequirement: "NONE", effect: "BOG_MOVE", countsAsOwnTurnCard: false },
  neutrality:   { id: "neutrality",   name: "Neutrality",   level: 9, timing: "AFTER_OWN_TURN", targetRequirement: "ENEMY_NON_KING_QUEEN", effect: "APPLY_NEUTRALITY", countsAsOwnTurnCard: false },
  warlord:      { id: "warlord",      name: "Warlord",      level: 10, timing: "AS_OWN_TURN", targetRequirement: "FRIENDLY_KING", effect: "APPLY_WARLORD", countsAsOwnTurnCard: true },
};

const SB_RANKS = [8, 7, 6, 5, 4, 3, 2, 1];
const SB_FILES = ["a", "b", "c", "d", "e", "f", "g", "h"];

const CARD_SCENARIOS = {
  disintegrate: {
    steps: [
      {
        pieces: { e1: "K", h1: "R", h5: "N", h6: "r", h8: "k" },
        highlights: { card: ["h5"] },
        caption: "White knight on h5 is blocking the rook — Disintegrate it before your move",
      },
      {
        pieces: { e1: "K", h1: "R", h6: "r", h8: "k" },
        highlights: { result: ["h1", "h6"] },
        caption: "Knight is gone — rook attacks h6 with the entire h-file open",
      },
    ],
  },
  charge: {
    steps: [
      {
        pieces: { e1: "K", b1: "N", e8: "k", b6: "q", f6: "r" },
        highlights: { card: ["b1"], path: ["c3", "d5"] },
        caption: "Knight charges: b1 → c3 (empty first hop, no capture) → d5",
      },
      {
        pieces: { e1: "K", d5: "N", e8: "k", b6: "q", f6: "r" },
        highlights: { result: ["d5"], threat: ["b6", "f6"] },
        caption: "Knight lands on d5, forking Black's queen and rook in one action",
      },
    ],
  },
  onslaught: {
    steps: [
      {
        pieces: { e1: "K", d4: "P", e4: "P", f4: "P", d6: "b", e6: "n", f6: "r", e8: "k" },
        highlights: { card: ["d4", "e4", "f4"] },
        caption: "Select all three pawns — Onslaught pushes them all forward at once",
      },
      {
        pieces: { e1: "K", d5: "P", e5: "P", f5: "P", d6: "b", e6: "n", f6: "r", e8: "k" },
        highlights: { result: ["d5", "e5", "f5"], threat: ["e6"] },
        caption: "Three pawns advance in one turn — e6 knight attacked from both sides",
      },
    ],
  },
  knightmare: {
    steps: [
      {
        pieces: { g1: "K", h2: "P", h4: "q", e8: "k" },
        highlights: { card: ["h4"], threat: ["h2"] },
        caption: "Black queen just moved to h4 threatening your pawn — play Knightmare",
      },
      {
        pieces: { g1: "K", h2: "P", d8: "q", e8: "k" },
        highlights: { result: ["d8"], forbidden: ["h4"] },
        caption: "Move undone — queen returns to d8. Black replays but cannot go back to h4",
      },
    ],
  },
  think_again: {
    steps: [
      {
        pieces: { g1: "K", h2: "P", h4: "q", e8: "k" },
        highlights: { card: ["h4"], threat: ["h2"] },
        caption: "Black queen just moved to h4 threatening your pawn — play Think Again!",
      },
      {
        pieces: { g1: "K", h2: "P", d8: "q", e8: "k" },
        highlights: { result: ["d8"], forbidden: ["h4"] },
        caption: "Move undone — queen returns to d8. Black replays but cannot go back to h4",
      },
    ],
  },
  bog: {
    steps: [
      {
        pieces: { e1: "K", a1: "R", a8: "r", e8: "k" },
        highlights: { card: ["a8"], path: ["a7", "a6", "a5", "a4", "a3", "a2"], threat: ["a1"] },
        caption: "Black rook slides down the a-file toward your rook on a1 — play Bog",
      },
      {
        pieces: { e1: "K", a1: "R", a7: "r", e8: "k" },
        highlights: { result: ["a7"], stopped: ["a6", "a5", "a4", "a3", "a2"] },
        caption: "Bog shortens the move to one square — rook stops at a7, your rook is safe",
      },
    ],
  },
  neutrality: {
    steps: [
      {
        pieces: { e1: "K", e8: "k", f6: "n", g8: "r" },
        highlights: { card: ["f6"] },
        caption: "Target the enemy knight on f6 after your move — mark it neutral",
      },
      {
        pieces: { e1: "K", e8: "k", f6: "n", g8: "r" },
        highlights: { neutral: ["f6"], threat: ["e8", "g8"] },
        caption: "Knight is neutral — when it moves it can check either king and capture any piece",
      },
    ],
  },
  warlord: {
    steps: [
      {
        pieces: { e1: "K", d3: "p", f3: "p", e8: "k" },
        highlights: { card: ["e1"] },
        caption: "Play Warlord on your King before your move — it gains power to move up to 2 squares",
      },
      {
        pieces: { e3: "K", d3: "p", f3: "p", e8: "k" },
        highlights: { warlord: ["e3"], result: ["e3"], threat: ["d3", "f3"] },
        caption: "Warlord King moved 2 squares forward: e1 → e2 → e3, threatening both pawns at once",
      },
    ],
  },
};

function ScenarioBoard({ steps }) {
  const [stepIndex, setStepIndex] = useState(0);
  const [fading, setFading] = useState(false);

  useEffect(() => {
    if (steps.length < 2) return;
    let fadeTimeout;
    const intervalId = setInterval(() => {
      setFading(true);
      fadeTimeout = setTimeout(() => {
        setStepIndex((i) => (i + 1) % steps.length);
        setFading(false);
      }, 260);
    }, 2800);
    return () => { clearInterval(intervalId); clearTimeout(fadeTimeout); };
  }, [steps.length]);

  function goToStep(i) {
    setFading(true);
    setTimeout(() => { setStepIndex(i); setFading(false); }, 200);
  }

  if (!steps.length) return null;
  const { pieces, highlights = {}, caption } = steps[stepIndex];

  return (
    <div className="sb-wrapper">
      <div className={`sb-board${fading ? " sb-fading" : ""}`}>
        {SB_RANKS.map((rank, rowIndex) =>
          SB_FILES.map((file, colIndex) => {
            const sq = `${file}${rank}`;
            const piece = pieces[sq] ?? "";
            const isLight = (rowIndex + colIndex) % 2 === 0;
            const isWhitePc = piece && piece === piece.toUpperCase();
            const hlCard     = highlights.card?.includes(sq);
            const hlPath     = highlights.path?.includes(sq);
            const hlResult   = highlights.result?.includes(sq);
            const hlThreat   = highlights.threat?.includes(sq);
            const hlForbid   = highlights.forbidden?.includes(sq);
            const hlNeutral  = highlights.neutral?.includes(sq);
            const hlStopped  = highlights.stopped?.includes(sq);
            const hlWarlord  = highlights.warlord?.includes(sq);

            return (
              <div
                key={sq}
                className={[
                  "sb-square",
                  isLight ? "sb-light" : "sb-dark",
                  hlCard    ? "sb-hl-card"    : "",
                  hlPath    ? "sb-hl-path"    : "",
                  hlResult  ? "sb-hl-result"  : "",
                  hlThreat  ? "sb-hl-threat"  : "",
                  hlStopped ? "sb-hl-stopped" : "",
                ].filter(Boolean).join(" ")}
              >
                {piece && (
                  <span className={`sb-piece ${isWhitePc ? "sb-piece-white" : "sb-piece-black"}${hlNeutral ? " sb-piece-neutral" : ""}${hlWarlord ? " sb-piece-warlord" : ""}`}>
                    {pieceSymbols[piece] ?? ""}
                  </span>
                )}
                {hlForbid   && <span className="sb-forbidden"    aria-hidden="true">✕</span>}
                {hlNeutral  && <span className="sb-neutral-dot"  aria-hidden="true" />}
              </div>
            );
          })
        )}
      </div>
      <p className="sb-caption">{caption}</p>
      {steps.length > 1 && (
        <div className="sb-dots" aria-hidden="true">
          {steps.map((_, i) => (
            <button
              key={i}
              type="button"
              className={`sb-dot${i === stepIndex ? " sb-dot-active" : ""}`}
              onClick={() => goToStep(i)}
              aria-label={`Step ${i + 1}`}
            />
          ))}
        </div>
      )}
    </div>
  );
}

function cardDescription(def) {
  if (!def) return "";
  if (def.effect === "DESTROY_TARGET" && def.targetRequirement === "FRIENDLY_NON_KING") {
    return "Destroy one of your non-king pieces before moving.";
  }
  if (def.effect === "CHARGE_KNIGHT") {
    return "Move a knight twice. First hop must land on an empty square.";
  }
  if (def.effect === "MOVE_PAWNS") {
    return "Move any number of your pawns one square forward.";
  }
  if (def.effect === "CANCEL_LAST_ACTION") {
    return "Cancel your opponent's last move or card. They must replay, but cannot repeat the same action.";
  }
  if (def.effect === "BOG_MOVE") {
    return "Slow a long rook, bishop, or queen move to one square.";
  }
  if (def.effect === "APPLY_NEUTRALITY") {
    return "Mark an opponent non-king, non-queen piece as neutral.";
  }
  if (def.effect === "APPLY_WARLORD") {
    return "Your King may move up to 2 squares in any direction per turn. Capture ends its move for that turn.";
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
  if (def.effect === "DESTROY_TARGET" && def.targetRequirement === "FRIENDLY_NON_KING") {
    return (
      <div className="card-art">
        <span className="card-art-piece">♙</span>
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
  if (def.effect === "CANCEL_LAST_ACTION") {
    return (
      <div className="card-art">
        <span className="card-art-piece card-art-knight">♞</span>
        <span className="card-art-x card-art-nightmare" aria-hidden="true">↺</span>
      </div>
    );
  }
  if (def.effect === "BOG_MOVE") {
    return (
      <div className="card-art">
        <span className="card-art-piece">Q</span>
        <span className="card-art-charge" aria-hidden="true">~</span>
      </div>
    );
  }
  if (def.effect === "APPLY_NEUTRALITY") {
    return (
      <div className="card-art">
        <span className="card-art-piece">N</span>
        <span className="card-art-charge" aria-hidden="true">○</span>
      </div>
    );
  }
  if (def.effect === "APPLY_WARLORD") {
    return (
      <div className="card-art card-art-warlord">
        <span className="card-art-piece card-art-warlord-king" aria-hidden="true">♔</span>
        <span className="card-art-warlord-crown" aria-hidden="true">⚔</span>
      </div>
    );
  }
  return <div className="card-art"><span className="card-art-piece">?</span></div>;
}

// Cards that trigger a cinematic effect on successful play
const CINEMATIC_CARDS = new Set(["knightmare", "think_again"]);

function CardCinematic({ event, onComplete }) {
  useEffect(() => {
    const timer = setTimeout(onComplete, 2300);
    return () => clearTimeout(timer);
  }, [onComplete]);

  if (event.type === "knightmare" || event.type === "think_again") {
    const title = event.type === "think_again" ? "THINK AGAIN!" : "KNIGHTMARE";
    return (
      <div className="cinematic-overlay" role="status" aria-atomic="true">
        <div className="cnm-flash" aria-hidden="true" />
        <div className="cinematic-knightmare">
          <span className="cnm-knight" aria-hidden="true">♞</span>
          <div className="cnm-title" aria-hidden="true">{title}</div>
          <div className="cnm-sub">Move Cancelled</div>
        </div>
      </div>
    );
  }
  return null;
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

function isFriendlyNonKing(piece, player) {
  if (!piece || piece.toLowerCase() === "k") return false;
  return (player === "White" && isWhitePiece(piece)) || (player === "Black" && !isWhitePiece(piece));
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
  const [neutralSquares, setNeutralSquares] = useState([]);
  const [warlordSquares, setWarlordSquares] = useState([]);
  const [gameOver, setGameOver] = useState(false);
  const [pendingCheckmate, setPendingCheckmate] = useState("");
  const [checkmateSecondsLeft, setCheckmateSecondsLeft] = useState(0);
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
  const [cinematicEvent, setCinematicEvent] = useState(null);
  const [continuingEffects, setContinuingEffects] = useState([]);
  const [afterOwnMovePlayer, setAfterOwnMovePlayer] = useState(null);
  const [cardScenario, setCardScenario] = useState(null);
  const [drawNotice, setDrawNotice] = useState(null); // { player, cardId }
  const [afterMovePrompt, setAfterMovePrompt] = useState(null); // { player, cards: def[] }
  const [promptEnabled, setPromptEnabled] = useState(() => localStorage.getItem("afterMovePromptEnabled") !== "false");

  const checkmateTimerEndRef = useRef(null);
  const checkmateIntervalRef = useRef(null);
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

  useEffect(() => {
    if (!pendingCheckmate) {
      if (checkmateIntervalRef.current) {
        clearInterval(checkmateIntervalRef.current);
        checkmateIntervalRef.current = null;
      }
      checkmateTimerEndRef.current = null;
      return;
    }
    const endTime = Date.now() + 120 * 1000;
    checkmateTimerEndRef.current = endTime;
    checkmateIntervalRef.current = setInterval(() => {
      const remaining = Math.max(0, Math.round((checkmateTimerEndRef.current - Date.now()) / 1000));
      setCheckmateSecondsLeft(remaining);
      if (remaining <= 0) {
        clearInterval(checkmateIntervalRef.current);
        checkmateIntervalRef.current = null;
        handleClaimCheckmate();
      }
    }, 1000);
    return () => {
      if (checkmateIntervalRef.current) clearInterval(checkmateIntervalRef.current);
    };
  }, [pendingCheckmate]); // eslint-disable-line react-hooks/exhaustive-deps

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
        setNeutralSquares(game.neutralSquares ?? []);
        setWarlordSquares(game.warlordSquares ?? []);
        setContinuingEffects(game.continuingEffects ?? []);
        setPendingCheckmate(game.pendingCheckmate ?? "");
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
    setNeutralSquares(result.neutralSquares ?? []);
    setWarlordSquares(result.warlordSquares ?? []);
    setContinuingEffects(result.continuingEffects ?? []);
    setPendingCheckmate(result.pendingCheckmate ?? "");
    setMessage(result.message);
    if (result.cards) setCardState(result.cards);
    // Detect auto-draw at start of turn from backend message
    const autoDrawMatch = result.message?.match(/(\w+) drew (\S+) at the end of their turn/);
    if (autoDrawMatch) {
      setDrawNotice({ player: autoDrawMatch[1], cardId: autoDrawMatch[2] });
      setTimeout(() => setDrawNotice(null), 3500);
    }
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
        const justMovedPlayer = currentTurn;
        setAfterOwnMovePlayer(justMovedPlayer);
        if (promptEnabled && !result.gameOver && justMovedPlayer === "White") {
          const hand = result.cards?.[justMovedPlayer]?.hand ?? [];
          const afterMoveCards = hand
            .map((id) => cardDefinitions.find((d) => d.id === id) ?? FALLBACK_CARD_DEFS[id])
            .filter((def) => def?.timing === "AFTER_OWN_TURN");
          if (afterMoveCards.length > 0) {
            setAfterMovePrompt({ player: justMovedPlayer, cards: afterMoveCards });
          }
        }
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

  async function executePlayCard(player, cardId, squares) {
    setTargeting(null);
    clearSelection();
    setIsThinking(true);
    busyRef.current = true;
    let showCinematic = false;

    try {
      const result = await playCard(player, cardId, squares);
      failedPollCount.current = 0;
      setBackendStatus("connected");
      applyGameResponse(result);
      if (result.cardResult?.success) setAfterOwnMovePlayer(null);
      if (result.cardResult?.success && CINEMATIC_CARDS.has(cardId)) {
        showCinematic = true;
        setCinematicEvent({ type: cardId, message: result.message });
      }
    } catch (error) {
      setBackendStatus("offline");
      setMessage(error.message || "Card play failed.");
    } finally {
      if (!showCinematic) {
        setIsThinking(false);
        busyRef.current = false;
      }
    }
  }

  const onCinematicComplete = useCallback(function () {
    setCinematicEvent(null);
    setIsThinking(false);
    busyRef.current = false;
  }, []);

  async function executeDrawCard(player) {
    setIsThinking(true);
    busyRef.current = true;

    try {
      const result = await drawCardAsTurn(player);
      failedPollCount.current = 0;
      setBackendStatus("connected");
      applyGameResponse(result);
    } catch (error) {
      setBackendStatus("offline");
      setMessage(error.message || "Draw failed.");
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

  function startTargeting(player, def) {
    clearSelection();
    setAfterMovePrompt(null);
    if (def.effect === "CANCEL_LAST_ACTION" || def.effect === "BOG_MOVE") {
      executePlayCard(player, def.id, {});  // no target selection needed
    } else if (def.targetRequirement === "FRIENDLY_NON_KING") {
      setTargeting({ cardId: def.id, player, step: 1, highlights: [] });
      setMessage("Select one of your non-king pieces to disintegrate.");
    } else if (def.targetRequirement === "ENEMY_NON_KING_QUEEN") {
      setTargeting({ cardId: def.id, player, step: 1, highlights: [] });
      setMessage("Select an opponent non-king, non-queen piece to make neutral.");
    } else if (def.targetRequirement === "FRIENDLY_KNIGHT") {
      setTargeting({ cardId: def.id, player, step: 1, highlights: [] });
      setMessage("Select one of your knights to charge with.");
    } else if (def.targetRequirement === "FRIENDLY_PAWNS") {
      setTargeting({ cardId: def.id, player, step: 1, highlights: [], selectedSquares: [] });
      setMessage("Select any pawns you want to move one square forward, then press Play Selected.");
    } else if (def.targetRequirement === "FRIENDLY_KING") {
      setTargeting({ cardId: def.id, player, step: 1, highlights: [] });
      setMessage("Select your King to grant it Warlord powers.");
    }
  }

  function cancelTargeting() {
    setTargeting(null);
    setMessage("Select a piece or play a card.");
  }

  function togglePrompt() {
    const next = !promptEnabled;
    setPromptEnabled(next);
    localStorage.setItem("afterMovePromptEnabled", String(next));
    if (!next) setAfterMovePrompt(null);
  }

  function isNeutralSquare(squareName) {
    return neutralSquares.includes(squareName);
  }

  function isFriendlyNonKingForCard(piece, player, squareName) {
    return isFriendlyNonKing(piece, player) || (isNeutralSquare(squareName) && piece && piece.toLowerCase() !== "k");
  }

  function isEnemyNonKingQueenForCard(piece, player, squareName) {
    if (!piece || piece.toLowerCase() === "k" || piece.toLowerCase() === "q") return false;
    return isNeutralSquare(squareName) || (player === "White" ? !isWhitePiece(piece) : isWhitePiece(piece));
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
      setPendingCardPromotion({ cardId: "onslaught", player: targeting.player ?? currentTurn, fromSquares });
      setMessage("Choose a promotion piece for Onslaught.");
      return;
    }

    await executePlayCard(targeting.player ?? currentTurn, "onslaught", { fromSquares, promotion: "Queen" });
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
      const targetPlayer = targeting.player ?? currentTurn;

      if (cardId === "disintegrate") {
        if (isFriendlyNonKingForCard(piece, targetPlayer, squareName)) {
          await executePlayCard(targetPlayer, cardId, { targetSquare: squareName });
        } else {
          setMessage("Select one of your non-king pieces to disintegrate, or press Cancel.");
        }
        return;
      }

      if (cardId === "neutrality") {
        if (isEnemyNonKingQueenForCard(piece, targetPlayer, squareName)) {
          await executePlayCard(targetPlayer, cardId, { targetSquare: squareName });
        } else {
          setMessage("Select an opponent non-king, non-queen piece for Neutrality.");
        }
        return;
      }

      if (cardId === "charge") {
        if (step === 1) {
          const isFriendlyKnight = (targetPlayer === "White" && piece === "N") || (targetPlayer === "Black" && piece === "n") || (isNeutralSquare(squareName) && piece?.toLowerCase() === "n");
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
              const isOwn = p && !isNeutralSquare(convertToChessSquare(r, c)) && ((targetPlayer === "White" && isWhitePiece(p)) || (targetPlayer === "Black" && !isWhitePiece(p)));
              if (!isOwn) validLandings.push(convertToChessSquare(r, c));
            }
          }
          setTargeting({ ...targeting, step: 3, highlights: validLandings, midSquare: squareName, midRow: rowIndex, midCol: colIndex });
          setMessage(`First hop: ${squareName}. Now choose the final landing square.`);
          return;
        }

        if (step === 3) {
          if (!highlights.includes(squareName)) { setMessage("That square isn't a valid landing. Choose a square the knight can reach from the hop."); return; }
          await executePlayCard(targetPlayer, cardId, {
            fromSquare:         targeting.fromSquare,
            targetSquare:       targeting.midSquare,
            secondTargetSquare: squareName,
          });
          return;
        }
      }

      if (cardId === "warlord") {
        const isMyKing = (targetPlayer === "White" && piece === "K") || (targetPlayer === "Black" && piece === "k");
        if (!isMyKing) { setMessage("Select your King to grant it Warlord powers."); return; }
        await executePlayCard(targetPlayer, cardId, { targetSquare: squareName });
        return;
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

        if (!canOnslaughtSelectPawn(board, rowIndex, colIndex, targetPlayer) && !(isNeutralSquare(squareName) && piece?.toLowerCase() === "p")) {
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
        !isNeutralSquare(squareName) &&
        ((currentTurn === "White" && !isWhitePiece(piece)) ||
        (currentTurn === "Black" &&  isWhitePiece(piece)))
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

    // Legal move check (only conclusive once moves have loaded)
    const isConfirmedLegal = !isLoadingMoves && legalMoves.includes(toSquare);
    // Any piece the current player can move (own color or neutral)
    const isClickedMoveable = piece !== "" && (
      isNeutralSquare(squareName) ||
      (currentTurn === "White" ? isWhitePiece(piece) : !isWhitePiece(piece))
    );

    // Re-select only when NOT a confirmed legal destination
    if (!isConfirmedLegal && isClickedMoveable) {
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
    const dragSquare = convertToChessSquare(rowIndex, colIndex);
    if (
      !neutralSquares.includes(dragSquare) &&
      ((currentTurn === "White" && !isWhitePiece(piece)) ||
      (currentTurn === "Black" &&  isWhitePiece(piece)))
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
    const isDragNeutral  = neutralSquares.includes(fromSquare);
    const isDropNeutral  = toSquare ? neutralSquares.includes(toSquare) : false;

    const isValidDrop =
      targetRow >= 0 &&
      !(targetRow === ds.fromRow && targetCol === ds.fromCol) &&
      (isDragNeutral || isDropNeutral || !areSameSide(ds.piece, targetPiece)) &&
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
      const { cardId, player, fromSquares } = pendingCardPromotion;
      setPendingCardPromotion(null);
      await executePlayCard(player ?? currentTurn, cardId, { fromSquares, promotion: pieceName });
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

  async function handleClaimCheckmate() {
    setIsThinking(true);
    busyRef.current = true;
    try {
      const result = await claimCheckmate();
      applyGameResponse(result);
    } catch {
      setMessage("Failed to finalize checkmate.");
    } finally {
      setIsThinking(false);
      busyRef.current = false;
    }
  }

  async function handleNewGame() {
    setIsThinking(true);
    busyRef.current = true;
    setPendingPromotion(null);
    setPendingCardPromotion(null);
    setActiveDrag(null);
    setTargeting(null);
    setAfterMovePrompt(null);
    dragStateRef.current = null;
    clearSelection();

    try {
      const result = await resetGame(startingBoard);
      applyGameResponse(result);
      setLastMove(null);
      setGameOver(false);
      setPendingCheckmate("");
      setAfterOwnMovePlayer(null);
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
    return cardState?.[player] ?? { deckCount: 0, hand: [], discardPile: [], activePile: [], cardPlayedOnOwnTurn: false, discardedThisTurn: false, drawnThisTurn: false };
  }

  function canPlayCardNow(player, def) {
    if (!def || gameOver || isThinking) return false;
    const state = getPlayerCards(player);
    const isMyTurn = currentTurn === player;
    const isOwnTurnTiming = def.timing === "BEFORE_OWN_TURN" || def.timing === "AS_OWN_TURN" || def.timing === "AFTER_OWN_TURN";
    if (def.timing === "AFTER_OWN_TURN") {
      if (state.cardPlayedOnOwnTurn) return false;
      return afterOwnMovePlayer === player && state.hand.includes(def.id);
    }
    if (!isMyTurn) return false;
    if (isOwnTurnTiming && state.cardPlayedOnOwnTurn) return false;
    return state.hand.includes(def.id);
  }

  function canDiscardNow(player, cardId) {
    if (gameOver || isThinking) return false;
    const state = getPlayerCards(player);
    return currentTurn === player && !state.discardedThisTurn && state.hand.includes(cardId);
  }

  function canDrawNow(player) {
    if (gameOver || isThinking) return false;
    const state = getPlayerCards(player);
    return currentTurn === player && !state.drawnThisTurn && state.deckCount > 0 && state.hand.length < 5;
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
                  <button
                    type="button"
                    className="card-eye-btn"
                    onClick={(e) => { e.stopPropagation(); setCardScenario(def); }}
                    title="Show example scenario"
                    aria-label={`Show scenario for ${def.name}`}
                  >
                    👁
                  </button>
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
                          onClick={() => startTargeting(player, def)}
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

  // ── Shared pile section renderer ─────────────────────────────────────────

  function renderPlayerPileSection(player) {
    const state = getPlayerCards(player);
    const topDiscardId = state.discardPile.at(-1);
    const topDiscardDef = topDiscardId
      ? cardDefinitions.find((d) => d.id === topDiscardId) ?? FALLBACK_CARD_DEFS[topDiscardId]
      : null;
    const canDraw = canDrawNow(player);

    return (
      <div className="side-player-section">
        <div className="side-player-label">
          <span className={`hand-dot ${player.toLowerCase()}`} aria-hidden="true" />
          {player}
        </div>
        <div className="side-piles">
          <div className="side-pile-col">
            <div
              className={`rp-deck-card${state.deckCount === 0 ? " rp-empty" : ""}`}
              title={`${state.deckCount} card${state.deckCount === 1 ? "" : "s"} in deck`}
              aria-hidden="true"
            >
              {state.deckCount > 0 && <span className="rp-deck-symbol">♜</span>}
              <span className="rp-deck-count">{state.deckCount}</span>
            </div>
            <div className="rp-pile-label">Deck</div>
            <button
              type="button"
              className="draw-btn"
              disabled={!canDraw}
              onClick={() => executeDrawCard(player)}
              title={canDraw ? "Draw a card from your deck" : (state.drawnThisTurn ? "Already drew this turn" : state.deckCount === 0 ? "Deck is empty" : state.hand.length >= 5 ? "Hand is full (5 cards max)" : "Not your turn")}
            >
              Draw
            </button>
          </div>
          <div className="side-pile-col">
            <div
              className={`rp-discard-card${topDiscardDef ? " has-card" : ""}`}
              title={`${state.discardPile.length} card${state.discardPile.length === 1 ? "" : "s"} in discard`}
              aria-hidden="true"
            >
              {topDiscardDef ? (
                <>
                  <span className="rp-discard-level">Lv.{topDiscardDef.level}</span>
                  <CardArt def={topDiscardDef} />
                  <span className="rp-discard-name">{topDiscardDef.name}</span>
                </>
              ) : (
                <span className="rp-discard-empty">—</span>
              )}
              {state.discardPile.length > 0 && (
                <span className="rp-discard-count">{state.discardPile.length}</span>
              )}
            </div>
            <div className="rp-pile-label">Discard</div>
          </div>
        </div>
        {state.drawAtStartOfNextTurn && (
          <div className="draw-pending-badge" title="You'll automatically draw a card at the start of your next turn">
            ↑ Draw at turn start
          </div>
        )}
      </div>
    );
  }

  // ── Left panel (Black deck / discard) ─────────────────────────────────────

  function renderLeftPanel() {
    return (
      <aside className="left-panel" aria-label="Black deck and discard pile">
        {renderPlayerPileSection("Black")}
      </aside>
    );
  }

  // ── Right panel (White deck / discard / continuing effects) ───────────────

  function renderRightPanel() {
    return (
      <aside className="right-panel" aria-label="White deck, discard pile, and active effects">
        {renderPlayerPileSection("White")}

        {continuingEffects.filter((e) => e.active).length > 0 && (
          <div className="continuing-effects-section">
            <div className="right-panel-section-label">Active Effects</div>
            {continuingEffects
              .filter((e) => e.active)
              .map((effect) => {
                const def = cardDefinitions.find((d) => d.id === effect.cardId) ?? FALLBACK_CARD_DEFS[effect.cardId];
                return (
                  <div key={effect.id} className={`ce-card ce-${effect.playedBy.toLowerCase()}`}>
                    <div className="ce-card-header">
                      <span className={`hand-dot ${effect.playedBy.toLowerCase()}`} aria-hidden="true" />
                      <span className="ce-card-name">{def?.name ?? effect.cardId}</span>
                    </div>
                    <div className="ce-card-detail">{def ? cardDescription(def) : effect.cardId}</div>
                  </div>
                );
              })}
          </div>
        )}
      </aside>
    );
  }

  function formatCheckmateTimer(seconds) {
    const m = Math.floor(seconds / 60);
    const s = seconds % 60;
    return `${m}:${String(s).padStart(2, "0")}`;
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
                {targeting.cardId === "disintegrate" && "⚔ Select your piece"}
                {targeting.cardId === "neutrality" && "○ Select opponent piece"}
                {targeting.cardId === "warlord" && "⚔ Select your King"}
                {targeting.cardId === "onslaught" && `⚔ ${(targeting.selectedSquares ?? []).length} pawn${(targeting.selectedSquares ?? []).length === 1 ? "" : "s"} selected`}
              </span>
            )}
          </div>
          <p className="status-message">{message}</p>
        </div>
        <div className="game-tools">
          <button
            type="button"
            className={`prompt-toggle-btn${promptEnabled ? " prompt-toggle-on" : ""}`}
            onClick={togglePrompt}
            title={promptEnabled ? "Card prompts on — click to disable" : "Card prompts off — click to enable"}
            aria-label={`Card prompts ${promptEnabled ? "enabled" : "disabled"}`}
          >
            <span className="prompt-toggle-dot" aria-hidden="true" />
            Prompts
          </button>
          <button type="button" className="new-game" onClick={handleNewGame} disabled={isThinking}>
            <span className="new-game-icon" aria-hidden="true">↺</span>
            New Game
          </button>
          <span className={`backend-status ${backendStatus}`}>
            {backendLabels[backendStatus]}
          </span>
        </div>
      </header>

      <div className="game-area">
      {cardState && renderLeftPanel()}
      <div
        className={`board${isThinking ? " thinking" : ""}${targeting ? " card-targeting-mode" : ""}${cinematicEvent ? " board-shaking" : ""}`}
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
            const isNeutral       = neutralSquares.includes(squareName);
            const isWarlord       = warlordSquares.includes(squareName);

            // Per-square targeting class for the active card's targeting state
            let targetingClass = "";
            if (targeting) {
              const { cardId, step, highlights = [], selectedSquares = [], fromRow, fromCol, midRow, midCol } = targeting;
              const targetPlayer = targeting.player ?? currentTurn;
              const isFrom = fromRow === rowIndex && fromCol === colIndex;
              const isMid  = midRow  === rowIndex && midCol  === colIndex;
              const inHighlights = highlights.includes(squareName);
              const isOnslaughtSelected = selectedSquares.includes(squareName);

              if (cardId === "disintegrate") {
                targetingClass = isFriendlyNonKingForCard(piece, targetPlayer, squareName) ? "sq-disintegrate-pick" : "sq-card-dim";
              } else if (cardId === "neutrality") {
                targetingClass = isEnemyNonKingQueenForCard(piece, targetPlayer, squareName) ? "sq-neutrality-pick" : "sq-card-dim";
              } else if (cardId === "charge") {
                if (step === 1) {
                  const isFriendlyKnight = (targetPlayer === "White" && piece === "N") || (targetPlayer === "Black" && piece === "n") || (isNeutral && piece?.toLowerCase() === "n");
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
                else if (canOnslaughtSelectPawn(board, rowIndex, colIndex, targetPlayer) || (isNeutral && piece?.toLowerCase() === "p")) targetingClass = "sq-onslaught-pick";
                else targetingClass = "sq-card-dim";
              } else if (cardId === "warlord") {
                const isMyKing = (targetPlayer === "White" && piece === "K") || (targetPlayer === "Black" && piece === "k");
                targetingClass = isMyKing ? "sq-warlord-pick" : "sq-card-dim";
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
                className={`square ${isLightSquare ? "light" : "dark"}${isSelected ? " selected" : ""}${isLastMove ? " last-move" : ""}${isLegalTarget ? (isCaptureTarget ? " capture-target" : " legal-target") : ""}${isNeutral ? " neutral-marker" : ""}${targetingClass ? ` ${targetingClass}` : ""}`}
                aria-label={`${squareName}${piece ? ` ${piece}` : ""}${isNeutral ? " neutral" : ""}${isWarlord ? " warlord" : ""}${isLegalTarget ? " legal destination" : ""}${targetingClass === "sq-disintegrate-pick" || targetingClass === "sq-neutrality-pick" || targetingClass === "sq-warlord-pick" || targetingClass === "sq-charge-pick" || targetingClass === "sq-charge-hop" || targetingClass === "sq-charge-land" || targetingClass === "sq-onslaught-pick" || targetingClass === "sq-onslaught-selected" ? " targetable" : ""}`}
                disabled={isThinking || Boolean(pendingPromotion) || Boolean(pendingCardPromotion)}
              >
                <span className={`piece${piece ? (isNeutral ? " neutral-piece" : isWhitePiece(piece) ? " white-piece" : " black-piece") : " empty"}${isWarlord ? " warlord-king" : ""}${isDragSource ? " drag-hidden" : ""}`}>
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

      {cardState && renderRightPanel()}
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
          className={`drag-ghost ${neutralSquares.includes(convertToChessSquare(activeDrag.fromRow, activeDrag.fromCol)) ? "neutral-piece" : isWhitePiece(activeDrag.piece) ? "white-piece" : "black-piece"}`}
          aria-hidden="true"
          style={{ width: activeDrag.squareSize, height: activeDrag.squareSize }}
        >
          {pieceSymbols[activeDrag.piece]}
        </div>
      )}

      {cinematicEvent && <CardCinematic event={cinematicEvent} onComplete={onCinematicComplete} />}

      {afterMovePrompt && (
        <div className="after-move-prompt" role="dialog" aria-live="polite">
          <span className="after-move-prompt-label">
            <span className={`hand-dot ${afterMovePrompt.player.toLowerCase()}`} aria-hidden="true" />
            Play a card before your turn ends?
          </span>
          <div className="after-move-prompt-cards">
            {afterMovePrompt.cards.map((def) => (
              <button
                key={def.id}
                type="button"
                className="card-btn card-btn-play after-move-play-btn"
                onClick={() => {
                  setAfterMovePrompt(null);
                  startTargeting(afterMovePrompt.player, def);
                }}
              >
                Play {def.name}
              </button>
            ))}
          </div>
          <button
            type="button"
            className="card-btn card-btn-cancel"
            onClick={() => setAfterMovePrompt(null)}
          >
            Skip
          </button>
        </div>
      )}

      {pendingCheckmate && (
        <div className="checkmate-escape-banner" role="alert" aria-live="assertive">
          <div className="cmb-icon" aria-hidden="true">♚</div>
          <div className="cmb-body">
            <div className="cmb-title">
              <span className={`turn-dot ${pendingCheckmate.toLowerCase()}`} aria-hidden="true" />
              {pendingCheckmate} is in checkmate!
            </div>
            <div className="cmb-sub">Play a reaction card to escape — or the game ends.</div>
          </div>
          <div className={`cmb-timer${checkmateSecondsLeft <= 30 ? " cmb-timer-urgent" : ""}`}>
            {formatCheckmateTimer(checkmateSecondsLeft)}
          </div>
          <button
            type="button"
            className="cmb-surrender"
            onClick={handleClaimCheckmate}
            disabled={isThinking}
          >
            Surrender
          </button>
        </div>
      )}

      {drawNotice && (
        <div className="draw-notice-toast" role="status" aria-live="polite">
          {drawNotice.player} drew {drawNotice.cardId} at the end of their turn
        </div>
      )}

      {cardScenario && (
        <div className="scenario-backdrop" onClick={() => setCardScenario(null)}>
          <section
            className="scenario-dialog"
            onClick={(e) => e.stopPropagation()}
            role="dialog"
            aria-modal="true"
            aria-label={`${cardScenario.name} scenario`}
          >
            <div className="scenario-header">
              <CardArt def={cardScenario} />
              <div className="scenario-header-info">
                <div className="scenario-card-name">{cardScenario.name}</div>
                <div className="scenario-timing">{cardTimingLabel(cardScenario.timing)}</div>
              </div>
              <button
                type="button"
                className="scenario-close"
                onClick={() => setCardScenario(null)}
                aria-label="Close scenario"
              >
                ✕
              </button>
            </div>
            <ScenarioBoard steps={CARD_SCENARIOS[cardScenario.id]?.steps ?? []} />
          </section>
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
