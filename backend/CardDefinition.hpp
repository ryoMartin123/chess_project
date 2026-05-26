#pragma once

#include <string>

#include "CardTypes.hpp"

struct CardDefinition
{
    std::string id;
    std::string name;
    int level;
    CardTiming timing;
    CardTargetRequirement targetRequirement;
    CardEffectType effect;
    bool countsAsOwnTurnCard;
};
