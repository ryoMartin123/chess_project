#include "GameState.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
const std::vector<std::string> STARTING_BOARD = {
    "r", "n", "b", "q", "k", "b", "n", "r",
    "p", "p", "p", "p", "p", "p", "p", "p",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "P", "P", "P", "P", "P", "P", "P", "P",
    "R", "N", "B", "Q", "K", "B", "N", "R"};

std::string squareForFrontendPosition(int row, int col)
{
    std::string square;
    square += static_cast<char>('a' + col);
    square += static_cast<char>('8' - row);
    return square;
}

std::unique_ptr<Pieces> createPiece(char symbol, const std::string &square)
{
    const bool white = std::isupper(static_cast<unsigned char>(symbol));
    const char type = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
    const std::string color = white ? "White" : "Black";

    if (type == 'p')
    {
        return std::make_unique<Pawn>(color, false, square);
    }
    if (type == 'n')
    {
        return std::make_unique<Knight>(color, false, square);
    }
    if (type == 'b')
    {
        return std::make_unique<Bishop>(color, false, square);
    }
    if (type == 'r')
    {
        return std::make_unique<Rook>(color, false, square);
    }
    if (type == 'q')
    {
        return std::make_unique<Queen>(color, false, square);
    }
    if (type == 'k')
    {
        return std::make_unique<King>(color, false, square);
    }

    return nullptr;
}

std::unique_ptr<Pieces> createPieceByType(const std::string &type, const std::string &color, const std::string &square)
{
    if (type == "Pawn")
    {
        return std::make_unique<Pawn>(color, false, square);
    }
    if (type == "Knight")
    {
        return std::make_unique<Knight>(color, false, square);
    }
    if (type == "Bishop")
    {
        return std::make_unique<Bishop>(color, false, square);
    }
    if (type == "Rook")
    {
        return std::make_unique<Rook>(color, false, square);
    }
    if (type == "Queen")
    {
        return std::make_unique<Queen>(color, false, square);
    }
    if (type == "King")
    {
        return std::make_unique<King>(color, false, square);
    }
    return nullptr;
}

const PieceSnapshot *findPieceSnapshotAt(const GameSnapshot &snapshot, const std::string &square)
{
    for (const PieceSnapshot &piece : snapshot.pieces)
    {
        if (piece.square == square)
        {
            return &piece;
        }
    }
    return nullptr;
}

int signOf(int value)
{
    if (value > 0)
    {
        return 1;
    }
    if (value < 0)
    {
        return -1;
    }
    return 0;
}

bool isBogPieceType(const std::string &type)
{
    return type == "Rook" || type == "Bishop" || type == "Queen";
}

bool bogPieceCanMoveInDirection(const std::string &type, int rowDiff, int colDiff)
{
    const bool straight = rowDiff == 0 || colDiff == 0;
    const bool diagonal = std::abs(rowDiff) == std::abs(colDiff);

    if (type == "Rook")
    {
        return straight;
    }
    if (type == "Bishop")
    {
        return diagonal;
    }
    return type == "Queen" && (straight || diagonal);
}
}

GameState::GameState()
{
    reset();
}

void GameState::reset()
{
    board = std::make_unique<Board>();
    pieces.clear();
    cardManager.setupStartingCards(whiteCards);
    cardManager.setupStartingCards(blackCards);
    lastAction = {};
    retryRestriction = {};
    continuingEffects.clear();
    nextContinuingEffectId = 1;

    std::ostringstream ignoredSetupMessages;
    std::streambuf *previousBuffer = std::cout.rdbuf(ignoredSetupMessages.rdbuf());

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            const std::string &symbol = STARTING_BOARD[static_cast<size_t>(row * 8 + col)];
            if (symbol.empty())
            {
                continue;
            }

            std::string square = squareForFrontendPosition(row, col);
            std::unique_ptr<Pieces> piece = createPiece(symbol[0], square);
            board->placePiece(piece.get(), piece->getType(), square);
            pieces.push_back(std::move(piece));
        }
    }

    std::cout.rdbuf(previousBuffer);
    lastMessage = "New game started. White to move.";
    pendingCheckmatePlayer = "";
}

