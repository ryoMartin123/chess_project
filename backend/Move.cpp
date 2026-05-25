#include <iostream>
#include <vector>
#include <algorithm>
#include "Move.hpp"
#include "ChessBoard.hpp"


Move::Move(std::string startSquare, std::string endSquare) {
    this-> startSquare = startSquare;
    this-> endSquare = endSquare;
}

std::string Move::getstartSquare() {
    return startSquare;
}

std::string Move::getendSquare() {
    return endSquare;
}

bool Move::isValid() {
    return true;
}
