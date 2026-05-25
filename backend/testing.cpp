#include <algorithm>
#include <iostream>
#include <sstream>
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

void printOutputContains(std::string label, std::string output, std::string expectedText)
{
    bool found = output.find(expectedText) != std::string::npos;

    if (found)
    {
        std::cout << "PASS: " << label << "\n";
    }
    else
    {
        std::cout << "FAIL: " << label << "\n";
        std::cout << "Expected output to contain: " << expectedText << "\n";
        std::cout << "Actual output was: " << output << "\n";
    }
}

void printLegalMoveResult(std::string label, Board &board, std::string startSquare,
                          std::string expectedSquare, bool expected)
{
    std::vector<std::string> moves = board.getLegalMoves(startSquare);
    bool found = std::find(moves.begin(), moves.end(), expectedSquare) != moves.end();
    printResult(label, found, expected);
}

bool movePieceAndCaptureOutput(Board &board, std::string startSquare, std::string endSquare,
                               std::string promotionPiece, std::string &output)
{
    std::ostringstream capturedOutput;
    std::streambuf *oldBuffer = std::cout.rdbuf(capturedOutput.rdbuf());
    bool result = board.movePiece(startSquare, endSquare, promotionPiece);
    std::cout.rdbuf(oldBuffer);
    output = capturedOutput.str();
    return result;
}

