#include "CardManager.hpp"

#include <algorithm>
#include <sstream>

#include "GameState.hpp"

namespace
{
CardResult failure(const std::string &message, const CardAction &action)
{
    return {false, message, action.cardId, action.targetSquare, ""};
}

CardResult success(const std::string &message, const CardAction &action)
{
    return {true, message, action.cardId, action.targetSquare, ""};
}

bool validPlayer(const std::string &player)
{
    return player == "White" || player == "Black";
}

bool handContains(const PlayerCardState &state, const std::string &cardId)
{
    return std::find(state.hand.begin(), state.hand.end(), cardId) != state.hand.end();
}

std::string oneSquareForward(Board &board, const std::string &square, const std::string &player)
{
    if (!board.isValid(square))
    {
        return "";
    }

    const int rowStep = player == "White" ? 1 : -1;
    return board.getSquare(board.getRow(square) + rowStep, board.getCol(square));
}

bool containsSquare(const std::vector<std::string> &squares, const std::string &square)
{
    return std::find(squares.begin(), squares.end(), square) != squares.end();
}

std::vector<std::string> orderedPawnSquares(Board &board, const std::vector<std::string> &squares, const std::string &player)
{
    std::vector<std::string> orderedSquares = squares;
    std::sort(orderedSquares.begin(), orderedSquares.end(), [&board, &player](const std::string &left, const std::string &right) {
        if (player == "White")
        {
            return board.getRow(left) > board.getRow(right);
        }
        return board.getRow(left) < board.getRow(right);
    });
    return orderedSquares;
}

void consumeCard(PlayerCardState &playerState, const CardDefinition &definition)
{
    auto card = std::find(playerState.hand.begin(), playerState.hand.end(), definition.id);
    playerState.hand.erase(card);
    playerState.discardPile.push_back(definition.id);
    if (definition.countsAsOwnTurnCard)
    {
        playerState.cardPlayedOnOwnTurn = true;
    }
}

void finishOwnTurnCard(GameState &game, const std::string &player, const CardDefinition &definition)
{
    if (!definition.countsAsOwnTurnCard)
    {
        return;
    }

    const std::string nextPlayer = game.getBoard().getoponentColor(player);
    game.getBoard().setTurn(nextPlayer);
    game.startTurn(nextPlayer);
}
}

CardManager::CardManager()
{
    CardDefinition destroyPawn{
        "destroy_pawn",
        "Destroy Pawn",
        1,
        CardTiming::AS_OWN_TURN,
        CardTargetRequirement::ENEMY_PAWN,
        CardEffectType::DESTROY_TARGET,
        true};
    definitions.emplace(destroyPawn.id, destroyPawn);

    CardDefinition charge{
        "charge",
        "Charge",
        6,
        CardTiming::AS_OWN_TURN,
        CardTargetRequirement::FRIENDLY_KNIGHT,
        CardEffectType::CHARGE_KNIGHT,
        true};
    definitions.emplace(charge.id, charge);

    CardDefinition onslaught{
        "onslaught",
        "Onslaught",
        6,
        CardTiming::AS_OWN_TURN,
        CardTargetRequirement::FRIENDLY_PAWNS,
        CardEffectType::MOVE_PAWNS,
        true
    };
    definitions.emplace(onslaught.id, onslaught);

}



void CardManager::setupStartingCards(PlayerCardState &playerState) const
{
    playerState = {};
    playerState.hand.push_back("destroy_pawn");
    playerState.deck.push_back("destroy_pawn");
    
    playerState.hand.push_back("charge");
    playerState.deck.push_back("charge");

    playerState.hand.push_back("onslaught");
    playerState.deck.push_back("onslaught");
}

const CardDefinition *CardManager::getDefinition(const std::string &cardId) const
{
    auto definition = definitions.find(cardId);
    return definition == definitions.end() ? nullptr : &definition->second;
}

std::vector<CardDefinition> CardManager::getDefinitions() const
{
    std::vector<CardDefinition> availableCards;
    for (const auto &entry : definitions)
    {
        availableCards.push_back(entry.second);
    }
    return availableCards;
}

