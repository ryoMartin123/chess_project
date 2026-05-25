
#include <iostream>
#include "ChessPiece.hpp"
#include "ChessBoard.hpp"
#include "Move.hpp"

int main()
{

    Board daBoard;

    Queen whiteQueen("White", false, "e2");
    Move queenMove(whiteQueen.getSquare(), "f3");

    King blackKing("Black", false, "c5");

    Knight blackKnight("Black", false, "b4");
    Move knightMove(blackKing.getSquare(), "d5");

    daBoard.placePiece(&whiteQueen, whiteQueen.getType(), whiteQueen.getSquare());
    daBoard.movePiece(queenMove.getstartSquare(), queenMove.getendSquare());

    daBoard.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
    
    daBoard.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());
    daBoard.movePiece(knightMove.getstartSquare(), knightMove.getendSquare());



}
