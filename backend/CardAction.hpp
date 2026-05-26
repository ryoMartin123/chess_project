#pragma once

#include <string>
#include <vector>

struct CardAction
{
    std::string cardId;
    std::string targetSquare;
    std::string fromSquare;
    std::string secondTargetSquare;
    std::string promotionPiece;
    std::vector<std::string> fromSquares;
};
