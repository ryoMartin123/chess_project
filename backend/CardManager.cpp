#include "CardManager.hpp"

#include <algorithm>
#include <random>
#include <sstream>

#include "GameState.hpp"

namespace
{
constexpr size_t STARTING_HAND_SIZE = 0;

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

bool isFriendlyOrNeutral(Pieces *piece, const std::string &player)
{
    return piece != nullptr && (piece->getColor() == player || piece->isNeutral());
}

bool isEnemyOrNeutral(Pieces *piece, const std::string &player)
{
    return piece != nullptr && (piece->getColor() != player || piece->isNeutral());
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
    playerState.cardPlayedOnOwnTurn = true;
}

std::string finishTurnCard(GameState &game, const std::string &player, const CardDefinition &definition)
{
    if (!definition.countsAsOwnTurnCard)
    {
        return "";
    }

    std::string drawMessage;
    PlayerCardState &playerState = game.getCardState(player);
    if (!playerState.drawnThisTurn && !playerState.deck.empty() && playerState.hand.size() < 5)
    {
        const std::string drawnCardId = playerState.deck.back();
        playerState.deck.pop_back();
        playerState.hand.push_back(drawnCardId);
        playerState.drawnThisTurn = true;
        drawMessage = player + " drew " + drawnCardId + " at the end of their turn.";
    }

    const std::string nextPlayer = game.getBoard().getoponentColor(player);
    game.getBoard().setTurn(nextPlayer);
    game.startTurn(nextPlayer);
    return drawMessage;
}

void drawCard(PlayerCardState &playerState)
{
    if (playerState.deck.empty())
    {
        return;
    }

    const std::string drawnCardId = playerState.deck.back();
    playerState.deck.pop_back();
    playerState.hand.push_back(drawnCardId);
}
}

CardManager::CardManager()
{
    CardDefinition disintegrate{
        "disintegrate",
        "Disintegrate",
        2,
        CardTiming::BEFORE_OWN_TURN,
        CardTargetRequirement::FRIENDLY_NON_KING,
        CardEffectType::DESTROY_TARGET,
        false,
        "Played before your move. Destroy any one of your pieces except your king. This does not spend your normal move."};
    definitions.emplace(disintegrate.id, disintegrate);

    CardDefinition charge{
        "charge",
        "Charge",
        6,
        CardTiming::AS_OWN_TURN,
        CardTargetRequirement::FRIENDLY_KNIGHT,
        CardEffectType::CHARGE_KNIGHT,
        true,
        "Played as your turn. Choose one of your knights. It makes two legal knight moves in a row, but the first move cannot capture."};
    definitions.emplace(charge.id, charge);

    CardDefinition onslaught{
        "onslaught",
        "Onslaught",
        6,
        CardTiming::AS_OWN_TURN,
        CardTargetRequirement::FRIENDLY_PAWNS,
        CardEffectType::MOVE_PAWNS,
        true,
        "Played as your turn. Move any number of your pawns one square forward. They cannot capture, and pawns on their starting square still move only one square. Any pawn that reaches the last rank may promote."
    };
    definitions.emplace(onslaught.id, onslaught);

    CardDefinition knightmare{
        "knightmare",
        "Knightmare",
        10,
        CardTiming::AFTER_OPPONENT_TURN,
        CardTargetRequirement::NONE,
        CardEffectType::CANCEL_LAST_ACTION,
        false,
        "Played immediately after your opponent's turn. Cancel their last move or card. They replay their turn, but cannot repeat the exact same move with the same piece or replay the same card."};
    definitions.emplace(knightmare.id, knightmare);

    CardDefinition think_again{
        "think_again",
        "Think Again!",
        10,
        CardTiming::AFTER_OPPONENT_TURN,
        CardTargetRequirement::NONE,
        CardEffectType::CANCEL_LAST_ACTION,
        false,
        "Played immediately after your opponent's turn. Cancel their last move or card. They replay their turn, but cannot repeat the exact same move with the same piece or replay the same card."};
    definitions.emplace(think_again.id, think_again);

    CardDefinition bog{
        "bog",
        "Bog",
        4,
        CardTiming::AFTER_OPPONENT_TURN,
        CardTargetRequirement::NONE,
        CardEffectType::BOG_MOVE,
        false,
        "Played immediately after your opponent's turn. If they moved a rook, bishop, or queen two or more squares, that move is shortened so the piece stops after one square in the chosen direction. This does not spend your turn."};
    definitions.emplace(bog.id, bog);

    CardDefinition neutrality{
        "neutrality",
        "Neutrality",
        9,
        CardTiming::AFTER_OWN_TURN,
        CardTargetRequirement::ENEMY_NON_KING_QUEEN,
        CardEffectType::APPLY_NEUTRALITY,
        false,
        "Played after your move. Choose one opponent piece except a king or queen. It becomes neutral and gets a marker. A neutral piece can check either king and, when that piece moves, it can capture pieces of any color. Cards that affect friendly or enemy pieces can also affect neutral pieces. This continuing effect lasts until the marked piece is captured or the continuing effect is removed by another card."};
    definitions.emplace(neutrality.id, neutrality);

    CardDefinition warlord{
        "warlord",
        "Warlord",
        10,
        CardTiming::AS_OWN_TURN,
        CardTargetRequirement::FRIENDLY_KING,
        CardEffectType::APPLY_WARLORD,
        true,
        "Played as your turn. Your King becomes a Warlord — it may move up to 2 squares per turn in any direction or combination. Making a capture ends its move for that turn. This continuing effect lasts until the Warlord is captured or the game ends."};
    definitions.emplace(warlord.id, warlord);
}