CardResult CardManager::canPlayCard(GameState &game, const std::string &player, const CardAction &action) const
{
    if (!validPlayer(player))
    {
        return failure("Unknown player.", action);
    }

    const CardDefinition *definition = getDefinition(action.cardId);
    if (definition == nullptr)
    {
        return failure("Unknown card.", action);
    }

    const PlayerCardState &playerState = game.getCardState(player);
    if (!handContains(playerState, action.cardId))
    {
        return failure("That card is not in your hand.", action);
    }

    if (definition->timing == CardTiming::AS_OWN_TURN && game.getBoard().getTurn() != player)
    {
        return failure("Destroy Pawn can only be played on your own turn.", action);
    }
    if (definition->timing != CardTiming::AS_OWN_TURN &&
        definition->timing != CardTiming::ANYTIME)
    {
        return failure("That card timing window is not implemented yet.", action);
    }

    if (definition->countsAsOwnTurnCard && playerState.cardPlayedOnOwnTurn)
    {
        return failure("You have already played your own-turn card this turn.", action);
    }

    Pieces *target = game.getBoard().getPieceAt(action.targetSquare);
    if (definition->targetRequirement == CardTargetRequirement::ENEMY_PAWN &&
        !game.getBoard().isValid(action.targetSquare))
    {
        return failure("Choose a valid target square.", action);
    }
    if (definition->targetRequirement == CardTargetRequirement::ENEMY_PAWN &&
        (target == nullptr || target->getColor() == player || target->getType() != "Pawn"))
    {
        return failure("Destroy Pawn must target one enemy pawn.", action);
    }

    if (definition->targetRequirement == CardTargetRequirement::FRIENDLY_KNIGHT)
    {
        if (!game.getBoard().isValid(action.fromSquare))
        {
            return failure("Choose a valid knight square.", action);
        }
        if (!game.getBoard().isValid(action.secondTargetSquare))
        {
            return failure("Choose a valid second target square.", action);
        }

        Pieces *movingPiece = game.getBoard().getPieceAt(action.fromSquare);
        if (movingPiece == nullptr || movingPiece->getColor() != player || movingPiece->getType() != "Knight")
        {
            return failure("Charge must start from one of your knights.", action);
        }
        if (target != nullptr)
        {
            return failure("Charge cannot capture on the first move.", action);
        }
    }

    if (definition->targetRequirement == CardTargetRequirement::FRIENDLY_PAWNS)
    {
        if (action.fromSquares.empty())
        {
            return failure("Onslaught must choose at least one pawn.", action);
        }

        std::vector<std::string> seenSquares;
        for (const std::string &fromSquare : action.fromSquares)
        {
            if (containsSquare(seenSquares, fromSquare))
            {
                return failure("Onslaught cannot choose the same pawn twice.", action);
            }
            seenSquares.push_back(fromSquare);

            if (!game.getBoard().isValid(fromSquare))
            {
                return failure("Onslaught must choose valid pawn squares.", action);
            }

            Pieces *pawn = game.getBoard().getPieceAt(fromSquare);
            if (pawn == nullptr || pawn->getColor() != player || pawn->getType() != "Pawn")
            {
                return failure("Onslaught may only choose your pawns.", action);
            }

            const std::string destination = oneSquareForward(game.getBoard(), fromSquare, player);
            if (destination.empty() || !game.getBoard().isEmpty(destination))
            {
                return failure("Onslaught pawns must move one square forward without capturing.", action);
            }

            const std::vector<std::string> legalMoves = game.getBoard().getLegalMoves(fromSquare);
            if (!containsSquare(legalMoves, destination))
            {
                return failure("Onslaught includes a pawn with an illegal forward move.", action);
            }
        }
    }

    return success("Card can be played.", action);
}

