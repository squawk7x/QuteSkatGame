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

  const auto& firstCard = trick_.cards().front();
  std::string requiredSuit = firstCard.suit();

  if (rule == Rule::Suit) {
    if (firstCard.rank() == "J") requiredSuit = trumpSuit_;

    // Case 1: First card is a Jack
    if (firstCard.rank() == "J" || firstCard.suit() == trumpSuit_) {
      bool hasJackInHand =
          std::ranges::any_of(playerList_.front()->handdeck_.cards(),
                              [](const Card& c) { return c.rank() == "J"; });

      bool hasTrumpSuitInHand = std::ranges::any_of(
          playerList_.front()->handdeck_.cards(),
          [this](const Card& c) { return c.suit() == trumpSuit_; });

      // If the player has a Jack or a trump suit card, they must play one of
      // them
      if (hasJackInHand || hasTrumpSuitInHand)
        return card.rank() == "J" || card.suit() == trumpSuit_;
      else
        return true;  // No Jacks or trump cards in hand, any card is valid
    }

    // Case 2: First card is neither a Jack nor a trump suit card
    bool hasRequiredSuitInHand = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(),
        [&requiredSuit](const Card& c) { return c.suit() == requiredSuit; });

    // If the player has a card of the required suit, they must play them
    if (hasRequiredSuitInHand)
      return card.suit() == requiredSuit;
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

  activateNextPlayer();
}

void Game::activateNextPlayer() {
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
