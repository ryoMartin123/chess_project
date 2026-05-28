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

void putCardInHand(GameState &game, const std::string &player, const std::string &cardId)
{
    PlayerCardState &state = game.getCardState(player);
    auto removeCard = [&cardId](std::vector<std::string> &cards) {
        cards.erase(std::remove(cards.begin(), cards.end(), cardId), cards.end());
    };

    removeCard(state.deck);
    removeCard(state.discardPile);
    removeCard(state.activePile);
    if (!contains(state.hand, cardId))
    {
        if (state.hand.size() >= 5)
        {
            state.hand.pop_back();
        }
        state.hand.push_back(cardId);
    }
}
}

int main()
{
    GameState game;
    CardAction disintegrate{"disintegrate", "a2"};
    putCardInHand(game, "White", "disintegrate");

    check("Disintegrate can target one of White's non-king pieces before White's move",
          game.canPlayCard("White", disintegrate).success);

    CardResult played = game.playCard("White", disintegrate);
    check("Disintegrate play succeeds", played.success);
    check("Disintegrate removes the piece from a2", game.getBoard().getPieceAt("a2") == nullptr);
    check("Playing Disintegrate consumes it from White's hand",
          !contains(game.getCardState("White").hand, "disintegrate"));
    check("Playing Disintegrate adds it to White's discard pile",
          contains(game.getCardState("White").discardPile, "disintegrate"));
    check("Disintegrate does not end White's turn",
          game.getBoard().getTurn() == "White");
    check("White can still move after playing Disintegrate",
          moveWithoutOutput(game, "b2", "b3"));

    game.reset();
    putCardInHand(game, "White", "disintegrate");
    CardResult kingTarget = game.playCard("White", {"disintegrate", "e1"});
    check("Disintegrate rejects king targets", !kingTarget.success);

    putCardInHand(game, "White", "disintegrate");
    CardResult enemyTarget = game.playCard("White", {"disintegrate", "a7"});
    check("Disintegrate rejects enemy targets", !enemyTarget.success);

    putCardInHand(game, "Black", "disintegrate");
    CardResult wrongTurn = game.playCard("Black", {"disintegrate", "a7"});
    check("Disintegrate rejects play outside the player's own turn", !wrongTurn.success);

    game.reset();
    putCardInHand(game, "White", "disintegrate");
    CardResult discarded = game.discardCard("White", "disintegrate");
    check("White can discard during White's turn", discarded.success);
    check("Discard schedules a draw instead of drawing immediately",
          !contains(game.getCardState("White").hand, "disintegrate") &&
              game.getCardState("White").drawAtStartOfNextTurn);

    check("White can finish the turn after discarding", moveWithoutOutput(game, "e2", "e4"));
    check("Black can finish the reply turn", moveWithoutOutput(game, "e7", "e5"));
    check("White draws a replacement card by the next White turn",
          game.getCardState("White").hand.size() == 1 &&
              !game.getCardState("White").drawAtStartOfNextTurn);

    putCardInHand(game, "White", "charge");
    CardResult drawnCardPlay = game.playCard("White", {"charge", "f3", "g1", "h4"});
    check("A card in hand after the turn-start draw can be played immediately", drawnCardPlay.success);

    game.reset();
    putCardInHand(game, "White", "charge");
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
    putCardInHand(game, "White", "onslaught");
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
    putCardInHand(game, "White", "onslaught");
    CardAction blockedOnslaught;
    blockedOnslaught.cardId = "onslaught";
    blockedOnslaught.fromSquares = {"a1"};
    CardResult blockedOnslaughtResult = game.playCard("White", blockedOnslaught);
    check("Onslaught rejects non-pawn selections",
          !blockedOnslaughtResult.success);

    game.reset();
    putCardInHand(game, "White", "knightmare");
    check("White makes the opening move before testing Knightmare", moveWithoutOutput(game, "g1", "f3"));
    check("Black makes a move that Knightmare can cancel", moveWithoutOutput(game, "g8", "f6"));
    CardResult knightmareMove = game.playCard("White", {"knightmare", ""});
    check("Knightmare can cancel the opponent's previous move", knightmareMove.success);
    check("Knightmare restores the moved piece to its original square",
          game.getBoard().getPieceAt("g8") != nullptr &&
              game.getBoard().getPieceAt("g8")->getType() == "Knight" &&
              game.getBoard().getPieceAt("f6") == nullptr);
    check("Knightmare gives the canceled player the turn again",
          game.getBoard().getTurn() == "Black");
    check("Knightmare prevents repeating the exact canceled move",
          !moveWithoutOutput(game, "g8", "f6"));
    check("Knightmare allows a move with a different piece",
          moveWithoutOutput(game, "b8", "c6"));

    game.reset();
    putCardInHand(game, "Black", "charge");
    putCardInHand(game, "White", "knightmare");
    check("White first passes turn for Black card test", moveWithoutOutput(game, "g1", "f3"));
    CardResult blackCard = game.playCard("Black", {"charge", "a6", "b8", "c5"});
    check("Black plays a card that Knightmare can cancel", blackCard.success);
    CardResult knightmareCard = game.playCard("White", {"knightmare", ""});
    check("Knightmare can cancel the opponent's previous card", knightmareCard.success);
    check("Knightmare restores the card effect",
          game.getBoard().getPieceAt("b8") != nullptr &&
              game.getBoard().getPieceAt("b8")->getType() == "Knight" &&
              game.getBoard().getPieceAt("c5") == nullptr);
    check("Knightmare returns the canceled card to the opponent's hand",
          contains(game.getCardState("Black").hand, "charge"));
    CardResult replaySameCard = game.playCard("Black", {"charge", "a6", "b8", "c5"});
    check("Knightmare prevents replaying the same canceled card",
          !replaySameCard.success);

    game.reset();
    putCardInHand(game, "White", "bog");
    check("White opens the board before testing Bog", moveWithoutOutput(game, "e2", "e4"));
    check("Black opens a bishop lane before testing Bog", moveWithoutOutput(game, "e7", "e5"));
    check("White makes a waiting move before Bog", moveWithoutOutput(game, "g1", "f3"));
    check("Black moves a bishop more than one square", moveWithoutOutput(game, "f8", "c5"));
    CardResult bog = game.playCard("White", {"bog", ""});
    check("Bog can slow the opponent's long bishop move", bog.success);
    check("Bog leaves the slowed bishop on the first square of its path",
          game.getBoard().getPieceAt("e7") != nullptr &&
              game.getBoard().getPieceAt("e7")->getType() == "Bishop" &&
              game.getBoard().getPieceAt("c5") == nullptr &&
              game.getBoard().getPieceAt("f8") == nullptr);
    check("Bog does not consume White's turn",
          game.getBoard().getTurn() == "White");
    check("Bog consumes the card from White's hand",
          !contains(game.getCardState("White").hand, "bog") &&
              contains(game.getCardState("White").discardPile, "bog"));

    game.reset();
    putCardInHand(game, "White", "bog");
    check("White makes the opening move before failed Bog test", moveWithoutOutput(game, "g1", "f3"));
    check("Black makes a knight move that Bog cannot slow", moveWithoutOutput(game, "g8", "f6"));
    CardResult badBog = game.playCard("White", {"bog", ""});
    check("Bog rejects non-rook-bishop-queen moves",
          !badBog.success);

    game.reset();
    putCardInHand(game, "White", "disintegrate");
    putCardInHand(game, "White", "neutrality");
    check("White discards to draw Neutrality for the Neutrality test",
          game.discardCard("White", "disintegrate").success);
    check("White opens before testing Neutrality", moveWithoutOutput(game, "e2", "e4"));
    check("Black moves a knight into a square that attacks Black's king", moveWithoutOutput(game, "g8", "f6"));
    check("White makes a move before playing Neutrality", moveWithoutOutput(game, "g1", "f3"));
    CardResult neutrality = game.playCard("White", {"neutrality", "f6"});
    Pieces *neutralKnight = game.getBoard().getPieceAt("f6");
    check("Neutrality can mark an opponent non-king, non-queen piece after your move",
          neutrality.success && neutralKnight != nullptr && neutralKnight->isNeutral());
    check("A neutral piece can check its own original king",
          game.getBoard().isInCheck("Black"));
    check("Neutrality does not take back the opponent's turn",
          game.getBoard().getTurn() == "Black");
    check("A neutral piece can capture a piece of its own color when moved",
          moveWithoutOutput(game, "f6", "d7") &&
              game.getBoard().getPieceAt("d7") != nullptr &&
              game.getBoard().getPieceAt("d7")->getType() == "Knight");
    check("Neutrality continuing effect tracks who played it and where it moved",
          game.getContinuingEffects().size() == 1 &&
              game.getContinuingEffects()[0].cardId == "neutrality" &&
              game.getContinuingEffects()[0].playedBy == "White" &&
              game.getContinuingEffects()[0].targetSquare == "d7" &&
              game.getContinuingEffects()[0].active);
    check("Removing the neutral piece deactivates its continuing effect",
          game.removePiece("d7") &&
              !game.getContinuingEffects().empty() &&
              !game.getContinuingEffects()[0].active &&
              game.getNeutralSquares().empty());

    game.reset();
    putCardInHand(game, "White", "neutrality");
    CardResult earlyNeutrality = game.playCard("White", {"neutrality", "b8"});
    check("Neutrality rejects play before White has moved",
          !earlyNeutrality.success);
    check("White discards to draw Neutrality for invalid target tests",
          (putCardInHand(game, "White", "disintegrate"),
           putCardInHand(game, "White", "neutrality"),
           game.discardCard("White", "disintegrate").success));
    check("White opens for invalid Neutrality target tests", moveWithoutOutput(game, "e2", "e4"));
    check("Black replies for invalid Neutrality target tests", moveWithoutOutput(game, "e7", "e5"));
    check("White moves before invalid Neutrality target tests", moveWithoutOutput(game, "g1", "f3"));
    CardResult queenNeutrality = game.playCard("White", {"neutrality", "d8"});
    check("Neutrality cannot target a queen",
          !queenNeutrality.success);
    return 0;
}