void printMoveAndMessageResult(std::string label, Board &board, std::string startSquare,
                               std::string endSquare, bool expectedMoveResult,
                               std::string expectedMessage)
{
    std::string output;
    bool actualMoveResult = movePieceAndCaptureOutput(board, startSquare, endSquare, "", output);
    printResult(label, actualMoveResult, expectedMoveResult);
    printOutputContains(label + " message", output, expectedMessage);
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

        printResult("White pawn should promote on a8", board.movePiece("a7", "a8", "Queen"), true);
        printResult("Black should make a legal move", board.movePiece("g7", "f5"), true);
        printResult("Promoted piece should move diagonally like a queen", board.movePiece("a8", "d5"), true);
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

    printScenario("White castling succeeds on both sides");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "h1");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printLegalMoveResult("White legal moves should include kingside castling", board, "e1", "g1", true);
        printMoveAndMessageResult("White should castle kingside", board, "e1", "g1", true, "White castled kingside!");
    }

    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printMoveAndMessageResult("White should castle queenside", board, "e1", "c1", true, "White castled queenside!");
    }

    printScenario("Black castling succeeds on both sides");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Knight whiteKnight("White", false, "b1");
        King blackKing("Black", false, "e8");
        Rook blackRook("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printResult("White setup move should be legal", board.movePiece("b1", "c3"), true);
        printMoveAndMessageResult("Black should castle kingside", board, "e8", "g8", true, "Black castled kingside!");
    }

    {
        Board board;
        King whiteKing("White", false, "e1");
        Knight whiteKnight("White", false, "b1");
        King blackKing("Black", false, "e8");
        Rook blackRook("Black", false, "a8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printResult("White setup move should be legal", board.movePiece("b1", "c3"), true);
        printMoveAndMessageResult("Black should castle queenside", board, "e8", "c8", true, "Black castled queenside!");
    }

    printScenario("Castling fails when the king has moved");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");
        whiteKing.markMoved();

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printMoveAndMessageResult("White cannot castle after king moved", board, "e1", "c1", false, "your king has already moved.");
    }

    printScenario("Castling fails from an invalid starting square");
    {
        Board board;
        King whiteKing("White", false, "d1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printMoveAndMessageResult("White cannot castle from d1", board, "d1", "b1", false, "valid castling square.");
    }

    printScenario("Castling fails when the rook is missing");
    {
        Board board;
        King whiteKing("White", false, "e1");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printMoveAndMessageResult("White cannot castle without the a1 rook", board, "e1", "c1", false, "rook is not on a1.");
    }

    printScenario("Castling fails when the rook has moved");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");
        whiteRook.markMoved();

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printMoveAndMessageResult("White cannot castle after rook moved", board, "e1", "c1", false, "rook on a1 has already moved.");
    }

    printScenario("Castling fails when a piece blocks the path");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        Bishop whiteBishop("White", false, "c1");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&whiteBishop, whiteBishop.getType(), whiteBishop.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printMoveAndMessageResult("White cannot castle through a blocked path", board, "e1", "c1", false, "piece between the king and rook.");
    }

    printScenario("Castling fails while the king is currently in check");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "e8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printMoveAndMessageResult("White cannot castle while in check", board, "e1", "c1", false, "your king is currently in check.");
    }

    printScenario("Castling fails when the king passes through check");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "d8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printMoveAndMessageResult("White cannot castle through attacked d1", board, "e1", "c1", false, "pass through check on d1.");
    }

    printScenario("Castling fails when the king lands in check");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "a1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "c8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printMoveAndMessageResult("White cannot castle onto attacked c1", board, "e1", "c1", false, "land in check on c1.");
    }

    printScenario("White en passant succeeds immediately");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "e5");
        Knight whiteKnight("White", false, "b1");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "d7");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White setup move should be legal", board.movePiece("b1", "c3"), true);
        printResult("Black pawn should move two squares beside white pawn", board.movePiece("d7", "d5"), true);
        printLegalMoveResult("White legal moves should include en passant", board, "e5", "d6", true);
        printResult("White should capture en passant", board.movePiece("e5", "d6"), true);
        printResult("White pawn should land on d6", whitePawn.getSquare() == "d6", true);
        printResult("Captured black pawn should leave d5 empty", board.isEmpty("d5"), true);
        printResult("Captured black pawn should be marked captured", blackPawn.isCaptured(), true);
    }

    printScenario("Black en passant succeeds immediately");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "e2");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "d4");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White pawn should move two squares beside black pawn", board.movePiece("e2", "e4"), true);
        printResult("Black should capture en passant", board.movePiece("d4", "e3"), true);
        printResult("Black pawn should land on e3", blackPawn.getSquare() == "e3", true);
        printResult("Captured white pawn should leave e4 empty", board.isEmpty("e4"), true);
        printResult("Captured white pawn should be marked captured", whitePawn.isCaptured(), true);
    }

    printScenario("En passant fails when it is not immediate");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "e5");
        Knight whiteKnight("White", false, "b1");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "d7");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White setup move should be legal", board.movePiece("b1", "c3"), true);
        printResult("Black pawn should move two squares", board.movePiece("d7", "d5"), true);
        printResult("White should skip the en passant chance", board.movePiece("c3", "b5"), true);
        printResult("Black should make another legal move", board.movePiece("h8", "g8"), true);
        printResult("White cannot en passant after another move", board.movePiece("e5", "d6"), false);
        printResult("Black pawn should still be on d5", blackPawn.getSquare() == "d5", true);
        printResult("Black pawn should not be captured", blackPawn.isCaptured(), false);
    }

    printScenario("En passant fails when the last pawn moved only one square");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Pawn whitePawn("White", false, "e5");
        Knight whiteKnight("White", false, "b1");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "d6");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White setup move should be legal", board.movePiece("b1", "c3"), true);
        printResult("Black pawn should move one square", board.movePiece("d6", "d5"), true);
        printResult("White cannot en passant a one-square pawn move", board.movePiece("e5", "d6"), false);
        printResult("Black pawn should remain on d5", blackPawn.getSquare() == "d5", true);
        printResult("Black pawn should not be captured", blackPawn.isCaptured(), false);
    }

    printScenario("En passant fails when it would expose the king to check");
    {
        Board board;
        King whiteKing("White", false, "e5");
        Pawn whitePawn("White", false, "d5");
        Knight whiteKnight("White", false, "g1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "a5");
        Pawn blackPawn("Black", false, "c7");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White setup move should be legal", board.movePiece("g1", "f3"), true);
        printResult("Black pawn should move two squares beside pinned pawn", board.movePiece("c7", "c5"), true);
        printResult("White cannot en passant if it exposes check", board.movePiece("d5", "c6"), false);
        printResult("White pawn should stay on d5", whitePawn.getSquare() == "d5", true);
        printResult("Black pawn should stay on c5", blackPawn.getSquare() == "c5", true);
        printResult("Black pawn should not be captured", blackPawn.isCaptured(), false);
    }

    printScenario("En passant prevents a false stalemate");
    {
        Board board;
        King whiteKing("White", false, "f7");
        Knight whiteKnight("White", false, "f6");
        Rook whiteRook("White", false, "d3");
        Pawn whitePawn("White", false, "e2");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "d4");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteKnight, whiteKnight.getType(), whiteKnight.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("White should open Black's en passant response", board.movePiece("e2", "e4"), true);
        printResult("Black should not be stalemated when en passant is legal", board.movePiece("d4", "e3"), true);
    }

    printScenario("Pinned piece cannot move and expose its king");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "e2");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "e8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printLegalMoveResult("Pinned rook legal moves should exclude exposing the king", board, "e2", "a2", false);
        printResult("White cannot move pinned rook away from the file", board.movePiece("e2", "a2"), false);
        printResult("Pinned rook should stay on e2", whiteRook.getSquare() == "e2", true);
    }

    printScenario("Castling kingside fails through attacked f1");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "h1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "f8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printMoveAndMessageResult("White cannot castle through attacked f1", board, "e1", "g1", false, "pass through check on f1.");
    }

    printScenario("Castling kingside fails when landing on attacked g1");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "h1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "g8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printMoveAndMessageResult("White cannot castle onto attacked g1", board, "e1", "g1", false, "land in check on g1.");
    }

    printScenario("Castling can succeed while the rook is attacked");
    {
        Board board;
        King whiteKing("White", false, "e1");
        Rook whiteRook("White", false, "h1");
        King blackKing("Black", false, "a8");
        Rook blackRook("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteRook, whiteRook.getType(), whiteRook.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());

        printMoveAndMessageResult("White can castle even if the rook is attacked", board, "e1", "g1", true, "White castled kingside!");
    }

    printScenario("Promotion can deliver checkmate");
    {
        Board board;
        King whiteKing("White", false, "g6");
        Pawn whitePawn("White", false, "f7");
        King blackKing("Black", false, "h8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        std::string output;
        bool moveResult = movePieceAndCaptureOutput(board, "f7", "f8", "Queen", output);
        printResult("White promotion should checkmate Black", moveResult, true);
        printOutputContains("White promotion checkmate message", output, "Checkmate! White won!");

        bool gameOverMove = movePieceAndCaptureOutput(board, "h8", "h7", "", output);
        printResult("Game should be over after promotion checkmate", gameOverMove, false);
        printOutputContains("Game over message after promotion checkmate", output, "The game is already over.");
    }

    printScenario("A king cannot be captured directly");
    {
        Board board;
        King whiteKing("White", false, "a1");
        Queen whiteQueen("White", false, "e7");
        King blackKing("Black", false, "e8");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whiteQueen, whiteQueen.getType(), whiteQueen.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());

        printLegalMoveResult("Legal moves should exclude the enemy king square", board, "e7", "e8", false);
        printResult("White cannot capture the black king", board.movePiece("e7", "e8"), false);
        printResult("Black king should remain on its square", board.getPieceAt("e8") == &blackKing, true);
    }

    printScenario("squareIsThreatened handles sliding pieces");
    {
        Board board;
        King whiteKing("White", false, "h1");
        King blackKing("Black", false, "h8");
        Rook blackRook("Black", false, "a8");
        Bishop blackBlocker("Black", false, "a4");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackRook, blackRook.getType(), blackRook.getSquare());
        board.placePiece(&blackBlocker, blackBlocker.getType(), blackBlocker.getSquare());

        printResult("Black rook threatens a4 before the blocker", board.squareIsThreatened("a4", "Black"), true);
        printResult("Black rook does not threaten a1 through a blocker", board.squareIsThreatened("a1", "Black"), false);
    }

    printScenario("squareIsThreatened handles pawn attacks");
    {
        Board board;
        King whiteKing("White", false, "a1");
        Pawn whitePawn("White", false, "b3");
        King blackKing("Black", false, "h8");
        Pawn blackPawn("Black", false, "e4");

        board.placePiece(&whiteKing, whiteKing.getType(), whiteKing.getSquare());
        board.placePiece(&whitePawn, whitePawn.getType(), whitePawn.getSquare());
        board.placePiece(&blackKing, blackKing.getType(), blackKing.getSquare());
        board.placePiece(&blackPawn, blackPawn.getType(), blackPawn.getSquare());

        printResult("Black pawn threatens d3 diagonally", board.squareIsThreatened("d3", "Black"), true);
        printResult("Black pawn does not threaten e3 straight ahead", board.squareIsThreatened("e3", "Black"), false);
        printResult("White pawn threatens c4 diagonally", board.squareIsThreatened("c4", "White"), true);
        printResult("White pawn does not threaten b4 straight ahead", board.squareIsThreatened("b4", "White"), false);
    }

    return 0;
}
