#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "CardAction.hpp"
#include "CardDefinition.hpp"
#include "CardResult.hpp"
#include "PlayerCardState.hpp"

class GameState;

class CardManager
{
private:
    std::unordered_map<std::string, CardDefinition> definitions;

public:
    CardManager();

    void setupStartingCards(PlayerCardState &playerState) const;
    const CardDefinition *getDefinition(const std::string &cardId) const;
    std::vector<CardDefinition> getDefinitions() const;

    CardResult canPlayCard(GameState &game, const std::string &player, const CardAction &action) const;
    CardResult playCard(GameState &game, const std::string &player, const CardAction &action) const;
    CardResult discardCard(GameState &game, const std::string &player, const std::string &cardId) const;
    CardResult startTurn(GameState &game, const std::string &player) const;
};
