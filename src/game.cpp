#include "game.h"

#include <QDebug>

#include "definitions.h"

Game::Game(
    QObject* parent)
    : QObject{parent} {}

void Game::initGame() {
  std::ranges::for_each(suits, [&](const std::string& suit) {
    std::ranges::for_each(ranks, [&](const std::string& rank) {
      Card card = Card(suit, rank);
      blind_.addCard(std::move(card));
    });
  });

  blind_.shuffle();
  geben();
}

void Game::startGame() {
  // Players have their cards and evaluate maxSagen
  player_1.maxBieten_ = 40;
  player_2.maxBieten_ = 30;
  player_3.maxBieten_ = 33;

  reizen();
}

// playerList_ toggles at each move
// ghs_ toggles at next round
// first round ghs_{0, 1, 2}, second round ghs_{1, 2, 0}, ...
// returns the player at ghs_[pos] position
Player& Game::getPlayerByPos(
    int pos) {
  auto it = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == ghs_[pos];
  });

  if (it != playerList_.end()) {
    return **it;  // Dereference the iterator**
  } else {
    throw std::runtime_error("Player not found at position " +
                             std::to_string(pos));
  }
}

int Game::bieten() {
  static int counter = 0;

  constexpr std::array<int, 57> gebote = {
      18,  20,  22,  23,  24,  27,  30,  33,  35,  36,  40,  44,  45,  46,  50,
      55,  59,  60,  63,  66,  70,  72,  77,  80,  81,  84,  88,  90,  96,  99,
      100, 108, 110, 117, 120, 121, 126, 130, 132, 135, 140, 143, 144, 150, 153,
      156, 162, 165, 168, 170, 180, 187, 192, 198, 204, 210, 216};

  int index = std::min(
      counter,
      static_cast<int>(gebote.size() - 1));  // Verhindert Out-of-Bounds-Zugriff
  counter++;  // Zählt nur hoch, wenn nicht am Ende

  return gebote[index];
}

void Game::geben() {
  // Distribuite cards 3 - skat(2) - 4 - 3
  for (Player* player : playerList_)
    for (int i = 1; i <= 3; i++) blind_.moveTopCardTo(player->handdeck_);
  blind_.moveTopCardTo(skat_);
  blind_.moveTopCardTo(skat_);
  for (Player* player : playerList_)
    for (int i = 1; i <= 4; i++) blind_.moveTopCardTo(player->handdeck_);
  for (Player* player : playerList_)
    for (int i = 1; i <= 3; i++) blind_.moveTopCardTo(player->handdeck_);

  for (Player* player : playerList_) player->handdeck_.sortByJandSuits();
}

QString Game::hoeren(
    int hoererPos, int angesagt) {
  auto hoerer =
      std::ranges::find_if(playerList_, [&, this](const Player* player) {
        return player->id() == ghs_[hoererPos];
      });
  if (angesagt <= (*hoerer)->maxBieten_) {
    (*hoerer)->solo_ = true;
    qDebug() << QString::fromStdString((*hoerer)->name()) << "ja" << "solo";
    return "ja";
  } else {
    (*hoerer)->solo_ = false;
    qDebug() << QString::fromStdString((*hoerer)->name()) << "weg";
    return "weg";
  }
}

/* GHS    0   1   2
 *        ---------
 *
 *        0   0   0   -> Ramsch
 *        X   0   0   -> pos 0
 *        X   Y   Z   -> pos 1  || pos 2  || pos 3
 */