Board &GameState::getBoard()
{
    return *board;
}

bool GameState::movePiece(const std::string &startSquare, const std::string &endSquare, const std::string &promotionPiece)
{
    std::string retryMessage;
    const std::string movingPlayer = board->getTurn();
    if (!retryAllowsMove(movingPlayer, startSquare, endSquare, retryMessage))
    {
        std::cout << retryMessage << "\n";
        lastMessage = retryMessage;
        return false;
    }

    GameSnapshot beforeAction = createSnapshot();
    Pieces *capturedPiece = board->getPieceAt(endSquare);
    const bool moved = board->movePiece(startSquare, endSquare, promotionPiece);
    if (moved)
    {
        if (capturedPiece != nullptr)
        {
            deactivateContinuingEffectsAt(endSquare);
        }
        moveContinuingEffects(startSquare, endSquare);
        PlayerCardState &movingPlayerState = getCardState(movingPlayer);
        if (!movingPlayerState.drawnThisTurn && !movingPlayerState.deck.empty() && movingPlayerState.hand.size() < 5)
        {
            const std::string drawnCardId = movingPlayerState.deck.back();
            movingPlayerState.deck.pop_back();
            movingPlayerState.hand.push_back(drawnCardId);
            movingPlayerState.drawnThisTurn = true;
            std::cout << movingPlayer << " drew " << drawnCardId << " at the end of their turn.\n";
        }
        cardManager.startTurn(*this, board->getTurn());
        clearRetryRestrictionFor(movingPlayer);
        rememberMoveAction(movingPlayer, startSquare, endSquare, beforeAction);

        if (board->isGameOver() && board->isInCheck(board->getTurn()) &&
            hasEscapeCardsInHand(board->getTurn()))
        {
            pendingCheckmatePlayer = board->getTurn();
            board->setGameOver(false);
        }
    }
    return moved;
}

PlayerCardState &GameState::getCardState(const std::string &player)
{
    if (player == "White")
    {
        return whiteCards;
    }
    if (player == "Black")
    {
        return blackCards;
    }
    throw std::invalid_argument("Unknown player.");
}

const PlayerCardState &GameState::getCardState(const std::string &player) const
{
    if (player == "White")
    {
        return whiteCards;
    }
    if (player == "Black")
    {
        return blackCards;
    }
    throw std::invalid_argument("Unknown player.");
}

const CardManager &GameState::getCardManager() const
{
    return cardManager;
}

CardResult GameState::canPlayCard(const std::string &player, const CardAction &action)
{
    return cardManager.canPlayCard(*this, player, action);
}

CardResult GameState::playCard(const std::string &player, const CardAction &action)
{
    GameSnapshot beforeAction = createSnapshot();
    CardResult result = cardManager.playCard(*this, player, action);
    lastMessage = result.message;
    if (result.success && action.cardId != "knightmare" && action.cardId != "think_again" && action.cardId != "bog")
    {
        clearRetryRestrictionFor(player);
        rememberCardAction(player, action, beforeAction);
    }

    if (result.success && !pendingCheckmatePlayer.empty())
    {
        if (!board->isGameOver())
        {
            pendingCheckmatePlayer = "";
        }
        else if (hasEscapeCardsInHand(pendingCheckmatePlayer))
        {
            board->setGameOver(false);
        }
        else
        {
            pendingCheckmatePlayer = "";
        }
    }

    return result;
}

CardResult GameState::discardCard(const std::string &player, const std::string &cardId)
{
    CardResult result = cardManager.discardCard(*this, player, cardId);
    lastMessage = result.message;
    return result;
}

CardResult GameState::drawCardAsTurn(const std::string &player)
{
    if (board->getTurn() != player)
    {
        return {false, "You can only draw on your own turn.", "", "", ""};
    }

    PlayerCardState &playerState = getCardState(player);
    if (playerState.drawnThisTurn)
    {
        return {false, "You have already drawn a card this turn.", "", "", ""};
    }
    if (playerState.deck.empty())
    {
        return {false, "Your deck is empty.", "", "", ""};
    }
    if (playerState.hand.size() >= 5)
    {
        return {false, "Your hand is full (5 cards max).", "", "", ""};
    }

    const std::string drawnCardId = playerState.deck.back();
    playerState.deck.pop_back();
    playerState.hand.push_back(drawnCardId);
    playerState.drawnThisTurn = true;

    lastMessage = player + " drew " + drawnCardId + ".";
    return {true, lastMessage, "", "", drawnCardId};
}

