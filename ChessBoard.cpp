#include <iostream>
#include <vector>
#include <algorithm>
#include "ChessBoard.hpp"
#include "ChessPiece.hpp"

std::vector<int> row = {0, 1, 2, 3, 4, 5, 6, 7};
std::vector<int> col = {0, 1, 2, 3, 4, 5, 6, 7};

Board::Board()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            board[i][j] = nullptr;
        }
    }
}

bool Board::isValid(std::string square)
{
    std::vector<char> char_piece_square = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    std::vector<char> num_piece_square = {'1', '2', '3', '4', '5', '6', '7', '8'};

    if (square.size() != 2)
    {
        return false;
    }

    bool fileIsValid = std::find(char_piece_square.begin(), char_piece_square.end(), square[0]) != char_piece_square.end();

    bool rankIsValid = std::find(num_piece_square.begin(), num_piece_square.end(), square[1]) != num_piece_square.end();

    return fileIsValid && rankIsValid;
}

int Board::getCol(std::string square)
{
    std::vector<char> char_piece_square = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    int col_index;
    for (int i = 0; i < char_piece_square.size(); i++)
    {
        if (char_piece_square[i] == square[0])
        {
            col_index = i;
            break;
        }
    }

    return col_index;
}

int Board::getRow(std::string square)
{
    std::vector<char> num_piece_square = {'1', '2', '3', '4', '5', '6', '7', '8'};
    int row_index;
    for (int i = 0; i < num_piece_square.size(); i++)
    {
        if (num_piece_square[i] == square[1])
        {
            row_index = i;
            break;
        }
    }
    return row_index;
}

bool Board::isEmpty(std::string square)
{

    if (board[getRow(square)][getCol(square)] == nullptr)
    {
        return true;
    }
    else
    {
        std::cout << "This square has a piece on it already!\n";
        return false;
    }
}

bool Board::isEnemy(std::string square, std::string yourColor)
{

    Pieces *targetPiece = board[getRow(square)][getCol(square)];
    std::string targetColor = targetPiece->getColor();

    if ((board[getRow(square)][getCol(square)] != nullptr) && (yourColor != targetColor))
    {
        return true;
    }
    else
    {
        std::cout << "This square does not have an enemy piece on it!\n";
        return false;
    }
}

bool Board::pathClear(std::string startSquare, std::string endSquare)
{
    if ((getRow(startSquare) == getRow(endSquare)) && getCol(endSquare) > getCol(startSquare))
    {
        for (int i = getCol(startSquare) + 1; i < getCol(endSquare); i++)
        {
            if (board[getRow(startSquare)][i] != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    if ((getRow(startSquare) == getRow(endSquare)) && getCol(endSquare) < getCol(startSquare))
    {
        for (int i = getCol(startSquare) + 1; i > getCol(endSquare); i--)
        {
            if (board[getRow(startSquare)][i] != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    if ((getCol(startSquare) == getCol(endSquare)) && getRow(endSquare) > getRow(startSquare))
    {
        for (int i = getRow(startSquare) + 1; i < getRow(endSquare); i++)
        {
            if (board[getRow(startSquare)][i] != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    if ((getCol(startSquare) == getCol(endSquare)) && getRow(endSquare) < getRow(startSquare))
    {
        for (int i = getRow(startSquare) + 1; i > getRow(endSquare); i--)
        {
            if (board[getRow(startSquare)][i] != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    if (((getCol(startSquare)) != getCol(endSquare)) && (getRow(startSquare) != getRow(endSquare)) && (getCol(startSquare) > getCol(endSquare)))
    {
        for (int i = getRow(startSquare) + 1; i < getRow(endSquare); i++)
        {
            if (board[getRow(startSquare)][i] != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    if (((getCol(startSquare)) != getCol(endSquare)) && (getRow(startSquare) != getRow(endSquare)) && (getCol(startSquare) < getCol(endSquare)))
    {
        for (int i = getRow(startSquare) + 1; i > getRow(endSquare); i--)
        {
            if (board[getRow(startSquare)][i] != nullptr)
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

void Board::placePiece(Pieces *piece, std::string pieceType, std::string square)
{

    if (board[getRow(square)][getCol(square)] == nullptr)
    {
        board[getRow(square)][getCol(square)] = piece;
        std::cout << "Your " << pieceType << " is now on " << square << "\n";
    }

    else
    {
        Pieces *existingPiece = board[getRow(square)][getCol(square)];
        std::cout << "There is a " << existingPiece->getType() << " already there!\n";
    }
}

void Board::movePiece(std::string pieceType, std::string startSquare, std::string endSquare)
{

    if (isValid(startSquare) == false)
    {
        std::cout << startSquare << " is not a valid starting square.\n";
        return;
    }

    if (isValid(endSquare) == false)
    {
        std::cout << endSquare << " is not a valid ending square.\n";
        return;
    }

    if (startSquare == endSquare)
    {
        std::cout << "Start square and end square cannot be the same.\n";
        return;
    }

    if (board[getRow(startSquare)][getCol(startSquare)] == nullptr)
    {
        std::cout << "There is no piece on " << startSquare << ".\n";
        return;
    }

    if (board[getRow(endSquare)][getCol(endSquare)] != nullptr)
    {
        std::cout << "Cannot move to " << endSquare << " because there is already a piece there.\n";
        return;
    }

    board[getRow(endSquare)][getCol(endSquare)] = board[getRow(startSquare)][getCol(startSquare)];
    board[getRow(startSquare)][getCol(startSquare)] = nullptr;
    board[getRow(endSquare)][getCol(endSquare)]->setSquare(endSquare);

    std::cout << "You moved your " << pieceType << " from " << startSquare << " to " << endSquare << ".\n";
}
