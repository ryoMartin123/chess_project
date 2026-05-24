#pragma once

#include <iostream>
#include <vector>
#include "ChessPiece.hpp"

class Board
{
private:
    std::string square;
    std::string turn;
    Pieces *board[8][8];

    void printInvalidSquare(std::string square);
    void printInvalidStartSquare(std::string square);
    void printInvalidEndSquare(std::string square);
    void printNoPieceOnSquare(std::string square);
    void printSameSquareMove();
    void printWrongTurn();
    void printStillTurn();
    void printNextTurn();
    void printIllegalMove(Pieces *piece);
    void printIllegalCapture(Pieces *piece);
    void printOwnPieceOnDestination(std::string square);
    void printMoveSuccess(Pieces *piece, std::string startSquare, std::string endSquare);
    void printCaptureSuccess(Pieces *attacker, Pieces *target, std::string endSquare);
    void printSelfCheckMove();
    void printNoKing();

public:
    Board();

    bool isValid(std::string square);
    int getCol(std::string square);
    int getRow(std::string square);
    bool isEmpty(std::string square);
    bool isEnemy(std::string square, std::string color);
    bool pathClear(std::string startSquare, std::string endSquare);
    void placePiece(Pieces *piece, std::string pieceType, std::string square);
    void movePiece(std::string startSquare, std::string endSquare);

    bool isInCheck(std::string color);
};
