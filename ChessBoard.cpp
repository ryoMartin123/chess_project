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
        return false;
    }
}

bool Board::isEnemy(std::string square, std::string yourColor) {

Pieces *targetPiece = board[getRow(square)][getCol(square)];

    if (targetPiece == nullptr)
    {
        std::cout << "This square does not have an enemy piece on it!\n";
        return false;
    }
    else
    {
        std::string targetColor = targetPiece->getColor();
        if (yourColor != targetColor) {
            return true;
        }
        else {
            std::cout << "This square has your own piece on it!\n";
            return false;
        }
    }
}

bool Board::pathClear(std::string startSquare, std::string endSquare)
{
    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);

    int rowDiff = endRow - startRow;
    int colDiff = endCol - startCol;

    bool straightMove = rowDiff == 0 || colDiff == 0;
    bool diagonalMove = rowDiff == colDiff || rowDiff == -colDiff;

    if (!straightMove && !diagonalMove)
    {
        return false;
    }

    int rowStep = 0;
    int colStep = 0;

    if (rowDiff > 0)
    {
        rowStep = 1;
    }
    else if (rowDiff < 0)
    {
        rowStep = -1;
    }

    if (colDiff > 0)
    {
        colStep = 1;
    }
    else if (colDiff < 0)
    {
        colStep = -1;
    }

    int currentRow = startRow + rowStep;
    int currentCol = startCol + colStep;

    while (currentRow != endRow || currentCol != endCol)
    {
        if (board[currentRow][currentCol] != nullptr)
        {
            return false;
        }

        currentRow += rowStep;
        currentCol += colStep;
    }

    return true;
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

    Pieces *movingPiece = board[getRow(startSquare)][getCol(startSquare)];
    Pieces *targetPiece = board[getRow(endSquare)][getCol(endSquare)];

    if (targetPiece != nullptr && movingPiece->getColor() == targetPiece->getColor())
    {
        std::cout << "Cannot move to " << endSquare << " because your own piece is there.\n";
        return;
    }

    if (targetPiece != nullptr)
    {
        targetPiece->capture();
        std::cout << "Your " << pieceType << " captured the " << targetPiece->getColor() << " " << targetPiece->getType() << " on " << endSquare << ".\n";
    }

    board[getRow(endSquare)][getCol(endSquare)] = movingPiece;
    board[getRow(startSquare)][getCol(startSquare)] = nullptr;
    board[getRow(endSquare)][getCol(endSquare)]->setSquare(endSquare);

    std::cout << "You moved your " << pieceType << " from " << startSquare << " to " << endSquare << ".\n";
}
