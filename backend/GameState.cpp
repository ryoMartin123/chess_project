#include "GameState.hpp"

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
}

Board &GameState::getBoard()
{
    return *board;
}

bool GameState::movePiece(const std::string &startSquare, const std::string &endSquare, const std::string &promotionPiece)
{
    const bool moved = board->movePiece(startSquare, endSquare, promotionPiece);
    if (moved)
    {
        cardManager.startTurn(*this, board->getTurn());
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
    CardResult result = cardManager.playCard(*this, player, action);
    lastMessage = result.message;
    return result;
}

CardResult GameState::discardCard(const std::string &player, const std::string &cardId)
{
    CardResult result = cardManager.discardCard(*this, player, cardId);
    lastMessage = result.message;
    return result;
}

CardResult GameState::startTurn(const std::string &player)
{
    return cardManager.startTurn(*this, player);
}

const std::string &GameState::getLastMessage() const
{
    return lastMessage;
}

void GameState::setLastMessage(const std::string &message)
{
    lastMessage = message;
}
