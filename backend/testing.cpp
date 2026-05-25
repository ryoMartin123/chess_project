#include <iostream>
#include <string>
#include "ChessPiece.hpp"
#include "ChessBoard.hpp"

void printScenario(std::string name)
{
    std::cout << "\n=== " << name << " ===\n";
}

void printResult(std::string label, bool actual, bool expected)
{
    if (actual == expected)
    {
        std::cout << "PASS: " << label << "\n";
    }
    else
    {
        std::cout << "FAIL: " << label << " expected " << expected << " but got " << actual << "\n";
    }
}

int main()
{
    printScenario("White pawn promotes and then moves like a queen");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "a7");
        King blackKing("Black", false, "h6");
        Knight blackKnight("Black", false, "g7");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());

        bool promotionMove = board.movePiece("a7", "a8", "Queen");
        printResult("White pawn should promote on a8", promotionMove, true);

        bool blackMove = board.movePiece("g7", "f5");
        printResult("Black should make a legal move", blackMove, true);

        bool queenMove = board.movePiece("a8", "d5");
        printResult("Promoted piece should move diagonally like a queen", queenMove, true);
    }

    printScenario("Black pawn promotes and then moves like a queen");
    {
        Board board;
        King whiteKing("White", false, "h2");
        Knight whiteKnight("White", false, "g2");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "a2");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White setup move should be legal", board.movePiece("g2", "f4"), true);
        printResult("Black pawn should promote on a1", board.movePiece("a2", "a1", "Queen"), true);
        printResult("White reply should be legal", board.movePiece("f4", "h5"), true);
        printResult("Black promoted piece should move diagonally like a queen", board.movePiece("a1", "d4"), true);
    }

    printScenario("Pawn promotes when capturing onto the final rank");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "b7");
        King blackKing("Black", false, "h6");
        Rook blackRook("Black", false, "a8");
        Knight blackKnight("Black", false, "g7");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());
        board.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());

        printResult("White pawn should capture and promote on a8", board.movePiece("b7", "a8", "Queen"), true);
        printResult("Black reply should be legal after capture promotion", board.movePiece("g7", "f5"), true);
        printResult("Capture-promoted piece should move backwards like a queen", board.movePiece("a8", "a7"), true);
    }

    printScenario("Promotion can immediately give check");
    {
        Board board;
        King whiteKing("White", false, "a1");
        Pawn whitePawn("White", false, "b7");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printResult("Promotion move should be legal", board.movePiece("b7", "b8", "Queen"), true);
        printResult("Promoted queen should place Black in check", board.isInCheck("Black"), true);
    }

    printScenario("Pawn does not promote before reaching the final rank");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "a6");
        King blackKing("Black", false, "h8");
        Knight blackKnight("Black", false, "g8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());

        printResult("White pawn should advance to a7", board.movePiece("a6", "a7"), true);
        printResult("Black reply should be legal", board.movePiece("g8", "f6"), true);
        printResult("Pawn on a7 should not move backwards like a queen", board.movePiece("a7", "a6"), false);
    }

    printScenario("Player selects knight promotion");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "a7");
        King blackKing("Black", false, "h8");
        Knight blackKnight("Black", false, "g8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackKnight, blackKnight.getType(), blackKnight.getSquare());

        printResult("White should promote to a knight", board.movePiece("a7", "a8", "Knight"), true);
        printResult("Black reply should be legal after knight promotion", board.movePiece("g8", "f6"), true);
        printResult("Promoted knight should move in an L shape", board.movePiece("a8", "b6"), true);
    }

    printScenario("Invalid promotion choice does not move the pawn");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "a7");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printResult("A pawn cannot promote to a king", board.movePiece("a7", "a8", "King"), false);
        printResult("Pawn remains available for a legal choice", board.movePiece("a7", "a8", "Knight"), true);
    }

    return 0;
}