void CardManager::setupStartingCards(PlayerCardState &playerState) const
{
    playerState = {};
    playerState.deck.push_back("disintegrate");
    playerState.deck.push_back("charge");
    playerState.deck.push_back("onslaught");
    playerState.deck.push_back("knightmare");
    playerState.deck.push_back("think_again");
    playerState.deck.push_back("bog");
    playerState.deck.push_back("neutrality");
    playerState.deck.push_back("warlord");

    static std::mt19937 rng(std::random_device{}());
    std::shuffle(playerState.deck.begin(), playerState.deck.end(), rng);
    while (playerState.hand.size() < STARTING_HAND_SIZE && !playerState.deck.empty())
    {
        drawCard(playerState);
    }
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
        return failure("That card can only be played as your own turn.", action);
    }
    if (definition->timing == CardTiming::BEFORE_OWN_TURN && game.getBoard().getTurn() != player)
    {
        return failure("That card can only be played before your move.", action);
    }
    if (definition->timing == CardTiming::AFTER_OPPONENT_TURN && !game.canReactAfterOpponentTurn(player))
    {
        return failure("That reaction can only be played after your opponent's turn.", action);
    }
    if (definition->timing == CardTiming::AFTER_OWN_TURN && !game.canPlayAfterOwnMove(player))
    {
        return failure("That card can only be played after your move.", action);
    }
    if (definition->timing != CardTiming::AS_OWN_TURN &&
        definition->timing != CardTiming::BEFORE_OWN_TURN &&
        definition->timing != CardTiming::AFTER_OWN_TURN &&
        definition->timing != CardTiming::AFTER_OPPONENT_TURN &&
        definition->timing != CardTiming::ANYTIME)
    {
        return failure("That card timing window is not implemented yet.", action);
    }

    std::string retryMessage;
    if (!game.retryAllowsCard(player, action.cardId, retryMessage))
    {
        return failure(retryMessage, action);
    }

    const bool isOwnTurnTiming = definition->timing == CardTiming::BEFORE_OWN_TURN ||
                                  definition->timing == CardTiming::AS_OWN_TURN ||
                                  definition->timing == CardTiming::AFTER_OWN_TURN;
    if (isOwnTurnTiming && playerState.cardPlayedOnOwnTurn)
    {
        return failure("You have already played a card this turn.", action);
    }

    if (definition->effect == CardEffectType::BOG_MOVE && !game.canBogLastMove(player))
    {
        return failure("Bog can only slow an opponent rook, bishop, or queen move of two or more squares.", action);
    }

    Pieces *target = game.getBoard().getPieceAt(action.targetSquare);
    if (definition->targetRequirement == CardTargetRequirement::FRIENDLY_NON_KING &&
        !game.getBoard().isValid(action.targetSquare))
    {
        return failure("Choose a valid target square.", action);
    }
    if (definition->targetRequirement == CardTargetRequirement::FRIENDLY_NON_KING &&
        (!isFriendlyOrNeutral(target, player) || target->getType() == "King"))
    {
        return failure("Disintegrate must target one of your non-king pieces.", action);
    }

    if (definition->targetRequirement == CardTargetRequirement::ENEMY_NON_KING_QUEEN)
    {
        if (!game.getBoard().isValid(action.targetSquare))
        {
            return failure("Choose a valid target square.", action);
        }
        if (!isEnemyOrNeutral(target, player) || target->getType() == "King" || target->getType() == "Queen")
        {
            return failure("Neutrality must target one opponent non-king, non-queen piece.", action);
        }
    }

    if (definition->targetRequirement == CardTargetRequirement::FRIENDLY_KING)
    {
        if (!game.getBoard().isValid(action.targetSquare))
        {
            return failure("Choose a valid target square.", action);
        }
        if (target == nullptr || target->getType() != "King" || target->getColor() != player)
        {
            return failure("Warlord must target your own King.", action);
        }
        if (target->isWarlord())
        {
            return failure("Your King is already a Warlord.", action);
        }
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
        if (!isFriendlyOrNeutral(movingPiece, player) || movingPiece->getType() != "Knight")
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
            if (!isFriendlyOrNeutral(pawn, player) || pawn->getType() != "Pawn")
            {
                return failure("Onslaught may only choose your pawns.", action);
            }

            const std::string destination = oneSquareForward(game.getBoard(), fromSquare, pawn->getColor());
            if (destination.empty() || !game.getBoard().isEmpty(destination))
            {
                return failure("Onslaught pawns must move one square forward without capturing.", action);
            }

            const std::string originalTurn = game.getBoard().getTurn();
            game.getBoard().setTurn(pawn->getColor());
            const std::vector<std::string> legalMoves = game.getBoard().getLegalMoves(fromSquare);
            game.getBoard().setTurn(originalTurn);
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
        if (!game.removePiece(action.targetSquare))
        {
            return failure("The card effect could not remove its target.", action);
        }

        PlayerCardState &playerState = game.getCardState(player);
        consumeCard(playerState, *definition);
        finishTurnCard(game, player, *definition);
        return success(player + " played Disintegrate on " + action.targetSquare + ".", action);
    }

    if (definition->effect == CardEffectType::CHARGE_KNIGHT)
    {
        Board &board = game.getBoard();
        Pieces *movingPiece = board.getPieceAt(action.fromSquare);
        const std::string movingPieceColor = movingPiece == nullptr ? player : movingPiece->getColor();
        board.setTurn(movingPieceColor);
        if (!board.movePiece(action.fromSquare, action.targetSquare, "Queen"))
        {
            board.setTurn(player);
            return failure("Charge first move was not legal.", action);
        }

        board.setTurn(movingPieceColor);
        if (!board.movePiece(action.targetSquare, action.secondTargetSquare, "Queen"))
        {
            board.setTurn(movingPieceColor);
            board.movePiece(action.targetSquare, action.fromSquare, "Queen");
            board.setTurn(player);
            return failure("Charge second move was not legal.", action);
        }

        PlayerCardState &playerState = game.getCardState(player);
        consumeCard(playerState, *definition);
        const std::string chargeDrawMessage = finishTurnCard(game, player, *definition);
        const std::string chargeSuffix = chargeDrawMessage.empty() ? "" : " " + chargeDrawMessage;
        return success(player + " played Charge from " + action.fromSquare + " to " +
                           action.targetSquare + " to " + action.secondTargetSquare + "." + chargeSuffix,
                       action);
    }

    if (definition->effect == CardEffectType::MOVE_PAWNS)
    {
        Board &board = game.getBoard();
        const std::vector<std::string> pawnSquares = orderedPawnSquares(board, action.fromSquares, player);

        for (const std::string &fromSquare : pawnSquares)
        {
            Pieces *pawn = board.getPieceAt(fromSquare);
            const std::string pawnColor = pawn == nullptr ? player : pawn->getColor();
            const std::string destination = oneSquareForward(board, fromSquare, pawnColor);
            board.setTurn(pawnColor);
            if (!board.movePiece(fromSquare, destination, action.promotionPiece))
            {
                board.setTurn(player);
                return failure("Onslaught could not move every selected pawn.", action);
            }
            board.setTurn(player);
        }

        PlayerCardState &playerState = game.getCardState(player);
        consumeCard(playerState, *definition);
        const std::string onslaughtDrawMessage = finishTurnCard(game, player, *definition);

        std::ostringstream message;
        message << player << " played Onslaught and moved " << pawnSquares.size() << " pawn";
        if (pawnSquares.size() != 1)
        {
            message << "s";
        }
        message << " one square forward.";
        if (!onslaughtDrawMessage.empty())
        {
            message << " " << onslaughtDrawMessage;
        }
        return success(message.str(), action);
    }

    if (definition->effect == CardEffectType::APPLY_NEUTRALITY)
    {
        Pieces *target = game.getBoard().getPieceAt(action.targetSquare);
        if (target == nullptr)
        {
            return failure("Neutrality could not find its target.", action);
        }

        target->setNeutral(true);
        game.addContinuingEffect(definition->id, player, action.targetSquare);
        PlayerCardState &playerState = game.getCardState(player);
        auto card = std::find(playerState.hand.begin(), playerState.hand.end(), definition->id);
        playerState.hand.erase(card);
        playerState.activePile.push_back(definition->id);
        playerState.cardPlayedOnOwnTurn = true;
        return success(player + " played Neutrality on " + action.targetSquare + ".", action);
    }

    if (definition->effect == CardEffectType::APPLY_WARLORD)
    {
        Pieces *target = game.getBoard().getPieceAt(action.targetSquare);
        if (target == nullptr)
        {
            return failure("Warlord could not find its target.", action);
        }

        target->setWarlord(true);
        game.addContinuingEffect(definition->id, player, action.targetSquare);
        PlayerCardState &playerState = game.getCardState(player);
        auto card = std::find(playerState.hand.begin(), playerState.hand.end(), definition->id);
        playerState.hand.erase(card);
        playerState.activePile.push_back(definition->id);
        playerState.cardPlayedOnOwnTurn = true;
        const std::string warlordDrawMessage = finishTurnCard(game, player, *definition);
        const std::string warlordSuffix = warlordDrawMessage.empty() ? "" : " " + warlordDrawMessage;
        return success(player + " played Warlord. The " + player + " King on " + action.targetSquare + " is now a Warlord!" + warlordSuffix, action);
    }

    if (definition->effect == CardEffectType::CANCEL_LAST_ACTION)
    {
        PlayerCardState playerStateAfterCost = game.getCardState(player);
        consumeCard(playerStateAfterCost, *definition);

        std::string message;
        if (!game.cancelLastActionWithKnightmare(player, playerStateAfterCost, definition->name, message))
        {
            return failure(message, action);
        }

        return success(message, action);
    }

    if (definition->effect == CardEffectType::BOG_MOVE)
    {
        PlayerCardState playerStateAfterCost = game.getCardState(player);
        consumeCard(playerStateAfterCost, *definition);

        std::string message;
        if (!game.bogLastMove(player, playerStateAfterCost, message))
        {
            return failure(message, action);
        }

        return success(message, action);
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
    playerState.drawAtStartOfNextTurn = false;
    playerState.drawnThisTurn = false;

    return success(player + " turn started.", action);
}
