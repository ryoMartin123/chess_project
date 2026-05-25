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
    lastStartSquare = "";
    lastEndSquare = "";
    lastMovedPiece = nullptr;
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
    return square[0] - 'a';
}

int Board::getRow(std::string square)
{
    return square[1] - '1';
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

Pieces *Board::getPieceAt(std::string square)
{
    if (!isValid(square))
    {
        return nullptr;
    }

    return board[getRow(square)][getCol(square)];
}

std::string Board::getTurn()
{
    return turn;
}

void Board::setTurn(std::string nextTurn)
{
    if (nextTurn == "Black" || nextTurn == "White")
    {
        turn = nextTurn;
    }
}

bool Board::isGameOver()
{
    return gameOver;
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

void Board::printIllegalCastle(std::string reason) {
    std::cout << "You cannot castle because " << reason << "\n";
}

void Board::printSuccessfulCastle(std::string color, std::string startSquare, std::string endSquare) {
    if (getCol(endSquare) > getCol(startSquare)) {
        std::cout << color << " castled kingside!\n";
    }
    else if (getCol(endSquare) < getCol(startSquare)) {
        std::cout << color << " castled queenside!\n";
    }
}

bool Board::squareIsThreatened(std::string square, std::string enemyColor) {
    if (!isValid(square)) {
        return false;
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != nullptr) {
                Pieces *piece = board[i][j];
                if (piece -> getColor() == enemyColor) {
                    int startRow = getRow(piece -> getSquare());
                    int startCol = getCol(piece -> getSquare());
                    int endRow = getRow(square);
                    int endCol = getCol(square);

                    if (piece->getType() == "Pawn") {
                        bool attacksDiagonal = abs(endCol - startCol) == 1;
                        if (enemyColor == "White" && attacksDiagonal && endRow == startRow + 1) {
                            return true;
                        }
                        if (enemyColor == "Black" && attacksDiagonal && endRow == startRow - 1) {
                            return true;
                        }
                        continue;
                    }

                    std::string color = enemyColor;
                    bool empty = isEmpty(square);
                    bool enemy = true;
                    bool clear = pathClear(piece -> getSquare(), square);
                    bool isThreatened = piece -> isvalidMove(startRow, startCol, endRow, endCol, color, empty, enemy, clear);
                    if (isThreatened) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
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

                        if (startSquare != endSquare && isLegalMoveForPosition(piece, startSquare, endSquare)) {
                            return true;
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

    Pieces *newPiece = nullptr;

    if (piece == "Queen") {
        newPiece = new Queen(color, false, square);
    }
    else if (piece == "Rook") {
        newPiece = new Rook(color, false, square);
    }
    else if (piece == "Knight") {
        newPiece = new Knight(color, false, square);
    }
    else if (piece == "Bishop") {
        newPiece = new Bishop(color, false, square);
    }

    Pieces *oldPawn = board[getRow(square)][getCol(square)];
    if (oldPawn != nullptr) {
        oldPawn -> capture();
    }

    promotedPieceVector.push_back(newPiece);
    board[getRow(square)][getCol(square)] = newPiece;
    printPromoted(color, piece);
    return true;
}

bool Board::castle(Pieces *king, std::string startSquare, std::string endSquare) {

    std::string color = king->getColor();
    std::string rookSquare;
    std::string rookDestination;
    std::string transitSquare;

    if (king->getHasMoved()) {
        printIllegalCastle("your king has already moved.");
        return false;
    }

    if (color == "White" && startSquare == "e1" && endSquare == "g1") {
        rookSquare = "h1";
        rookDestination = "f1";
        transitSquare = "f1";
    }
    else if (color == "White" && startSquare == "e1" && endSquare == "c1") {
        rookSquare = "a1";
        rookDestination = "d1";
        transitSquare = "d1";
    }
    else if (color == "Black" && startSquare == "e8" && endSquare == "g8") {
        rookSquare = "h8";
        rookDestination = "f8";
        transitSquare = "f8";
    }
    else if (color == "Black" && startSquare == "e8" && endSquare == "c8") {
        rookSquare = "a8";
        rookDestination = "d8";
        transitSquare = "d8";
    }
    else {
        printIllegalCastle("the king is not moving from its starting square to a valid castling square.");
        return false;
    }

    Pieces *rook = board[getRow(rookSquare)][getCol(rookSquare)];

    if (rook == nullptr || rook->getColor() != color || rook->getType() != "Rook") {
        printIllegalCastle("the required " + color + " rook is not on " + rookSquare + ".");
        return false;
    }

    if (rook->getHasMoved()) {
        printIllegalCastle("the rook on " + rookSquare + " has already moved.");
        return false;
    }

    if (!pathClear(startSquare, rookSquare)) {
        printIllegalCastle("there is a piece between the king and rook.");
        return false;
    }

    if (isInCheck(color)) {
        printIllegalCastle("your king is currently in check.");
        return false;
    }

    board[getRow(transitSquare)][getCol(transitSquare)] = king;
    board[getRow(startSquare)][getCol(startSquare)] = nullptr;
    king->setSquare(transitSquare);

    bool crossesCheck = isInCheck(color);

    board[getRow(startSquare)][getCol(startSquare)] = king;
    board[getRow(transitSquare)][getCol(transitSquare)] = nullptr;
    king->setSquare(startSquare);

    if (crossesCheck) {
        printIllegalCastle("your king would pass through check on " + transitSquare + ".");
        return false;
    }

    board[getRow(endSquare)][getCol(endSquare)] = king;
    board[getRow(startSquare)][getCol(startSquare)] = nullptr;
    board[getRow(rookDestination)][getCol(rookDestination)] = rook;
    board[getRow(rookSquare)][getCol(rookSquare)] = nullptr;
    king->setSquare(endSquare);
    rook->setSquare(rookDestination);

    if (isInCheck(color)) {
        board[getRow(startSquare)][getCol(startSquare)] = king;
        board[getRow(endSquare)][getCol(endSquare)] = nullptr;
        board[getRow(rookSquare)][getCol(rookSquare)] = rook;
        board[getRow(rookDestination)][getCol(rookDestination)] = nullptr;
        king->setSquare(startSquare);
        rook->setSquare(rookSquare);
        printIllegalCastle("your king would land in check on " + endSquare + ".");
        return false;
    }

    rook->markMoved();
    king->markMoved();
    return true;
}

bool Board::canEnPassant(Pieces *movingPiece, std::string startSquare, std::string endSquare) {

    if (lastMovedPiece == nullptr) {
        return false;
    }

    if (movingPiece -> getType() != "Pawn") {
        return false;
    }

    if (lastMovedPiece -> getType() != "Pawn") {
        return false;
    }

    if (lastMovedPiece -> getColor() == movingPiece -> getColor()) {
        return false;
    }

    if (!isEmpty(endSquare)) {
        return false;
    }

    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);
    int lastStartRow = getRow(lastStartSquare);
    int lastEndRow = getRow(lastEndSquare);
    int lastEndCol = getCol(lastEndSquare);

    if (board[lastEndRow][lastEndCol] != lastMovedPiece) {
        return false;
    }

    if (abs(lastEndRow - lastStartRow) != 2) {
        return false;
    }

    if (startRow != lastEndRow) {
        return false;
    }

    if (abs(startCol - lastEndCol) != 1) {
        return false;
    }

    if (endCol != lastEndCol) {
        return false;
    }

    if (movingPiece -> getColor() == "White") {
        return endRow == startRow + 1;
    }

    return endRow == startRow - 1;
}

bool Board::canCastleSafely(Pieces *king, std::string startSquare, std::string endSquare) {
    if (king == nullptr || king->getType() != "King" || king->getHasMoved()) {
        return false;
    }

    std::string color = king->getColor();
    std::string rookSquare;
    std::string rookDestination;
    std::string transitSquare;

    if (color == "White" && startSquare == "e1" && endSquare == "g1") {
        rookSquare = "h1";
        rookDestination = "f1";
        transitSquare = "f1";
    }
    else if (color == "White" && startSquare == "e1" && endSquare == "c1") {
        rookSquare = "a1";
        rookDestination = "d1";
        transitSquare = "d1";
    }
    else if (color == "Black" && startSquare == "e8" && endSquare == "g8") {
        rookSquare = "h8";
        rookDestination = "f8";
        transitSquare = "f8";
    }
    else if (color == "Black" && startSquare == "e8" && endSquare == "c8") {
        rookSquare = "a8";
        rookDestination = "d8";
        transitSquare = "d8";
    }
    else {
        return false;
    }

    Pieces *rook = board[getRow(rookSquare)][getCol(rookSquare)];
    if (rook == nullptr || rook->getColor() != color || rook->getType() != "Rook" ||
        rook->getHasMoved() || !pathClear(startSquare, rookSquare) || isInCheck(color)) {
        return false;
    }

    board[getRow(transitSquare)][getCol(transitSquare)] = king;
    board[getRow(startSquare)][getCol(startSquare)] = nullptr;
    king->setSquare(transitSquare);
    bool crossesCheck = isInCheck(color);
    board[getRow(startSquare)][getCol(startSquare)] = king;
    board[getRow(transitSquare)][getCol(transitSquare)] = nullptr;
    king->setSquare(startSquare);

    if (crossesCheck) {
        return false;
    }

    board[getRow(endSquare)][getCol(endSquare)] = king;
    board[getRow(startSquare)][getCol(startSquare)] = nullptr;
    board[getRow(rookDestination)][getCol(rookDestination)] = rook;
    board[getRow(rookSquare)][getCol(rookSquare)] = nullptr;
    king->setSquare(endSquare);
    rook->setSquare(rookDestination);
    bool landsInCheck = isInCheck(color);
    board[getRow(startSquare)][getCol(startSquare)] = king;
    board[getRow(endSquare)][getCol(endSquare)] = nullptr;
    board[getRow(rookSquare)][getCol(rookSquare)] = rook;
    board[getRow(rookDestination)][getCol(rookDestination)] = nullptr;
    king->setSquare(startSquare);
    rook->setSquare(rookSquare);

    return !landsInCheck;
}

bool Board::isLegalMoveForPosition(Pieces *movingPiece, std::string startSquare, std::string endSquare) {
    if (movingPiece == nullptr || !isValid(startSquare) || !isValid(endSquare) || startSquare == endSquare) {
        return false;
    }

    Pieces *targetPiece = board[getRow(endSquare)][getCol(endSquare)];
    if (targetPiece != nullptr &&
        (targetPiece->getColor() == movingPiece->getColor() || targetPiece->getType() == "King")) {
        return false;
    }

    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);
    bool castleMove = movingPiece->getType() == "King" && startRow == endRow && abs(endCol - startCol) == 2;

    if (castleMove) {
        return canCastleSafely(movingPiece, startSquare, endSquare);
    }

    bool validEnPassant = canEnPassant(movingPiece, startSquare, endSquare);
    bool validMove = movingPiece->isvalidMove(
        startRow,
        startCol,
        endRow,
        endCol,
        movingPiece->getColor(),
        isEmpty(endSquare),
        targetPiece != nullptr && targetPiece->getColor() != movingPiece->getColor(),
        pathClear(startSquare, endSquare));

    if (!validMove && !validEnPassant) {
        return false;
    }

    Pieces *enPassantPawn = nullptr;
    if (validEnPassant) {
        enPassantPawn = board[getRow(lastEndSquare)][getCol(lastEndSquare)];
        board[getRow(lastEndSquare)][getCol(lastEndSquare)] = nullptr;
    }

    board[endRow][endCol] = movingPiece;
    board[startRow][startCol] = nullptr;
    movingPiece->setSquare(endSquare);
    bool leavesKingInCheck = isInCheck(movingPiece->getColor());
    board[startRow][startCol] = movingPiece;
    board[endRow][endCol] = targetPiece;
    movingPiece->setSquare(startSquare);
    if (enPassantPawn != nullptr) {
        board[getRow(lastEndSquare)][getCol(lastEndSquare)] = enPassantPawn;
    }

    return !leavesKingInCheck;
}

std::vector<std::string> Board::getLegalMoves(std::string startSquare) {
    std::vector<std::string> moves;
    Pieces *movingPiece = getPieceAt(startSquare);

    if (gameOver || movingPiece == nullptr || movingPiece->getColor() != turn) {
        return moves;
    }

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            std::string endSquare = getSquare(row, col);
            if (isLegalMoveForPosition(movingPiece, startSquare, endSquare)) {
                moves.push_back(endSquare);
            }
        }
    }

    return moves;
}

bool Board::moveLeavesKingInCheck(Pieces *movingPiece, Pieces *targetPiece, std::string startSquare, std::string endSquare) {
    int startRow = getRow(startSquare);
    int startCol = getCol(startSquare);
    int endRow = getRow(endSquare);
    int endCol = getCol(endSquare);

    board[endRow][endCol] = movingPiece;
    board[startRow][startCol] = nullptr;
    movingPiece -> setSquare(endSquare);

    bool leavesKingInCheck = isInCheck(turn);

    board[startRow][startCol] = movingPiece;
    board[endRow][endCol] = targetPiece;
    movingPiece -> setSquare(startSquare);

    if (targetPiece != nullptr) {
        targetPiece -> setSquare(endSquare);
    }

    return leavesKingInCheck;
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

    if (targetPiece != nullptr &&
        targetPiece->getColor() != movingPiece->getColor() &&
        targetPiece->getType() == "King") {
        printIllegalCapture(movingPiece);
        printStillTurn();
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
    bool castleMove = movingPiece -> getType() == "King" && startRow == endRow && abs(endCol - startCol) == 2;
    bool validEnpassant = canEnPassant(movingPiece, startSquare, endSquare);

    if (validEnpassant) {
        Pieces *capturedPawn = lastMovedPiece;

        board[endRow][endCol] = movingPiece;
        board[startRow][startCol] = nullptr;
        board[getRow(lastEndSquare)][getCol(lastEndSquare)] = nullptr;
        movingPiece -> setSquare(endSquare);

        if (isInCheck(turn)) {
            board[startRow][startCol] = movingPiece;
            board[endRow][endCol] = nullptr;
            board[getRow(lastEndSquare)][getCol(lastEndSquare)] = capturedPawn;
            movingPiece -> setSquare(startSquare);

            printSelfCheckMove();
            printStillTurn();
            return false;
        }

        capturedPawn -> capture();
        movingPiece -> markMoved();
        printMoveSuccess(movingPiece, startSquare, endSquare);
        return finishSuccessfulMove(movingPiece, startSquare, endSquare);
    }

    if (castleMove) {
        if (!castle(movingPiece, startSquare, endSquare)) {
            return false;
        }

        printSuccessfulCastle(movingPiece->getColor(), startSquare, endSquare);
        return finishSuccessfulMove(movingPiece, startSquare, endSquare);
    }

    if (movingPiece -> isvalidMove(startRow, startCol, endRow, endCol, movingPieceColor, empty, enemy, clear)) {
        if (promotionMove && !isValidPromotionChoice(promotedPiece)) {
            printInvalidPromotion(promotedPiece);
            return false;
        }

        if (moveLeavesKingInCheck(movingPiece, targetPiece, startSquare, endSquare)) {
            printSelfCheckMove();
            printStillTurn();
            return false;
        }

        board[endRow][endCol] = movingPiece;
        board[startRow][startCol] = nullptr;
        movingPiece -> setSquare(endSquare);

        if (targetPiece != nullptr)
        {
            targetPiece->capture();
            printCaptureSuccess(movingPiece, targetPiece, endSquare);
        }

        movingPiece->markMoved();
        printMoveSuccess(movingPiece, startSquare, endSquare);

        if (promotionMove) {
            createPromotedPiece(promotedPiece, movingPiece -> getColor(), endSquare);
            movingPiece = board[endRow][endCol];
        }

        return finishSuccessfulMove(movingPiece, startSquare, endSquare);
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

void Board::rememberLastMove(Pieces *piece, std::string startSquare, std::string endSquare) {
    lastStartSquare = startSquare;
    lastEndSquare = endSquare;
    lastMovedPiece = piece;
}

bool Board::finishSuccessfulMove(Pieces *movingPiece, std::string startSquare, std::string endSquare)
{
    std::string opponentColor = getoponentColor(movingPiece->getColor());
    rememberLastMove(movingPiece, startSquare, endSquare);
    turn = opponentColor;

    if (isCheckmate(opponentColor)) {
        printCheckMate(movingPiece->getColor());
        gameOver = true;
        return true;
    }

    else if (isInCheck(opponentColor)) {
        printCheck(opponentColor);
    }

    else if (isStalemate(opponentColor)) {
        printStaleMate(movingPiece->getColor());
        gameOver = true;
        return true;
    }

    printNextTurn();
    return true;
}
