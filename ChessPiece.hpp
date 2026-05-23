#pragma once

#include <iostream>
#include <vector>

class Pieces
{

public:
    virtual bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear);

protected:
    std::string type;
    std::string color;
    bool captured;
    std::string square;

public:
    Pieces(std::string type, std::string color, bool captured, std::string square);

    bool isValid(std::string square);
    std::string getType();
    std::string getColor();
    std::string getSquare();
    bool isCaptured();
    std::string setSquare(std::string newSquare);
    void capture();
};

class Pawn : public Pieces
{
public:
    Pawn(std::string color, bool captured, std::string square);
    bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear) override;
};

class Knight : public Pieces
{
public:
    Knight(std::string color, bool captured, std::string square);
    bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear) override;
};

class Bishop : public Pieces
{
public:
    Bishop(std::string color, bool captured, std::string square);
    bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear) override;
};

class Rook : public Pieces
{
public:
    Rook(std::string color, bool captured, std::string square);
    bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear) override;
};

class Queen : public Pieces
{
public:
    Queen(std::string color, bool captured, std::string square);
    bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear) override;
};

class King : public Pieces
{
public:
    King(std::string color, bool captured, std::string square);
    bool isvalidMove(int startRow, int startCol, int endRow, int endCol, std::string color, bool targetEmpty, bool targetEnemy, bool pathClear) override;
};
