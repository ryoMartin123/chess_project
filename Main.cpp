
#include <iostream>
#include "ChessPiece.hpp"
#include "ChessBoard.hpp"
#include "Move.hpp"

int main()
{

    Board daBoard;

    King blackKing("Black", false, "c2");
    Move kingMove(blackKing.getSquare(), "d4");

    Pawn whitePawn("White", false, "d2");
    Move pawnMove(whitePawn.getSquare(), "d3");

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

    daBoard.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

    if (blackKing.isvalidMove(daBoard.getRow(kingMove.getstartSquare()), daBoard.getCol(kingMove.getstartSquare()),
                              daBoard.getRow(kingMove.getendSquare()), daBoard.getCol(kingMove.getendSquare()), blackKing.getColor(),
                              daBoard.isEmpty(kingMove.getendSquare()), daBoard.isEnemy(kingMove.getendSquare(), blackKing.getColor()), daBoard.pathClear(kingMove.getstartSquare(), kingMove.getendSquare())))
    {
        daBoard.movePiece(blackKing.getType(), kingMove.getstartSquare(), kingMove.getendSquare());
        std::cout << "Yes!\n";
    }

    else
    {
        std::cout << "That is not a valid " << blackKing.getType() << " move!\n";
        return false;
    }
}