CardResult GameState::startTurn(const std::string &player)
{
    return cardManager.startTurn(*this, player);
}

bool GameState::removePiece(const std::string &square)
{
    const bool removed = board->removePiece(square);
    if (removed)
    {
        deactivateContinuingEffectsAt(square);
    }
    return removed;
}

void GameState::addContinuingEffect(const std::string &cardId, const std::string &playedBy, const std::string &targetSquare)
{
    continuingEffects.push_back({
        nextContinuingEffectId++,
        cardId,
        playedBy,
        targetSquare,
        true});
}

const std::string &GameState::getLastMessage() const
{
    return lastMessage;
}

void GameState::setLastMessage(const std::string &message)
{
    lastMessage = message;
}

GameSnapshot GameState::createSnapshot() const
{
    GameSnapshot snapshot;
    snapshot.whiteCards = whiteCards;
    snapshot.blackCards = blackCards;
    snapshot.continuingEffects = continuingEffects;
    snapshot.nextContinuingEffectId = nextContinuingEffectId;
    snapshot.turn = board->getTurn();
    snapshot.lastMessage = lastMessage;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            const std::string square = board->getSquare(row, col);
            Pieces *piece = board->getPieceAt(square);
            if (piece == nullptr)
            {
                continue;
            }

            snapshot.pieces.push_back({
                piece->getType(),
                piece->getColor(),
                square,
                piece->getHasMoved(),
                piece->isNeutral(),
                piece->isWarlord()});
        }
    }

    return snapshot;
}

void GameState::restoreSnapshot(const GameSnapshot &snapshot)
{
    board = std::make_unique<Board>();
    pieces.clear();
    whiteCards = snapshot.whiteCards;
    blackCards = snapshot.blackCards;
    continuingEffects = snapshot.continuingEffects;
    nextContinuingEffectId = snapshot.nextContinuingEffectId;

    std::ostringstream ignoredSetupMessages;
    std::streambuf *previousBuffer = std::cout.rdbuf(ignoredSetupMessages.rdbuf());
    for (const PieceSnapshot &pieceSnapshot : snapshot.pieces)
    {
        std::unique_ptr<Pieces> piece = createPieceByType(pieceSnapshot.type, pieceSnapshot.color, pieceSnapshot.square);
        if (piece == nullptr)
        {
            continue;
        }

        piece->setHasMoved(pieceSnapshot.hasMoved);
        piece->setNeutral(pieceSnapshot.neutral);
        piece->setWarlord(pieceSnapshot.warlord);
        board->placePiece(piece.get(), piece->getType(), pieceSnapshot.square);
        pieces.push_back(std::move(piece));
    }
    std::cout.rdbuf(previousBuffer);

    board->setTurn(snapshot.turn);
    lastMessage = snapshot.lastMessage;
}

void GameState::rememberMoveAction(const std::string &player, const std::string &fromSquare, const std::string &toSquare, const GameSnapshot &beforeAction)
{
    lastAction = {};
    lastAction.available = true;
    lastAction.player = player;
    lastAction.actionType = "move";
    lastAction.fromSquare = fromSquare;
    lastAction.toSquare = toSquare;
    lastAction.beforeAction = beforeAction;
}

void GameState::rememberCardAction(const std::string &player, const CardAction &action, const GameSnapshot &beforeAction)
{
    lastAction = {};
    lastAction.available = true;
    lastAction.player = player;
    lastAction.actionType = "card";
    lastAction.cardId = action.cardId;
    lastAction.fromSquare = action.fromSquare;
    lastAction.toSquare = action.targetSquare;
    lastAction.beforeAction = beforeAction;
}

bool GameState::canReactAfterOpponentTurn(const std::string &player) const
{
    if (!lastAction.available || lastAction.player == player)
    {
        return false;
    }
    return board->getTurn() == player;
}

