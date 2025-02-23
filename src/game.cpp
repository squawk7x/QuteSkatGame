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

  // for testing
  playerList_.front()->tricks_.push_back(skat_);
  countRound();
}

bool Game::isCardInHand(
    const Card& card) {
  auto& hand = playerList_.front()->handdeck_.cards();

  // 1. Check if the card is in the active player's hand, if not return
  if (std::find(hand.begin(), hand.end(), card) < hand.end()) {
    return true;
  }
  return false;
}

bool Game::isCardValid(
    const Card& card, Rule rule) {
  if (trick_.cards().size() == 3) {
    trick_.cards().clear();
    emit clearTrickLayout();
  }

  if (trick_.cards().empty()) {
    return true;
  }

  const auto& firstCard = trick_.cards().front();
  std::string requiredSuit = firstCard.suit();

  if (rule == Rule::Suit) {
    if (firstCard.rank() == "J") requiredSuit = trumpSuit_;

    // Case 1: First card is a Jack or trump
    if (firstCard.rank() == "J" || firstCard.suit() == trumpSuit_) {
      bool hasTrumpInHand = std::ranges::any_of(
          playerList_.front()->handdeck_.cards(), [this](const Card& c) {
            return c.suit() == trumpSuit_ || c.rank() == "J";
          });

      // bool hasTrumpSuitInHand = std::ranges::any_of(
      //     playerList_.front()->handdeck_.cards(),
      //     [this](const Card& c) { return c.suit() == trumpSuit_; });

      // If the player has a Jack or a trump suit card, they must play one of
      // them
      // if (hasJackInHand || hasTrumpSuitInHand)
      if (hasTrumpInHand)
        return card.rank() == "J" || card.suit() == trumpSuit_;
      else
        return true;  // No Jacks or trump cards in hand, any card is valid
    }

    // Case 2: First card is neither a Jack nor a trump suit card
    bool hasRequiredSuitInHand = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(), [&requiredSuit](const Card& c) {
          return (c.suit() == requiredSuit) && (c.rank() != "J");
        });

    // If the player has a card of the required suit, they must play them
    if (hasRequiredSuitInHand)
      return (card.suit() == requiredSuit) && (card.rank() != "J");
    else
      return true;  // No card of the required suit in hand, any card is valid
  }

  else if (rule == Rule::Grand || rule == Rule::Ramsch) {
    if (firstCard.rank() == "J") {
      bool hasJackInHand =
          std::ranges::any_of(playerList_.front()->handdeck_.cards(),
                              [](const Card& c) { return c.rank() == "J"; });

      // If the player has a Jack card, they must play them
      if (hasJackInHand)
        return card.rank() == "J";
      else
        return true;  // No Jacks or trump cards in hand, any card is valid
    }

    // Case 2: First card is neither a Jack nor a trump suit card
    bool hasRequiredSuitInHand = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(), [&requiredSuit](const Card& c) {
          return (c.suit() == requiredSuit) && (c.rank() != "J");
        });

    // If the player has a card of the required suit, they must play them
    if (hasRequiredSuitInHand)
      return (card.suit() == requiredSuit) && (card.rank() != "J");
    else
      return true;  // No card of the required suit in hand, any card is valid
  }

  else if (rule == Rule::Null) {
    bool hasRequiredSuitInHand = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(),
        [&requiredSuit](const Card& c) { return c.suit() == requiredSuit; });

    // If the player has a card of the required suit, they must play it
    if (hasRequiredSuitInHand)
      return card.suit() == requiredSuit;
    else
      return true;
  }

  return false;
}

bool Game::isCardGreater(
    const Card& card, Rule rule) {
  if (trick_.cards().empty()) {
    return true;  // Erste gespielte Karte ist immer "größer"
  }

  const auto& firstCard = trick_.cards().front();

  // Trick ohne die letzte gespielte Karte
  auto trickWithoutLast = std::ranges::subrange(
      trick_.cards().begin(), std::prev(trick_.cards().end()));

  if (rule == Rule::Suit) {
    if (card.suit() == firstCard.suit() || card.suit() == trumpSuit_ ||
        card.rank() == "J") {
      return std::ranges::all_of(trickWithoutLast,
                                 [&card, this](const Card& trickCard) {
                                   return card.power(trumpSuit_, rule_) >
                                          trickCard.power(trumpSuit_, rule_);
                                 });
    }
  } else if (rule == Rule::Grand || rule == Rule::Ramsch) {
    if (card.suit() == firstCard.suit() || card.rank() == "J") {
      return std::ranges::all_of(trickWithoutLast,
                                 [&card, this](const Card& trickCard) {
                                   return card.power(trumpSuit_, rule_) >
                                          trickCard.power(trumpSuit_, rule_);
                                 });
    }
  } else if (rule == Rule::Null) {
    if (card.suit() == firstCard.suit()) {
      return std::ranges::all_of(trickWithoutLast,
                                 [&card, this](const Card& trickCard) {
                                   return card.power(trumpSuit_, rule_) >
                                          trickCard.power(trumpSuit_, rule_);
                                 });
    }
  }

  return false;
}

// Slots:
void Game::playCard(
    const Card& card) {
  // Card Validation in table.cpp connect (...)

  // Move the card from hand to trick
  playerList_.front()->handdeck_.moveCardTo(card, trick_);

  //  Check if the card's power is greater than all cards in the trick
  if (isCardGreater(card, rule_)) {
    // Reset the trick status for all players
    for (auto& player : playerList_) player->hasTrick_ = false;

    // Mark the current player as having the trick
    playerList_.front()->hasTrick_ = true;
    qDebug() << playerList_.front()->name() << " has the trick now!";
  }

  // Rotate playerlist
  activateNextPlayer();
}

void Game::activateNextPlayer() {
  if (trick_.cards().size() == 3) {
    // Find the player who has the trick
    auto trickholder =
        std::find_if(playerList_.begin(), playerList_.end(),
                     [](const auto& player) { return player->hasTrick_; });

    // Rotate the trickholder to the front
    if (trickholder != playerList_.end() && !playerList_.front()->hasTrick_) {
      std::rotate(playerList_.begin(), trickholder, playerList_.end());
    }
    qDebug() << "Rotating to: " << playerList_.front()->name();
    // moving trick to players tricks
    playerList_.front()->tricks_.push_back(trick_);
    qDebug() << "Trick moved to Trickholder " << playerList_.front()->name();

  } else {
    // Rotate to the next player if the trick is not full
    std::rotate(playerList_.begin(), playerList_.begin() + 1,
                playerList_.end());
    qDebug() << "Next player: " << playerList_.front()->name();
  }
  // count at end of round
  // if (playerList_.front()->handdeck_.cards().empty()) {
  countRound();
  // }
}

void Game::countRound() {
  int sum{0};
  for (const auto& player : playerList_) {
    for (CardVec& vec : player->tricks_) {
      sum += vec.value();
    }
    qDebug() << player->name() << " - Total Points: " << sum;
    sum = 0;
  }
}
