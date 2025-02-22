#include "game.h"

#include <QDebug>

Game::Game(
    QObject *parent)
    : QObject{parent} {}

void Game::initGame() {
  {
    std::ranges::for_each(suits, [&](const std::string& suit) {
      std::ranges::for_each(ranks, [&](const std::string& rank) {
        blind_.addCard(Card(suit, rank));
      });
    });
    blind_.shuffle();

    // Distribuite cards 3 - skat(2) - 4 - 3
    for (int i = 1; i <= 3; i++)
      for (Player* player : playerList_)
        blind_.moveTopCardTo(player->handdeck_);
    blind_.moveTopCardTo(skat_);
    blind_.moveTopCardTo(skat_);
    for (int i = 1; i <= 4; i++)
      for (Player* player : playerList_)
        blind_.moveTopCardTo(player->handdeck_);
    for (int i = 1; i <= 3; i++)
      for (Player* player : playerList_)
        blind_.moveTopCardTo(player->handdeck_);

    for (Player* player : playerList_) player->handdeck_.sortCardsByPattern();
  }
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
    const Card& card) {
  if (trick_.cards().size() == 3) {
    trick_.cards().clear();
    emit clearTrickLayout();
  }

  if (trick_.cards().empty()) {
    return true;
  }

  const auto& firstCard = trick_.cards().front();
  const std::string& requiredSuit = firstCard.suit();

  bool hasRequiredSuit = std::ranges::any_of(
      playerList_.front()->handdeck_.cards(),
      [&requiredSuit](const Card& c) { return c.suit() == requiredSuit; });

  if (!hasRequiredSuit) {
    return true;
  }

  if (card.suit() == requiredSuit) {
    return true;
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
  if (!isCardValid(card)) {
    return;
  }

  // 3. Move the card from hand to trick
  playerList_.front()->handdeck_.moveCardTo(card, trick_);

  // 4. Check if the card's power is greater than all cards in the trick
  if (trick_.cards().size() == 1) {
    playerList_.front()->hasTrick_ = true;
    qDebug() << playerList_.front()->name() << " has the trick!";
  } else {
    bool isCardGreater =
        (card.suit() == trick_.cards().front().suit() ||
         card.suit() == trumpSuit_) &&
        std::ranges::all_of(trick_.cards(), [&card](Card& trickCard) {
          qDebug() << trickCard.power() << card.power();
          return card.power() > trickCard.power();
        });

    if (isCardGreater) {
      for (auto& player : playerList_) player->hasTrick_ = false;
      playerList_.front()->hasTrick_ = true;
      qDebug() << playerList_.front()->name() << " has the trick now!";
    }
  }

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