// Geber Hoerer Sager ghs_{0, 1, 2} initial condition
// void reizen(int sagerPos = 2, int hoererPos = 1, int ansagen = 0);
void Game::reizen(
    int sagerPos, int hoererPos, int ansagen) {
  auto sager =
      std::ranges::find_if(playerList_, [&, this](const Player* player) {
        return player->id() == ghs_[sagerPos];
      });

  auto hoerer =
      std::ranges::find_if(playerList_, [&, this](const Player* player) {
        return player->id() == ghs_[hoererPos];
      });

  // Base case: If both sager and hoerer can't bid anymore, stop recursion
  if ((*sager)->maxBieten_ <= gereizt_ && (*hoerer)->maxBieten_ <= gereizt_) {
    qDebug() << "Bidding complete. Highest bid:" << gereizt_;
    return;
  }

  // Start bidding process
  while (hoeren(hoererPos, (*sager)->geboten_) == "ja" &&
         ((*sager)->geboten_ < (*sager)->maxBieten_)) {
    (*sager)->geboten_ = bieten();
    qDebug() << QString::fromStdString((*sager)->name()) << "sagt"
             << (*sager)->geboten_;
    gereizt_ = (*sager)->geboten_;
    emit gesagt();
  }

  // Stop recursion if bidding is finished
  if (hoeren(hoererPos, (*sager)->geboten_) == "weg") {
    // (*hoerer)->solo_ = false;
    (*sager)->solo_ = true;
    qDebug() << QString::fromStdString((*sager)->name()) << (*sager)->geboten_
             << "solo";
    return;
  }

  // Update the new hoerer
  hoererPos = (hoeren(hoererPos, (*sager)->geboten_) == "ja") ? 1 : 2;

  // Update sager position
  sagerPos = 0;

  sager = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == ghs_[sagerPos];
  });

  hoerer = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == ghs_[hoererPos];
  });

  // Stop recursion if no one else can bid
  if ((*sager)->maxBieten_ <= gereizt_ || (*hoerer)->maxBieten_ <= gereizt_) {
    qDebug() << "Bidding complete. Final bid:" << gereizt_;
    return;
  }

  // Recurse safely with termination checks
  reizen(sagerPos, hoererPos, gereizt_);
}

int Game::spielwert() { return 0; }

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

  // Farbenspiel "♣", "♥", "♠", "♦"
  if (rule == Rule::Suit) {
    if (firstCard.rank() == "J") requiredSuit = trump_;

    // Case 1: First card is a Jack or trump
    if (firstCard.rank() == "J" || firstCard.suit() == trump_) {
      bool hasTrumpInHand = std::ranges::any_of(
          playerList_.front()->handdeck_.cards(), [this](const Card& c) {
            return c.suit() == trump_ || c.rank() == "J";
          });

      // if (hasJackInHand || hasTrumpSuitInHand)
      if (hasTrumpInHand)
        return card.rank() == "J" || card.suit() == trump_;
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

  // Grand oder Ramsch
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

  // Null
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
    if (card.suit() == firstCard.suit() || card.suit() == trump_ ||
        card.rank() == "J") {
      return std::ranges::all_of(
          trickWithoutLast, [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule == Rule::Grand || rule == Rule::Ramsch) {
    if (card.suit() == firstCard.suit() || card.rank() == "J") {
      return std::ranges::all_of(
          trickWithoutLast, [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule == Rule::Null) {
    if (card.suit() == firstCard.suit()) {
      return std::ranges::all_of(
          trickWithoutLast, [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
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
    qDebug() << QString::fromStdString(playerList_.front()->name())
             << "has the trick now!";
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

    std::rotate(playerList_.begin(), trickholder, playerList_.end());
    qDebug() << "Rotating to:"
             << QString::fromStdString(playerList_.front()->name());

    // moving trick to players tricks
    playerList_.front()->tricks_.push_back(trick_);
    qDebug() << "Trick moved to Trickholder"
             << QString::fromStdString(playerList_.front()->name());

    playerList_.front()->setPoints();

    if (playerList_.front()->handdeck_.cards().size() == 0) finishRound();

  } else {
    // Rotate to the next player if the trick is not full
    std::rotate(playerList_.begin(), playerList_.begin() + 1,
                playerList_.end());
    qDebug() << "Next player:"
             << QString::fromStdString(playerList_.front()->name());
  }

  showPoints();
}

void Game::showPoints() {
  for (const auto& player : playerList_) {
    qDebug() << QString::fromStdString(player->name())
             << " - Total Points: " << player->points();
    qDebug() << "Handdeck size: "
             << playerList_.front()->handdeck_.cards().size();
  }
  qDebug() << "blind size: " << blind_.cards().size();
}

void Game::finishRound() {
  qDebug() << "finishing round ...\n";
  showPoints();
}
