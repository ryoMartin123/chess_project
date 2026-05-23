
#include <iostream>
#include "ChessPiece.hpp"
#include "ChessBoard.hpp"
#include "Move.hpp"

int main()
{

    Board daBoard;

    std::cout << "Yes";

    Knight whiteKnight("White", false, "g1");
    Move knightMove(whiteKnight.getSquare(), "f3");

    Pawn whitePawn("White", false, "g2");
    Move pawnMove(whitePawn.getSquare(), "g3");

    daBoard.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
    daBoard.movePiece(whiteKnight.getType(), knightMove.getstartSquare(), knightMove.getendSquare());

    daBoard.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());

    if (whitePawn.isvalidMove(daBoard.getRow(pawnMove.getstartSquare()), daBoard.getCol(pawnMove.getstartSquare()),
                              daBoard.getRow(pawnMove.getendSquare()), daBoard.getCol(pawnMove.getendSquare()), whitePawn.getColor(),
                              daBoard.isEmpty(pawnMove.getendSquare()), daBoard.isEnemy(pawnMove.getendSquare(), whitePawn.getColor()), daBoard.pathClear(pawnMove.getstartSquare(), pawnMove.getendSquare())))
    {
        daBoard.movePiece(whitePawn.getType(), pawnMove.getstartSquare(), pawnMove.getendSquare());
        std::cout << "Yes!";
    }

    else
    {
        std::cout << "That is not a valid pawn move!";
        return false;
    }
}