bool GameState::canPlayAfterOwnMove(const std::string &player) const
{
    return lastAction.available &&
        lastAction.player == player &&
        lastAction.actionType == "move" &&
        board->getTurn() != player;
}

bool GameState::canKnightmareCancel(const std::string &player) const
{
    return canReactAfterOpponentTurn(player);
}

bool GameState::canBogLastMove(const std::string &player) const
{
    if (!canReactAfterOpponentTurn(player) || lastAction.actionType != "move")
    {
        return false;
    }

    const PieceSnapshot *movedPiece = findPieceSnapshotAt(lastAction.beforeAction, lastAction.fromSquare);
    if (movedPiece == nullptr || movedPiece->color != lastAction.player || !isBogPieceType(movedPiece->type))
    {
        return false;
    }

    const int startRow = board->getRow(lastAction.fromSquare);
    const int startCol = board->getCol(lastAction.fromSquare);
    const int endRow = board->getRow(lastAction.toSquare);
    const int endCol = board->getCol(lastAction.toSquare);
    const int rowDiff = endRow - startRow;
    const int colDiff = endCol - startCol;
    const int distance = std::max(std::abs(rowDiff), std::abs(colDiff));

    return distance >= 2 && bogPieceCanMoveInDirection(movedPiece->type, rowDiff, colDiff);
}

bool GameState::retryAllowsMove(const std::string &player, const std::string &fromSquare, const std::string &toSquare, std::string &message) const
{
    if (!retryRestriction.active || retryRestriction.player != player)
    {
        return true;
    }

    if (!retryRestriction.forbiddenMoveTo.empty() &&
        fromSquare == retryRestriction.requiredMoveFrom &&
        toSquare == retryRestriction.forbiddenMoveTo)
    {
        message = retryRestriction.cardName + " prevents repeating the canceled move.";
        return false;
    }

    return true;
}

bool GameState::retryAllowsCard(const std::string &player, const std::string &cardId, std::string &message) const
{
    if (!retryRestriction.active || retryRestriction.player != player)
    {
        return true;
    }

    if (!retryRestriction.forbiddenCardId.empty() && cardId == retryRestriction.forbiddenCardId)
    {
        message = retryRestriction.cardName + " prevents replaying the canceled card.";
        return false;
    }

    return true;
}

void GameState::clearRetryRestrictionFor(const std::string &player)
{
    if (retryRestriction.active && retryRestriction.player == player)
    {
        retryRestriction = {};
    }
}

void GameState::moveContinuingEffects(const std::string &fromSquare, const std::string &toSquare)
{
    for (ContinuingEffect &effect : continuingEffects)
    {
        if (effect.active && effect.targetSquare == fromSquare)
        {
            effect.targetSquare = toSquare;
        }
    }
}

void GameState::deactivateContinuingEffectsAt(const std::string &square)
{
    for (ContinuingEffect &effect : continuingEffects)
    {
        if (effect.active && effect.targetSquare == square)
        {
            effect.active = false;
            PlayerCardState &playerState = getCardState(effect.playedBy);
            auto card = std::find(playerState.activePile.begin(), playerState.activePile.end(), effect.cardId);
            if (card != playerState.activePile.end())
            {
                playerState.discardPile.push_back(effect.cardId);
                playerState.activePile.erase(card);
            }
        }
    }
}

bool GameState::cancelLastActionWithKnightmare(const std::string &player, const PlayerCardState &playerStateAfterCost, const std::string &cardName, std::string &message)
{
    if (!canKnightmareCancel(player))
    {
        message = cardName + " can only cancel the opponent's last action after their turn.";
        return false;
    }

    const LastActionSnapshot canceledAction = lastAction;
    const std::string opponent = canceledAction.player;

    restoreSnapshot(canceledAction.beforeAction);
    getCardState(player) = playerStateAfterCost;

    retryRestriction = {};
    retryRestriction.active = true;
    retryRestriction.player = opponent;
    retryRestriction.cardName = cardName;
    if (canceledAction.actionType == "move")
    {
        retryRestriction.requiredMoveFrom = canceledAction.fromSquare;
        retryRestriction.forbiddenMoveTo = canceledAction.toSquare;
    }
    else if (canceledAction.actionType == "card")
    {
        retryRestriction.requiredMoveFrom = canceledAction.fromSquare;
        retryRestriction.forbiddenCardId = canceledAction.cardId;
    }

    board->setTurn(opponent);
    lastAction = {};

    message = player + " played " + cardName + ". " + opponent + "'s " +
        (canceledAction.actionType == "card" ? canceledAction.cardId : "move") +
        " was canceled.";
    lastMessage = message;
    return true;
}

