#pragma once

#include <iostream>
#include <vector>
#include "ChessPiece.hpp"

class Board
{
private:
    std::string square;
    Pieces *board[8][8];

public:
    Board();

    bool isValid(std::string square);
    int getCol(std::string square);
    int getRow(std::string square);
    bool isEmpty(std::string square);
    bool isEnemy(std::string square, std::string color);
    bool pathClear(std::string startSquare, std::string endSquare);
    void placePiece(Pieces *piece, std::string pieceType, std::string square);
    void movePiece(std::string pieceType, std::string startSquare, std::string endSquare);
};