#pragma once

#include <string>
#include <vector>

struct PlayerCardState
{
    std::vector<std::string> deck;
    std::vector<std::string> hand;
    std::vector<std::string> discardPile;
    bool cardPlayedOnOwnTurn = false;
    bool discardedThisTurn = false;
    bool drawAtStartOfNextTurn = false;
};
