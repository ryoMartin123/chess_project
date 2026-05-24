#include <iostream>
#include "ChessPiece.hpp"
#include "ChessBoard.hpp"

void printScenario(std::string name)
{
    std::cout << "\n=== " << name << " ===\n";
}

int main()
{
    printScenario("King tries to move into rook attack");
    {
        Board board;
        King whiteKing("White", false, "e1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "e8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        board.movePiece("e1", "e2");
    }

    printScenario("Pinned piece moves away and exposes king");
    {
        Board board;
        King whiteKing("White", false, "e1");
        King blackKing("Black", false, "h8");
        Rook whiteRook("White", false, "e2");
        Rook blackRook("Black", false, "e8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        board.movePiece("e2", "f2");
    }

    printScenario("Pinned piece moves but still blocks the check line");
    {
        Board board;
        King whiteKing("White", false, "e1");
        King blackKing("Black", false, "h8");
        Rook whiteRook("White", false, "e2");
        Rook blackRook("Black", false, "e8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        board.movePiece("e2", "e3");
    }

    printScenario("Capture is rejected because it exposes king");
    {
        Board board;
        King whiteKing("White", false, "e1");
        King blackKing("Black", false, "h8");
        Rook whiteRook("White", false, "e2");
        Rook blackRook("Black", false, "e8");
        Knight blackKnight("Black", false, "f2");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());
        board.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());

        board.movePiece("e2", "f2");
    }

    return 0;
}
