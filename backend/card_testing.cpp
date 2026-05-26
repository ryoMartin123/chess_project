#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "CardAction.hpp"
#include "GameState.hpp"

namespace
{
void check(const std::string &label, bool value)
{
    std::cout << (value ? "PASS: " : "FAIL: ") << label << "\n";
}

bool moveWithoutOutput(GameState &game, const std::string &fromSquare, const std::string &toSquare)
{
    std::ostringstream ignored;
    std::streambuf *previousBuffer = std::cout.rdbuf(ignored.rdbuf());
    bool moved = game.movePiece(fromSquare, toSquare, "Queen");
    std::cout.rdbuf(previousBuffer);
    return moved;
}

bool contains(const std::vector<std::string> &cards, const std::string &cardId)
{
    return std::find(cards.begin(), cards.end(), cardId) != cards.end();
}
}

int main()
{
    GameState game;
    CardAction destroyPawn{"destroy_pawn", "a7"};

    check("Destroy Pawn can target a Black pawn on White's turn",
          game.canPlayCard("White", destroyPawn).success);

    CardResult played = game.playCard("White", destroyPawn);
    check("Destroy Pawn play succeeds", played.success);
    check("Destroy Pawn removes the pawn from a7", game.getBoard().getPieceAt("a7") == nullptr);
    check("Playing the card consumes it from White's hand",
          !contains(game.getCardState("White").hand, "destroy_pawn"));
    check("Playing the card adds it to White's discard pile",
          contains(game.getCardState("White").discardPile, "destroy_pawn"));
    check("Destroy Pawn records White's own-turn card use",
          game.getCardState("White").cardPlayedOnOwnTurn);

    game.reset();
    CardResult wrongTarget = game.playCard("White", {"destroy_pawn", "a2"});
    check("Destroy Pawn rejects a friendly pawn target", !wrongTarget.success);

    CardResult wrongTurn = game.playCard("Black", {"destroy_pawn", "a2"});
    check("Destroy Pawn rejects play outside the player's own turn", !wrongTurn.success);

    game.reset();
    CardResult discarded = game.discardCard("White", "destroy_pawn");
    check("White can discard during White's turn", discarded.success);
    check("Discard schedules a draw instead of drawing immediately",
          !contains(game.getCardState("White").hand, "destroy_pawn") &&
              game.getCardState("White").drawAtStartOfNextTurn);

    check("White can finish the turn after discarding", moveWithoutOutput(game, "e2", "e4"));
    check("Black can finish the reply turn", moveWithoutOutput(game, "e7", "e5"));
    check("White draws the replacement at the start of the next White turn",
          game.getCardState("White").hand.size() == 3 &&
              !game.getCardState("White").drawAtStartOfNextTurn);

    CardResult drawnCardPlay = game.playCard("White", {"charge", "f3", "g1", "h4"});
    check("A card in hand after the turn-start draw can be played immediately", drawnCardPlay.success);

    game.reset();
    CardResult charged = game.playCard("White", {"charge", "f3", "g1", "h4"});
    Pieces *chargedKnight = game.getBoard().getPieceAt("h4");
    check("Charge lets one friendly knight make two legal moves",
          charged.success && chargedKnight != nullptr && chargedKnight->getType() == "Knight");
    check("Charge ends White's turn after the second knight move",
          game.getBoard().getTurn() == "Black");
    check("Charge consumes the card from White's hand",
          !contains(game.getCardState("White").hand, "charge") &&
              contains(game.getCardState("White").discardPile, "charge"));

    game.reset();
    CardAction onslaught;
    onslaught.cardId = "onslaught";
    onslaught.promotionPiece = "Queen";
    onslaught.fromSquares = {"a2", "b2", "c2"};
    CardResult onslaughtPlayed = game.playCard("White", onslaught);
    Pieces *aPawn = game.getBoard().getPieceAt("a3");
    Pieces *bPawn = game.getBoard().getPieceAt("b3");
    Pieces *cPawn = game.getBoard().getPieceAt("c3");
    check("Onslaught moves multiple friendly pawns one square forward",
          onslaughtPlayed.success &&
              aPawn != nullptr && aPawn->getType() == "Pawn" &&
              bPawn != nullptr && bPawn->getType() == "Pawn" &&
              cPawn != nullptr && cPawn->getType() == "Pawn");
    check("Onslaught ends White's turn",
          game.getBoard().getTurn() == "Black");
    check("Onslaught consumes the card from White's hand",
          !contains(game.getCardState("White").hand, "onslaught") &&
              contains(game.getCardState("White").discardPile, "onslaught"));

    game.reset();
    CardAction blockedOnslaught;
    blockedOnslaught.cardId = "onslaught";
    blockedOnslaught.fromSquares = {"a1"};
    CardResult blockedOnslaughtResult = game.playCard("White", blockedOnslaught);
    check("Onslaught rejects non-pawn selections",
          !blockedOnslaughtResult.success);
    return 0;
}
