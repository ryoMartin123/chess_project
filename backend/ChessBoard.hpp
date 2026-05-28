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
    std::vector<Pieces *> promotedPieceVector;
    bool gameOver;
    std::string lastStartSquare;
    std::string lastEndSquare;
    Pieces *lastMovedPiece;

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
    void printCheck(std::string color);
    void printCheckMate(std::string color);
    void printStaleMate(std::string color);
    void printPromoted(std::string color, std::string promotedPiece);
    void printInvalidPromotion(std::string piece);
    void printGameOver();
    void printIllegalCastle(std::string reason);
    void printSuccessfulCastle(std::string color, std::string startSquare, std::string endSquare);
    bool isValidPromotionChoice(std::string piece);

    void rememberLastMove(Pieces *piece, std::string startSquare, std::string endSquare);
    bool castle(Pieces *king, std::string startSquare, std::string endSquare);
    bool canCastleSafely(Pieces *king, std::string startSquare, std::string endSquare);
    bool canEnPassant(Pieces *movingPiece, std::string startSquare, std::string endSquare);
    bool canWarlordTwoStep(Pieces *movingPiece, std::string startSquare, std::string endSquare);
    bool isLegalMoveForPosition(Pieces *movingPiece, std::string startSquare, std::string endSquare);
    bool createPromotedPiece(std::string piece, std::string color, std::string square);
    bool moveLeavesKingInCheck(Pieces *movingPiece, Pieces *targetPiece, std::string startSquare, std::string endSquare);
    bool finishSuccessfulMove(Pieces *movingPiece, std::string startSquare, std::string endSquare);

public:
    Board();
    ~Board();

    bool isValid(std::string square);
    int getCol(std::string square);
    int getRow(std::string square);
    std::string getSquare(int row, int col);
    std::string getoponentColor(std::string yourColor);
    bool isEmpty(std::string square);
    bool isEnemy(std::string square, std::string color);
    bool pathClear(std::string startSquare, std::string endSquare);
    void placePiece(Pieces *piece, std::string pieceType, std::string square);
    bool removePiece(std::string square);
    Pieces *getPieceAt(std::string square);
    std::string getTurn();
    void setTurn(std::string nextTurn);
    bool isGameOver();
    void setGameOver(bool value);
    std::vector<std::string> getLegalMoves(std::string startSquare);
    bool movePiece(std::string startSquare, std::string endSquare, std::string promotionPiece = "");

    bool isInCheck(std::string color);
    bool hasAnyLegalMove(std::string color);
    bool isCheckmate(std::string color);
    bool isStalemate(std::string color);
    bool squareIsThreatened(std::string square, std::string enemyColor);
};
