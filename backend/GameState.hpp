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

class GameState
{
private:
    std::unique_ptr<Board> board;
    std::vector<std::unique_ptr<Pieces>> pieces;
    CardManager cardManager;
    PlayerCardState whiteCards;
    PlayerCardState blackCards;
    std::string lastMessage;

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
    CardResult startTurn(const std::string &player);
    const std::string &getLastMessage() const;
    void setLastMessage(const std::string &message);
};
