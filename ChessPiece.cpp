#include <iostream>
#include "ChessPiece.hpp"
#include <algorithm>

std::vector<std::string> piece_types = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
std::vector<std::string> piece_color = {"White", "Black"};

Pieces::Pieces(std::string type, std::string color, bool captured, std::string square)
{
    this->type = type;
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Pawn::Pawn(std::string color, bool captured, std::string square) : Pieces("Pawn", color, captured, square)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Knight::Knight(std::string color, bool captured, std::string square) : Pieces("Knight", color, captured, square)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Bishop::Bishop(std::string color, bool captured, std::string square) : Pieces("Bishop", color, captured, square)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Rook::Rook(std::string color, bool captured, std::string square) : Pieces("Rook", color, captured, square)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Queen::Queen(std::string color, bool captured, std::string square) : Pieces("Queen", color, captured, square)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

King::King(std::string color, bool captured, std::string square) : Pieces("King", color, captured, square)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

bool Pieces::isValid(std::string square)
{

    std::vector<char> char_piece_square = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    std::vector<char> num_piece_square = {'1', '2', '3', '4', '5', '6', '7', '8'};

    bool typeIsValid = std::find(piece_types.begin(), piece_types.end(), type) != piece_types.end();
    bool colorIsValid = std::find(piece_color.begin(), piece_color.end(), color) != piece_color.end();
    bool squareIsValid;
    if ((square.size() == 2) && (std::find(char_piece_square.begin(), char_piece_square.end(), (square[0])) != char_piece_square.end()) && (std::find(num_piece_square.begin(), num_piece_square.end(), (square[1])) != num_piece_square.end()))
    {
        squareIsValid = true;
    }
    else
    {
        squareIsValid = false;
    }

    return typeIsValid && colorIsValid && squareIsValid;
}

std::string Pieces::getType()
{
    return type;
}
std::string Pieces::getColor()
{
    return color;
}
std::string Pieces::getSquare()
{
    return square;
}
bool Pieces::isCaptured()
{
    return captured;
}

std::string Pieces::setSquare(std::string newSquare)
{
    square = newSquare;
    return square;
}

void Pieces::capture()
{
    captured = true;
}

bool Pawn::isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear)
{
    bool pawnForward = false;
    bool pawnForward2 = false;
    bool pawnCapture = false;
    // bool pawnEn_passant = false;
    // bool Promotion = false;
    bool validMove = false;

    if (color == "White" && startRow == 2)
    {

        if (endRow == startRow + 2 && endCol == startCol && pathClear && targetEmpty)
        {
            pawnForward2 = true;
        }

        if (endRow == startRow + 1 && endCol == startCol && targetEmpty)
        {
            pawnForward = true;
        }

        if ((endRow == startRow + 1 && ((endCol == startCol + 1) || (endCol == startCol - 1))) && targetEnemy)
        {
            pawnCapture = true;
        }
    }

    else if (color == "White")
    {
        if (endRow == startRow + 1 && endCol == startCol && targetEmpty)
        {
            pawnForward = true;
        }

        if ((endRow == startRow + 1 && ((endCol == startCol + 1) || (endCol == startCol - 1))) && targetEnemy)
        {
            pawnCapture = true;
        }
    }

    if (color == "Black" && startRow == 7)
    {

        if (endRow == startRow - 2 && endCol == startCol && pathClear && targetEmpty)
        {
            pawnForward2 = true;
        }

        if (endRow == startRow - 1 && endCol == startCol && targetEmpty)
        {
            pawnForward = true;
        }

        if ((endRow == startRow - 1 && ((endCol == startCol + 1) || (endCol == startCol - 1))) && targetEnemy)
        {
            pawnCapture = true;
        }
    }

    else if (color == "Black")
    {
        if (endRow == startRow - 1 && endCol == startCol && targetEmpty)
        {
            pawnForward = true;
        }

        if ((endRow == startRow - 1 && ((endCol == startCol + 1) || (endCol == startCol - 1))) && targetEnemy)
        {
            pawnCapture = true;
        }
    }

    validMove = pawnCapture || pawnForward || pawnForward2;
    return validMove;
}

bool Pieces::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                         std::string color, bool targetEmpty,
                         bool targetEnemy, bool pathClear)
{
    return false;
}

bool Knight::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                         std::string color, bool targetEmpty,
                         bool targetEnemy, bool pathClear)
{
    // knight logic
    return false;
}

bool Bishop::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                         std::string color, bool targetEmpty,
                         bool targetEnemy, bool pathClear)
{
    // bishop logic
    return false;
}

bool Rook::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                       std::string color, bool targetEmpty,
                       bool targetEnemy, bool pathClear)
{
    // rook logic
    return false;
}

bool Queen::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                        std::string color, bool targetEmpty,
                        bool targetEnemy, bool pathClear)
{
    // queen logic
    return false;
}

bool King::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                       std::string color, bool targetEmpty,
                       bool targetEnemy, bool pathClear)
{
    // king logic
    return false;
}