
#include <iostream>
#include "ChessPiece.hpp"
#include "ChessBoard.hpp"
#include "Move.hpp"

int main()
{

    Board daBoard;
    
    Knight blackKnight("Black", false, "d5");
    Move knightMove(blackKnight.getSquare(), "f3");

    Pawn whitePawn("White", false, "f3");
    Move pawnMove(whitePawn.getSquare(), "f4");


    daBoard.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());

    if (whitePawn.isvalidMove(daBoard.getRow(pawnMove.getstartSquare()), daBoard.getCol(pawnMove.getstartSquare()),
                              daBoard.getRow(pawnMove.getendSquare()), daBoard.getCol(pawnMove.getendSquare()), whitePawn.getColor(),
                              daBoard.isEmpty(pawnMove.getendSquare()), daBoard.isEnemy(pawnMove.getendSquare(), whitePawn.getColor()), daBoard.pathClear(pawnMove.getstartSquare(), pawnMove.getendSquare())))
    {
        daBoard.movePiece(whitePawn.getType(), pawnMove.getstartSquare(), pawnMove.getendSquare());
        std::cout << "Yes!\n";
    }

    else
    {
        std::cout << "That is not a valid pawn move!";
        return false;
    }

    daBoard.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());
    
    if (blackKnight.isvalidMove(daBoard.getRow(knightMove.getstartSquare()), daBoard.getCol(knightMove.getstartSquare()),
                              daBoard.getRow(knightMove.getendSquare()), daBoard.getCol(knightMove.getendSquare()), blackKnight.getColor(),
                              daBoard.isEmpty(knightMove.getendSquare()), daBoard.isEnemy(knightMove.getendSquare(), blackKnight.getColor()), daBoard.pathClear(knightMove.getstartSquare(), knightMove.getendSquare())))
    {
        daBoard.movePiece(blackKnight.getType(), knightMove.getstartSquare(), knightMove.getendSquare());
        std::cout << "Yes!\n";
    }

    else
    {
        std::cout << "That is not a valid " << blackKnight.getType() << " move!\n";
        return false;
    }


}



