#include <iostream>
#include "ChessPiece.hpp"
#include <algorithm>

std::vector<std::string> piece_types = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
std::vector<std::string> piece_color = {"White", "Black"};

Pieces::Pieces(std::string type, std::string color, bool captured, std::string square, bool hasMoved)
{
    this->type = type;
    this->color = color;
    this->captured = captured;
    this->square = square;
    this->hasMoved = hasMoved;
    this->neutral = false;
    this->warlord = false;
}

Pawn::Pawn(std::string color, bool captured, std::string square) : Pieces("Pawn", color, captured, square, false)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Knight::Knight(std::string color, bool captured, std::string square) : Pieces("Knight", color, captured, square, false)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Bishop::Bishop(std::string color, bool captured, std::string square) : Pieces("Bishop", color, captured, square, false)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Rook::Rook(std::string color, bool captured, std::string square) : Pieces("Rook", color, captured, square, false)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

Queen::Queen(std::string color, bool captured, std::string square) : Pieces("Queen", color, captured, square, false)
{
    this->color = color;
    this->captured = captured;
    this->square = square;
}

King::King(std::string color, bool captured, std::string square) : Pieces("King", color, captured, square, false)
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

bool Pieces::getHasMoved() {
    return hasMoved;
}
void Pieces::markMoved() {
    hasMoved = true;
}
void Pieces::setHasMoved(bool moved) {
    hasMoved = moved;
}
bool Pieces::isNeutral() const {
    return neutral;
}
void Pieces::setNeutral(bool neutral) {
    this->neutral = neutral;
}
bool Pieces::isWarlord() const {
    return warlord;
}
void Pieces::setWarlord(bool warlord) {
    this->warlord = warlord;
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

void Pieces::undoCapture() {
    captured = false;
}

bool Pawn::isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear)
{
    bool pawnForward = false;
    bool pawnForward2 = false;
    bool pawnCapture = false;
    // bool pawnEn_passant = false;
    // bool Promotion = false;
    bool validMove = false;

    if (color == "White" && startRow == 1)
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

    if (color == "Black" && startRow == 6)
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
    (void)startRow;
    (void)startCol;
    (void)endRow;
    (void)endCol;
    (void)color;
    (void)targetEmpty;
    (void)targetEnemy;
    (void)pathClear;
    return false;
}

bool Knight::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                         std::string color, bool targetEmpty,
                         bool targetEnemy, bool pathClear)
{
    // knight logic
    (void)color;
    (void)pathClear;
    bool knightMove = false;

    if (((startCol + 2 == endCol) || (startCol - 2 == endCol)) && ((startRow + 1 == endRow) || (startRow - 1 == endRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            knightMove = true;
        }
    }

    if (((startCol + 1 == endCol) || (startCol - 1 == endCol)) && ((startRow + 2 == endRow) || (startRow - 2 == endRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            knightMove = true;
        }
    }
    return knightMove;
}

bool Bishop::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                         std::string color, bool targetEmpty,
                         bool targetEnemy, bool pathClear)
{
    // bishop logic
    (void)color;
    bool bishopMove = false;

    if ((startRow < endRow && startCol != endCol && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            bishopMove = true;
        }
    }

    if ((startRow > endRow && startCol != endCol && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            bishopMove = true;
        }
    }

    if ((startCol > endCol && startRow != endRow && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            bishopMove = true;
        }
    }

    if ((startCol < endCol && startRow != endRow && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            bishopMove = true;
        }
    }

    return bishopMove;
}

bool Rook::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                       std::string color, bool targetEmpty,
                       bool targetEnemy, bool pathClear)
{
    // rook logic
    (void)color;
    bool rookMove = false;

    if (startRow != endRow && startCol == endCol)
    {
        rookMove = true;
    }

    if (startCol != endCol && startRow == endRow)
    {
        rookMove = true;
    }

    return rookMove && pathClear && (targetEmpty || targetEnemy);
}

bool Queen::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                        std::string color, bool targetEmpty,
                        bool targetEnemy, bool pathClear)
{
    // queen logic
    (void)color;

    bool queenMove = false;

    if ((startRow != endRow && startCol == endCol) && (pathClear && (targetEmpty || targetEnemy)))
    {
        queenMove = true;
    }

    if ((startCol != endCol && startRow == endRow) && (pathClear && (targetEmpty || targetEnemy)))
    {
        queenMove = true;
    }

    if ((startRow < endRow && startCol != endCol && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            queenMove = true;
        }
    }

    if ((startRow > endRow && startCol != endCol && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            queenMove = true;
        }
    }

    if ((startCol > endCol && startRow != endRow && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            queenMove = true;
        }
    }

    if ((startCol < endCol && startRow != endRow && pathClear) && (abs(endCol - startCol) == abs(endRow - startRow)))
    {
        if (targetEmpty || targetEnemy)
        {
            queenMove = true;
        }
    }

    return queenMove;
}

bool King::isvalidMove(int startRow, int startCol, int endRow, int endCol,
                       std::string color, bool targetEmpty,
                       bool targetEnemy, bool pathClear)
{
    // king logic
    (void)color;
    (void)pathClear;

    bool kingMove = false;

    int colDif = endCol - startCol;
    int rowDif = endRow - startRow;

    if ((abs(colDif) < 2 && abs(rowDif) < 2) && (rowDif != 0 || colDif != 0))
    {
        kingMove = true;
    }

    return kingMove && (targetEmpty || targetEnemy);
}
