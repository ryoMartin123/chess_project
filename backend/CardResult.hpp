#pragma once

#include <string>

struct CardResult
{
    bool success;
    std::string message;
    std::string cardId;
    std::string targetSquare;
    std::string drawnCardId;
};
