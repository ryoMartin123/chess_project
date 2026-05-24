#include <iostream>
#include <vector>
#include <algorithm>
#include "ChessBoard.hpp"
#include "ChessPiece.hpp"

std::vector<int> row = {0, 1, 2, 3, 4, 5, 6, 7};
std::vector<int> col = {0, 1, 2, 3, 4, 5, 6, 7};

Board::Board()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            board[i][j] = nullptr;
        }
    }

    turn = "White";
}

bool Board::isValid(std::string square)
{
    std::vector<char> char_piece_square = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    std::vector<char> num_piece_square = {'1', '2', '3', '4', '5', '6', '7', '8'};

    if (square.size() != 2)
    {
        return false;
    }

    bool fileIsValid = std::find(char_piece_square.begin(), char_piece_square.end(), square[0]) != char_piece_square.end();

    bool rankIsValid = std::find(num_piece_square.begin(), num_piece_square.end(), square[1]) != num_piece_square.end();

    return fileIsValid && rankIsValid;
}

int Board::getCol(std::string square)
{
    std::vector<char> char_piece_square = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    int col_index;
    for (int i = 0; i < char_piece_square.size(); i++)
    {
        if (char_piece_square[i] == square[0])
        {
            col_index = i;
            break;
        }
    }

    return col_index;
}

int Board::getRow(std::string square)
{
    std::vector<char> num_piece_square = {'1', '2', '3', '4', '5', '6', '7', '8'};
    int row_index;
    for (int i = 0; i < num_piece_square.size(); i++)
    {
        if (num_piece_square[i] == square[1])
        {
            row_index = i;
            break;
        }
    }
    return row_index;
}

bool Board::isEmpty(std::string square)
{

    if (board[getRow(square)][getCol(square)] == nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Board::isEnemy(std::string square, std::string yourColor) {

Pieces *targetPiece = board[getRow(square)][getCol(square)];

    if (targetPiece == nullptr)
    {
        return false;
    }
    else
    {
        std::string targetColor = targetPiece->getColor();
        if (yourColor != targetColor) {
            return true;
        }
        else {
            return false;
        }
    }
}

bool Board::pathClear(std::string startSquare, std::string endSquare)
{
    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);

    int rowDiff = endRow - startRow;
    int colDiff = endCol - startCol;

    bool straightMove = rowDiff == 0 || colDiff == 0;
    bool diagonalMove = rowDiff == colDiff || rowDiff == -colDiff;

    if (!straightMove && !diagonalMove)
    {
        return false;
    }

    int rowStep = 0;
    int colStep = 0;

    if (rowDiff > 0)
    {
        rowStep = 1;
    }
    else if (rowDiff < 0)
    {
        rowStep = -1;
    }

    if (colDiff > 0)
    {
        colStep = 1;
    }
    else if (colDiff < 0)
    {
        colStep = -1;
    }

    int currentRow = startRow + rowStep;
    int currentCol = startCol + colStep;

    while (currentRow != endRow || currentCol != endCol)
    {
        if (board[currentRow][currentCol] != nullptr)
        {
            Pieces *piece = board[currentRow][currentCol];
            return false;
        }

        currentRow += rowStep;
        currentCol += colStep;
    }

    return true;
}

void Board::placePiece(Pieces *piece, std::string pieceType, std::string square)
{

    if (isValid(square) == false)
    {
        printInvalidSquare(square);
        return;
    }

    if (board[getRow(square)][getCol(square)] == nullptr)
    {
        board[getRow(square)][getCol(square)] = piece;
        std::cout << "Your " << pieceType << " is now on " << square << "\n";
    }

    else
    {
        Pieces *existingPiece = board[getRow(square)][getCol(square)];
        std::cout << "There is a " << existingPiece->getType() << " already there!\n";
    }
}

void Board::printInvalidSquare(std::string square)
{
    std::cout << square << " is not a valid square.\n";
}

void Board::printInvalidStartSquare(std::string square)
{
    std::cout << square << " is not a valid starting square.\n";
}

void Board::printInvalidEndSquare(std::string square)
{
    std::cout << square << " is not a valid ending square.\n";
}

void Board::printNoPieceOnSquare(std::string square)
{
    std::cout << "There is no piece on " << square << ".\n";
}

void Board::printSameSquareMove()
{
    std::cout << "Start square and end square cannot be the same.\n";
}

void Board::printWrongTurn()
{
    std::cout << "It is " << turn << "'s turn!\n";
}

void Board::printStillTurn()
{
    std::cout << "It is still " << turn << "'s turn\n";
}

void Board::printNextTurn()
{
    std::cout << "It is " << turn << "'s turn\n";
}

void Board::printIllegalMove(Pieces *piece)
{
    std::cout << "This is an illegal " << piece->getType() << " move!\n";
}

void Board::printIllegalCapture(Pieces *piece)
{
    std::cout << "This is an illegal " << piece->getType() << " capture!\n";
}

void Board::printOwnPieceOnDestination(std::string square)
{
    std::cout << "Cannot move to " << square << " because your own piece is there.\n";
}

void Board::printMoveSuccess(Pieces *piece, std::string startSquare, std::string endSquare)
{
    std::cout << "You moved your " << piece->getColor() << " " << piece->getType() << " from " << startSquare << " to " << endSquare << ".\n";
}

void Board::printCaptureSuccess(Pieces *attacker, Pieces *target, std::string endSquare)
{
    std::cout << "Your " << attacker->getColor() << " " << attacker->getType() << " captured the " << target->getColor() << " " << target->getType() << " on " << endSquare << ".\n";
}

void Board::printSelfCheckMove()
{
    std::cout << "You cannot make that move. Your king will be in check!\n";
}

void Board::printNoKing()
{
    std::cout << "There is no king on the board!\n";
}


bool Board::isInCheck(std::string color) {
    Pieces *king = nullptr;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != nullptr) {
                if ((board[i][j] -> getType() == "King") && (board[i][j] -> getColor()) == color) {
                    king = board[i][j];
                }
            }
        }
    }

    if (king == nullptr) {
        printNoKing();
        return false;
    }
    
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board[i][j] != nullptr) {
                Pieces *piece = board[i][j];
                if (piece -> getColor() != color) {
                    int startRow = getRow(piece -> getSquare());
                    int startCol = getCol(piece -> getSquare());
                    int endRow = getRow(king -> getSquare());
                    int endCol = getCol(king -> getSquare());
                    std::string movingPieceColor = piece -> getColor();
                    bool empty = false;
                    bool enemy = true;
                    bool clear = pathClear(piece -> getSquare(), king -> getSquare());
                    bool check = piece -> isvalidMove(startRow, startCol, endRow, endCol, movingPieceColor, empty, enemy, clear);

                    if (check) {
                        return true;
                    }
                }

            }
            
        }
    }

    return false;

}

