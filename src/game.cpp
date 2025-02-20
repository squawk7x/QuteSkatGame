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
    while (!playerList_.front()->hasTrick_) {
      std::rotate(playerList_.begin(), playerList_.begin() + 1,
                  playerList_.end());
    }
    qDebug() << "Trick full. Moving to player's tricks.";
    qDebug() << "Next player with hasTrick: " << playerList_.front()->id();
    playerList_.front()->tricks_.push_back(trick_);
    trick_.clearCards();
    emit clearTrickLayout();
    return false;
  }

  if (trick_.cards().empty())
    return true;  // Any card is valid if trick is empty

  const auto& firstCard = trick_.cards().front();
  const std::string& requiredSuit = firstCard.suit();

  // Check if player has a card of the required suit
  bool hasRequiredSuit = std::ranges::any_of(
      playerList_.front()->handdeck_.cards(),
      [&requiredSuit](const Card& c) { return c.suit() == requiredSuit; });

  qDebug() << "Checking card validity for player ID: "
           << playerList_.front()->id();
  qDebug() << "Trying to play: " << QString::fromStdString(card.str());
  qDebug() << "First card in trick: "
           << QString::fromStdString(firstCard.str());
  qDebug() << "Required suit: " << QString::fromStdString(requiredSuit);
  qDebug() << "Player has required suit? " << hasRequiredSuit;

  // If the player has the required suit, check the rank condition
  if (hasRequiredSuit) {
    bool isRankGreaterThanAllInTrick =
        std::ranges::all_of(trick_.cards(), [&card](const Card& trickCard) {
          return card.rank() > trickCard.rank();
        });

    // If the card rank is greater than all in the trick, set the hasTrick flag
    if (isRankGreaterThanAllInTrick) {
      playerList_[0]->hasTrick_ = true;
    } else {
      playerList_[0]->hasTrick_ = false;  // You may want to reset the flag
    }
  }

  // If the card doesn't match the required suit, it may still be valid
  return !(hasRequiredSuit && card.suit() != requiredSuit);
}

// Slots:
void Game::playCard(
    const Card& card) {
  auto& hand = playerList_.front()->handdeck_.cards();

  // Check if the card is in the player's hand
  if (std::find(hand.begin(), hand.end(), card) == hand.end()) {
    qDebug() << "Error: Card not found in player's hand!";
    return;
  }

  if (!isCardValid(card)) {
    qDebug() << "Move rejected: Invalid card choice. Player must follow suit.";
    return;
  }

  // Move the card from hand to trick
  playerList_.front()->handdeck_.moveCardTo(card, trick_);
  qDebug() << "Card played: " << QString::fromStdString(card.str());
  qDebug() << "Updated trick_: " << trick_.print();

  // Rotate turn order
  std::rotate(playerList_.begin(), playerList_.begin() + 1, playerList_.end());
  qDebug() << "Next player: " << playerList_.front()->id();
}