CardResult CardManager::playCard(GameState &game, const std::string &player, const CardAction &action) const
{
    CardResult validation = canPlayCard(game, player, action);
    if (!validation.success)
    {
        return validation;
    }

    const CardDefinition *definition = getDefinition(action.cardId);
    if (definition->effect == CardEffectType::DESTROY_TARGET)
    {
        if (!game.getBoard().removePiece(action.targetSquare))
        {
            return failure("The card effect could not remove its target.", action);
        }

        PlayerCardState &playerState = game.getCardState(player);
        consumeCard(playerState, *definition);
        finishOwnTurnCard(game, player, *definition);
        return success(player + " played Destroy Pawn on " + action.targetSquare + ".", action);
    }

    if (definition->effect == CardEffectType::CHARGE_KNIGHT)
    {
        Board &board = game.getBoard();
        if (!board.movePiece(action.fromSquare, action.targetSquare, "Queen"))
        {
            return failure("Charge first move was not legal.", action);
        }

        board.setTurn(player);
        if (!board.movePiece(action.targetSquare, action.secondTargetSquare, "Queen"))
        {
            board.movePiece(action.targetSquare, action.fromSquare, "Queen");
            board.setTurn(player);
            return failure("Charge second move was not legal.", action);
        }

        PlayerCardState &playerState = game.getCardState(player);
        consumeCard(playerState, *definition);
        game.startTurn(board.getTurn());
        return success(player + " played Charge from " + action.fromSquare + " to " +
                           action.targetSquare + " to " + action.secondTargetSquare + ".",
                       action);
    }

    if (definition->effect == CardEffectType::MOVE_PAWNS)
    {
        Board &board = game.getBoard();
        const std::vector<std::string> pawnSquares = orderedPawnSquares(board, action.fromSquares, player);

        for (const std::string &fromSquare : pawnSquares)
        {
            const std::string destination = oneSquareForward(board, fromSquare, player);
            if (!board.movePiece(fromSquare, destination, action.promotionPiece))
            {
                return failure("Onslaught could not move every selected pawn.", action);
            }
            board.setTurn(player);
        }

        PlayerCardState &playerState = game.getCardState(player);
        consumeCard(playerState, *definition);
        finishOwnTurnCard(game, player, *definition);

        std::ostringstream message;
        message << player << " played Onslaught and moved " << pawnSquares.size() << " pawn";
        if (pawnSquares.size() != 1)
        {
            message << "s";
        }
        message << " one square forward.";
        return success(message.str(), action);
    }

    return failure("That card effect is not implemented yet.", action);
}

CardResult CardManager::discardCard(GameState &game, const std::string &player, const std::string &cardId) const
{
    CardAction action{cardId, ""};
    if (!validPlayer(player))
    {
        return failure("Unknown player.", action);
    }

    if (game.getBoard().getTurn() != player)
    {
        return failure("You may discard only on your own turn.", action);
    }

    PlayerCardState &playerState = game.getCardState(player);
    if (!handContains(playerState, cardId))
    {
        return failure("That card is not in your hand.", action);
    }

    if (playerState.discardedThisTurn)
    {
        return failure("You have already discarded a card this turn.", action);
    }

    auto card = std::find(playerState.hand.begin(), playerState.hand.end(), cardId);
    playerState.hand.erase(card);
    playerState.discardPile.push_back(cardId);
    playerState.discardedThisTurn = true;
    playerState.drawAtStartOfNextTurn = true;
    return success(player + " discarded " + cardId + " and will draw at the start of their next turn.", action);
}

CardResult CardManager::startTurn(GameState &game, const std::string &player) const
{
    CardAction action{"", ""};
    if (!validPlayer(player))
    {
        return failure("Unknown player.", action);
    }

    PlayerCardState &playerState = game.getCardState(player);
    playerState.cardPlayedOnOwnTurn = false;
    playerState.discardedThisTurn = false;

    if (!playerState.drawAtStartOfNextTurn)
    {
        return success(player + " turn started.", action);
    }

    playerState.drawAtStartOfNextTurn = false;
    if (playerState.deck.empty())
    {
        return success(player + " has no card to draw.", action);
    }

    const std::string drawnCardId = playerState.deck.back();
    playerState.deck.pop_back();
    playerState.hand.push_back(drawnCardId);
    return {true, player + " drew " + drawnCardId + " at the start of their turn.", "", "", drawnCardId};
}