std::vector<std::string> GameState::getNeutralSquares() const
{
    std::vector<std::string> neutralSquares;
    for (const ContinuingEffect &effect : continuingEffects)
    {
        if (effect.active && effect.cardId == "neutrality")
        {
            Pieces *piece = board->getPieceAt(effect.targetSquare);
            if (piece != nullptr && piece->isNeutral())
            {
                neutralSquares.push_back(effect.targetSquare);
            }
        }
    }
    return neutralSquares;
}

std::vector<std::string> GameState::getWarlordSquares() const
{
    std::vector<std::string> warlordSquares;
    for (const ContinuingEffect &effect : continuingEffects)
    {
        if (effect.active && effect.cardId == "warlord")
        {
            Pieces *piece = board->getPieceAt(effect.targetSquare);
            if (piece != nullptr && piece->isWarlord())
            {
                warlordSquares.push_back(effect.targetSquare);
            }
        }
    }
    return warlordSquares;
}

std::vector<ContinuingEffect> GameState::getContinuingEffects() const
{
    return continuingEffects;
}

bool GameState::hasEscapeCardsInHand(const std::string &player) const
{
    const PlayerCardState &state = getCardState(player);
    for (const std::string &cardId : state.hand)
    {
        const CardDefinition *def = cardManager.getDefinition(cardId);
        if (def && def->timing == CardTiming::AFTER_OPPONENT_TURN)
        {
            return true;
        }
    }
    return false;
}

std::string GameState::getPendingCheckmatePlayer() const
{
    return pendingCheckmatePlayer;
}

void GameState::claimCheckmate()
{
    if (!pendingCheckmatePlayer.empty())
    {
        const std::string loser = pendingCheckmatePlayer;
        const std::string winner = board->getoponentColor(loser);
        board->setGameOver(true);
        pendingCheckmatePlayer = "";
        lastMessage = "Checkmate! " + winner + " wins!";
    }
}

bool GameState::bogLastMove(const std::string &player, const PlayerCardState &playerStateAfterCost, std::string &message)
{
    if (!canBogLastMove(player))
    {
        message = "Bog can only slow an opponent rook, bishop, or queen move of two or more squares.";
        return false;
    }

    const GameSnapshot beforeBog = createSnapshot();
    const LastActionSnapshot boggedAction = lastAction;
    const std::string opponent = boggedAction.player;

    const int startRow = board->getRow(boggedAction.fromSquare);
    const int startCol = board->getCol(boggedAction.fromSquare);
    const int endRow = board->getRow(boggedAction.toSquare);
    const int endCol = board->getCol(boggedAction.toSquare);
    const std::string oneStepSquare = board->getSquare(
        startRow + signOf(endRow - startRow),
        startCol + signOf(endCol - startCol));

    restoreSnapshot(boggedAction.beforeAction);
    getCardState(player) = playerStateAfterCost;
    board->setTurn(opponent);

    std::ostringstream ignoredMoveMessages;
    std::streambuf *previousBuffer = std::cout.rdbuf(ignoredMoveMessages.rdbuf());
    const bool moved = board->movePiece(boggedAction.fromSquare, oneStepSquare, "Queen");
    std::cout.rdbuf(previousBuffer);

    if (!moved)
    {
        restoreSnapshot(beforeBog);
        message = "Bog could not shorten that move legally.";
        return false;
    }

    board->setTurn(player);
    retryRestriction = {};
    lastAction = {};

    message = player + " played Bog. " + opponent + "'s " + boggedAction.toSquare +
        " move was slowed to " + oneStepSquare + ".";
    lastMessage = message;
    return true;
}
