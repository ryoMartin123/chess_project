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
    gameOver = false;
}

Board::~Board()
{
    for (Pieces *piece : promotedPieceVector)
    {
        delete piece;
    }
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

std::string Board::getSquare(int row, int col)
{
    if (row < 0 || row > 7 || col < 0 || col > 7)
    {
        return "";
    }

    std::string square;
    square += static_cast<char>('a' + col);
    square += static_cast<char>('1' + row);
    return square;
}

std::string Board::getoponentColor(std::string yourColor) {
    if (yourColor == "White") {
        return "Black";
    }
    else {
        return "White";
    }
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

void Board::printCheck(std::string color) {
    std::cout << color << " is in check!\n";
}

void Board::printCheckMate(std::string color) {
    std::cout << "Checkmate! " << color << " won!\n";
}

void Board::printStaleMate(std::string color) {
    std::cout << color << " caused Stalemate! The game is a draw!\n";
}

void Board::printPromoted(std::string color, std::string promotedPiece) {
    std::cout << "Your " << color << " pawn promoted to a " << promotedPiece << "\n";
}

void Board::printInvalidPromotion(std::string piece) {
    std::cout << "You cannot promote to a " << piece << "\n";
    std::cout << "Choose: Queen, Rook, Knight, or Bishop\n";
}

void Board::printGameOver() {
    std::cout << "The game is already over.\n";
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

bool Board::hasAnyLegalMove(std::string color) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((board[i][j] != nullptr) && (board[i][j] -> getColor() == color)) {
                Pieces *piece = board[i][j];
                std::string startSquare = piece -> getSquare();

                for (int x = 0; x < 8; x++) {
                    for (int y = 0; y < 8; y++) {
                        std::string endSquare = getSquare(x, y);

                            if (startSquare == endSquare) {
                                continue;
                            }
                            int startRow = getRow(piece -> getSquare());
                            int startCol = getCol(piece -> getSquare());
                            int endRow = getRow(endSquare);
                            int endCol = getCol(endSquare);
                            std::string movingPieceColor = color;
                            Pieces * targetPiece = board[endRow][endCol];

                            if (targetPiece != nullptr && targetPiece -> getColor() == color) {
                                continue;
                            }

                            bool empty = isEmpty(endSquare);
                            bool enemy = false;
                            if (targetPiece != nullptr && targetPiece -> getColor() != color) {
                                enemy = true;
                            }
                            bool clear = pathClear(piece -> getSquare(), endSquare);

                            if(piece -> isvalidMove(startRow, startCol, endRow, endCol, movingPieceColor, empty, enemy, clear)) {
                                board[endRow][endCol] = piece;
                                board[startRow][startCol] = nullptr;
                                piece -> setSquare(endSquare);

                                bool stillInCheck = isInCheck(color);
                                board[startRow][startCol] = piece;
                                board[endRow][endCol] = targetPiece;
                                piece -> setSquare(startSquare);
                                if (targetPiece != nullptr) {
                                    targetPiece -> setSquare(endSquare);
                                }

                                if (stillInCheck == false) {
                                    return true;
                                }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool Board::isCheckmate(std::string color) {
    if (!hasAnyLegalMove(color) && isInCheck(color)) {
        return true;
    }
    return false;
}

bool Board::isStalemate(std::string color) {
    if (!hasAnyLegalMove(color) && !isInCheck(color)) {
        return true;
    }
    return false;
}

bool Board::isValidPromotionChoice(std::string piece) {
    std::vector<std::string> promotionChoices = {"Knight", "Bishop", "Rook", "Queen"};
    return std::find(promotionChoices.begin(), promotionChoices.end(), piece) != promotionChoices.end();
}

bool Board::createPromotedPiece(std::string piece, std::string color, std::string square) {
    if (!isValidPromotionChoice(piece)) {
        return false;
    }

    if (piece == "Queen") {
        Pieces *newQueen = new Queen(color, false, square);
        promotedPieceVector.push_back(newQueen);
        board[getRow(square)][getCol(square)] = newQueen;
        printPromoted(color, piece);
        return true;
    }
    if (piece == "Rook") {
        Pieces *newRook = new Rook(color, false, square);
        promotedPieceVector.push_back(newRook);
        board[getRow(square)][getCol(square)] = newRook;
        printPromoted(color, piece);
        return true;
    }
    if (piece == "Knight") {
        Pieces *newKnight = new Knight(color, false, square);
        promotedPieceVector.push_back(newKnight);
        board[getRow(square)][getCol(square)] = newKnight;
        printPromoted(color, piece);
        return true;
    }
    if (piece == "Bishop") {
        Pieces *newBishop = new Bishop(color, false, square);
        promotedPieceVector.push_back(newBishop);
        board[getRow(square)][getCol(square)] = newBishop;
        printPromoted(color, piece);
        return true;
    }

    return false;
}
bool Board::movePiece(std::string startSquare, std::string endSquare, std::string promotedPiece)
{

    if (gameOver) {
        printGameOver();
        return false;
    }

    if (isValid(startSquare) == false)
    {
        printInvalidStartSquare(startSquare);
        return false;
    }

    if (isValid(endSquare) == false)
    {
        printInvalidEndSquare(endSquare);
        return false;
    }

    if (startSquare == endSquare)
    {
        printSameSquareMove();
        return false;
    }

    if (board[getRow(startSquare)][getCol(startSquare)] == nullptr)
    {
        printNoPieceOnSquare(startSquare);
        return false;
    }

    Pieces *targetPiece = board[getRow(endSquare)][getCol(endSquare)];
    Pieces *movingPiece = board[getRow(startSquare)][getCol(startSquare)];

    if  (movingPiece -> getColor() != turn) {
        printWrongTurn();
        return false;
    }
    
    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);
    std::string movingPieceColor = movingPiece -> getColor();
    bool empty = isEmpty(endSquare);
    bool enemy = isEnemy(endSquare, movingPiece -> getColor());
    bool clear = pathClear(startSquare, endSquare);
    bool promotionMove = movingPiece -> getType() == "Pawn" &&
        ((movingPiece -> getColor() == "White" && endRow == 7) ||
         (movingPiece -> getColor() == "Black" && endRow == 0));

    
    if (movingPiece -> isvalidMove(startRow, startCol, endRow, endCol, movingPieceColor, empty, enemy, clear)) {
        if (promotionMove && !isValidPromotionChoice(promotedPiece)) {
            printInvalidPromotion(promotedPiece);
            return false;
        }

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
            return false;
        }

        if (targetPiece != nullptr)
        {
            targetPiece->capture();
            printCaptureSuccess(movingPiece, targetPiece, endSquare);
        }

        printMoveSuccess(movingPiece, startSquare, endSquare);

        if (promotionMove) {
            if (!createPromotedPiece(promotedPiece, movingPiece -> getColor(), endSquare)) {
                printInvalidPromotion(promotedPiece);
                return false;
            }
        }

        std::string opponentColor = getoponentColor(movingPiece->getColor());

        if (isCheckmate(opponentColor)) {
            printCheckMate(movingPiece -> getColor());
            gameOver = true;
            return true;
        }

        else if (isInCheck(opponentColor)) {
            printCheck(opponentColor);
        }

        else if (isStalemate(opponentColor)) {
            printStaleMate(movingPiece -> getColor());
            gameOver = true;
            return true;
        }

        turn = opponentColor;
        printNextTurn();
        return true;
    }

    else {

        if (targetPiece != nullptr && targetPiece -> getColor() != movingPiece -> getColor()) {
            printIllegalCapture(movingPiece);
            printStillTurn();
            return false;
        }

        else if (targetPiece != nullptr && movingPiece->getColor() == targetPiece->getColor())
        {
        printOwnPieceOnDestination(endSquare);
        printStillTurn();
        return false;
        }

        else {
            printIllegalMove(movingPiece);
            printStillTurn();
            return false;
        }
    }
}
