#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "ChessBoard.hpp"
#include "ChessPiece.hpp"





class Move {
private:
    std::string startSquare;
    std::string endSquare;

public: 
    Move(std::string startSquare, std::string endSquare);
    std::string getstartSquare();
    std::string getendSquare();
    bool isValid();


};