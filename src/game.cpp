#include "game.h"

#include <QDebug>

Game::Game(
    QObject *parent)
    : QObject{parent} {}

void Game::initGame() {
  std::ranges::for_each(suits, [&](const std::string& suit) {
    std::ranges::for_each(ranks, [&](const std::string& rank) {
      Card card = Card(suit, rank);
      // QObject::connect(this, &Game::setCardToPower, card,
      //                  &Card::onSetCardPower);
      blind_.addCard(std::move(card));
    });
  });

  blind_.shuffle();

  // Distribuite cards 3 - skat(2) - 4 - 3
  for (int i = 1; i <= 3; i++)
    for (Player* player : playerList_) blind_.moveTopCardTo(player->handdeck_);
  blind_.moveTopCardTo(skat_);
  blind_.moveTopCardTo(skat_);
  for (int i = 1; i <= 4; i++)
    for (Player* player : playerList_) blind_.moveTopCardTo(player->handdeck_);
  for (int i = 1; i <= 3; i++)
    for (Player* player : playerList_) blind_.moveTopCardTo(player->handdeck_);

  for (Player* player : playerList_) player->handdeck_.sortCardsByPattern();
}

/*
 *    Trick
 *    -----
 *    empty     -> all cards
 *    not empty -> suit  || not same suit in hand -> any
 *    trump     -> trump || no trump -> any
 *
 */

bool Game::isCardValid(
    const Card& card, Rule rule) {
  if (trick_.cards().size() == 3) {
    trick_.cards().clear();
    emit clearTrickLayout();
  }

  if (trick_.cards().empty()) {
    return true;
  }

  if (rule == Rule::Suit) {
    const auto& firstCard = trick_.cards().front();
    std::string requiredSuit = firstCard.suit();
    if (firstCard.rank() == "J") requiredSuit = trumpSuit_;

    // If the first card is a Jack, the player must play either trumpSuit or "J"
    if (firstCard.rank() == "J") {
      bool hasJackInHand =
          std::ranges::any_of(playerList_.front()->handdeck_.cards(),
                              [](const Card& c) { return c.rank() == "J"; });

      bool hasTrumpSuitInHand = std::ranges::any_of(
          playerList_.front()->handdeck_.cards(),
          [this](const Card& c) { return c.suit() == trumpSuit_; });

      // If the player has either a "J" or a trump card, they must play one of
      // those
      if (hasJackInHand || hasTrumpSuitInHand) {
        return card.rank() == "J" || card.suit() == trumpSuit_;
      } else {
        // If they don't have a "J" or a trump suit, they can play any card
        return true;
      }
    }

    // If the first card's suit is not the trump suit and the player has the
    // required suit in hand, they cannot play a Jack with the same suit
    if (requiredSuit != trumpSuit_) {
      bool hasRequiredSuitInHand = std::ranges::any_of(
          playerList_.front()->handdeck_.cards(),
          [&requiredSuit](const Card& c) { return c.suit() == requiredSuit; });

      if (hasRequiredSuitInHand) {
        if (card.rank() == "J" && card.suit() == requiredSuit) {
          return false;  // If the player has the required suit, "J" with the
                         // same suit is not allowed
        }
      }
    }

    // If the required suit is the trump suit, a "J" can always be played
    if (requiredSuit == trumpSuit_) {
      if (card.rank() == "J") {
        return true;  // "J" can be played regardless of its suit
      }
    }

    // If it's not a Jack and not the trump suit, proceed with checking if the
    // player has the required suit
    bool hasRequiredSuit = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(),
        [&requiredSuit](const Card& c) { return c.suit() == requiredSuit; });

    if (!hasRequiredSuit) {
      return true;
    }

    if (card.suit() == requiredSuit) {
      return true;
    }
  }

  else if (rule == Rule::Grand) {
  }

  else if (rule == Rule::Null) {
  }

  else if (rule == Rule::Ramsch) {
  }

  return false;
}

bool Game::isCardGreater(
    const Card& card, Rule rule) {
  if (rule == Rule::Suit) {
    // Case 1: If the trick is empty, return true because any card is valid
    if (trick_.cards().empty()) {
      return true;
    }

    // Case 2: Compare the power of the card with all the previously played
    // cards in the trick
    return std::ranges::all_of(
        std::ranges::subrange(trick_.cards().begin(),
                              std::prev(trick_.cards().end())),
        [&card, this](Card& trickCard) {
          qDebug() << trickCard.power(trumpSuit_, rule_)
                   << card.power(trumpSuit_, rule_);
          return card.power(trumpSuit_, rule_) >
                 trickCard.power(trumpSuit_, rule_);
        });
  }

  else if (rule == Rule::Grand) {
  } else if (rule == Rule::Null) {
  } else if (rule == Rule::Ramsch) {
  }

  return false;
}

// Slots:
void Game::playCard(
    const Card& card) {
  auto& activePlayer = playerList_.front();  // The current player
  auto& hand = activePlayer->handdeck_.cards();

  // 1. Check if the card is in the active player's hand, if not return
  if (std::find(hand.begin(), hand.end(), card) == hand.end()) {
    return;
  }

  // 2. Validate the card move
  if (!isCardValid(card, rule_)) {
    return;
  }

  // 3. Move the card from hand to trick
  playerList_.front()->handdeck_.moveCardTo(card, trick_);

  // 4. Check if the card's power is greater than all cards in the trick
  if (isCardGreater(card, rule_)) {
    // Reset the trick status for all players
    for (auto& player : playerList_) player->hasTrick_ = false;

    // Mark the current player as having the trick
    playerList_.front()->hasTrick_ = true;
    qDebug() << playerList_.front()->name() << " has the trick now!";
  }
  // if (trick_.cards().size() == 1) {
  //   playerList_.front()->hasTrick_ = true;
  //   qDebug() << playerList_.front()->name() << " has the trick!";
  // } else {
  //   if (isCardGreater(card, rule_)) {
  //     for (auto& player : playerList_) player->hasTrick_ = false;
  //     playerList_.front()->hasTrick_ = true;
  //     qDebug() << playerList_.front()->name() << " has the trick now!";
  //   }
  // }

  // 5. Rotate turn order based on the number of cards in the trick
  if (trick_.cards().size() == 3) {
    // Find the player who has the trick
    auto winner =
        std::find_if(playerList_.begin(), playerList_.end(),
                     [](const auto& player) { return player->hasTrick_; });

    // If the winner is not already at the front, rotate the list so that the
    // winner becomes front.
    if (winner != playerList_.end() && !playerList_.front()->hasTrick_) {
      std::rotate(playerList_.begin(), winner, playerList_.end());
    }
    qDebug() << "Rotating to: " << playerList_.front()->name();
  } else {
    // Rotate to the next player if the trick is not full
    std::rotate(playerList_.begin(), playerList_.begin() + 1,
                playerList_.end());
    qDebug() << "Next player: " << playerList_.front()->name();
  }
}