void Board::movePiece(std::string startSquare, std::string endSquare)
{

    if (isValid(startSquare) == false)
    {
        printInvalidStartSquare(startSquare);
        return;
    }

    if (isValid(endSquare) == false)
    {
        printInvalidEndSquare(endSquare);
        return;
    }

    if (startSquare == endSquare)
    {
        printSameSquareMove();
        return;
    }

    if (board[getRow(startSquare)][getCol(startSquare)] == nullptr)
    {
        printNoPieceOnSquare(startSquare);
        return;
    }

    Pieces *targetPiece = board[getRow(endSquare)][getCol(endSquare)];
    Pieces * movingPiece = board[getRow(startSquare)][getCol(startSquare)];

    if  (movingPiece -> getColor() != turn) {
        printWrongTurn();
        return;
    }
    
    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);
    std::string movingPieceColor = movingPiece -> getColor();
    bool empty = isEmpty(endSquare);
    bool enemy = isEnemy(endSquare, movingPiece -> getColor());
    bool clear = pathClear(startSquare, endSquare);

    
    if (movingPiece -> isvalidMove(startRow, startCol, endRow, endCol, movingPieceColor, empty, enemy, clear)) {

        board[getRow(endSquare)][getCol(endSquare)] = movingPiece;
        board[getRow(startSquare)][getCol(startSquare)] = nullptr;
        board[getRow(endSquare)][getCol(endSquare)]->setSquare(endSquare);

        if (isInCheck(turn)) {

            printSelfCheckMove();
            printStillTurn();
            board[getRow(startSquare)][getCol(startSquare)] = movingPiece;
            movingPiece -> setSquare(startSquare);
            board[getRow(endSquare)][getCol(endSquare)] = nullptr;
            if (targetPiece != nullptr) {
                board[getRow(endSquare)][getCol(endSquare)] = targetPiece;
                targetPiece -> setSquare(endSquare);
            }
            return;
        }

        if (targetPiece != nullptr)
        {
            targetPiece->capture();
            printCaptureSuccess(movingPiece, targetPiece, endSquare);
        }

        
        if (turn == "White") {
            turn = "Black";
            printMoveSuccess(movingPiece, startSquare, endSquare);
            printNextTurn();
            return;
        }
        
        else {
            turn = "White";
            printMoveSuccess(movingPiece, startSquare, endSquare);
            printNextTurn();
            return;
        }
    }

    else {

        if (targetPiece != nullptr && targetPiece -> getColor() != movingPiece -> getColor()) {
            printIllegalCapture(movingPiece);
            printStillTurn();
            return; 
        }

        else if (targetPiece != nullptr && movingPiece->getColor() == targetPiece->getColor())
        {
        printOwnPieceOnDestination(endSquare);
        printStillTurn();
        return;
        }

        else {
            printIllegalMove(movingPiece);
            printStillTurn();
            return;
        }

    }

}



