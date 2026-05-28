#pragma once

#include <memory>
#include <string>
#include <vector>

#include "CardAction.hpp"
#include "CardManager.hpp"
#include "CardResult.hpp"
#include "ChessBoard.hpp"
#include "ChessPiece.hpp"
#include "PlayerCardState.hpp"

struct PieceSnapshot
{
    std::string type;
    std::string color;
    std::string square;
    bool hasMoved = false;
    bool neutral = false;
    bool warlord = false;
};

struct ContinuingEffect
{
    int id = 0;
    std::string cardId;
    std::string playedBy;
    std::string targetSquare;
    bool active = false;
};

struct GameSnapshot
{
    std::vector<PieceSnapshot> pieces;
    PlayerCardState whiteCards;
    PlayerCardState blackCards;
    std::vector<ContinuingEffect> continuingEffects;
    int nextContinuingEffectId = 1;
    std::string turn;
    std::string lastMessage;
};

struct LastActionSnapshot
{
    bool available = false;
    std::string player;
    std::string actionType;
    std::string fromSquare;
    std::string toSquare;
    std::string cardId;
    GameSnapshot beforeAction;
};

struct RetryRestriction
{
    bool active = false;
    std::string player;
    std::string cardName;
    std::string requiredMoveFrom;
    std::string forbiddenMoveTo;
    std::string forbiddenCardId;
};

class GameState
{
private:
    std::unique_ptr<Board> board;
    std::vector<std::unique_ptr<Pieces>> pieces;
    CardManager cardManager;
    PlayerCardState whiteCards;
    PlayerCardState blackCards;
    std::string lastMessage;
    LastActionSnapshot lastAction;
    RetryRestriction retryRestriction;
    std::vector<ContinuingEffect> continuingEffects;
    int nextContinuingEffectId = 1;
    std::string pendingCheckmatePlayer;

    bool hasEscapeCardsInHand(const std::string &player) const;
    GameSnapshot createSnapshot() const;
    void restoreSnapshot(const GameSnapshot &snapshot);
    void rememberMoveAction(const std::string &player, const std::string &fromSquare, const std::string &toSquare, const GameSnapshot &beforeAction);
    void rememberCardAction(const std::string &player, const CardAction &action, const GameSnapshot &beforeAction);
    bool retryAllowsMove(const std::string &player, const std::string &fromSquare, const std::string &toSquare, std::string &message) const;
    void clearRetryRestrictionFor(const std::string &player);
    void moveContinuingEffects(const std::string &fromSquare, const std::string &toSquare);
    void deactivateContinuingEffectsAt(const std::string &square);

public:
    GameState();

    void reset();
    Board &getBoard();
    bool movePiece(const std::string &startSquare, const std::string &endSquare, const std::string &promotionPiece);
    PlayerCardState &getCardState(const std::string &player);
    const PlayerCardState &getCardState(const std::string &player) const;
    const CardManager &getCardManager() const;
    CardResult canPlayCard(const std::string &player, const CardAction &action);
    CardResult playCard(const std::string &player, const CardAction &action);
    CardResult discardCard(const std::string &player, const std::string &cardId);
    CardResult drawCardAsTurn(const std::string &player);
    CardResult startTurn(const std::string &player);
    bool removePiece(const std::string &square);
    void addContinuingEffect(const std::string &cardId, const std::string &playedBy, const std::string &targetSquare);
    bool canPlayAfterOwnMove(const std::string &player) const;
    bool canReactAfterOpponentTurn(const std::string &player) const;
    bool canKnightmareCancel(const std::string &player) const;
    bool canBogLastMove(const std::string &player) const;
    bool retryAllowsCard(const std::string &player, const std::string &cardId, std::string &message) const;
    bool cancelLastActionWithKnightmare(const std::string &player, const PlayerCardState &playerStateAfterCost, const std::string &cardName, std::string &message);
    bool bogLastMove(const std::string &player, const PlayerCardState &playerStateAfterCost, std::string &message);
    std::vector<std::string> getNeutralSquares() const;
    std::vector<std::string> getWarlordSquares() const;
    std::vector<ContinuingEffect> getContinuingEffects() const;
    const std::string &getLastMessage() const;
    void setLastMessage(const std::string &message);
    std::string getPendingCheckmatePlayer() const;
    void claimCheckmate();
};
