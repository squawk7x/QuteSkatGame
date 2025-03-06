#include "game.h"

#include <QDebug>
#include <QThread>

#include "definitions.h"

Game::Game(
    QObject* parent)
    : QObject{parent} {}

// started by Table constructor
void Game::init() {
  // provide the blind
  std::ranges::for_each(suits, [&](const std::string& suit) {
    std::ranges::for_each(ranks, [&](const std::string& rank) {
      Card card = Card(suit, rank);
      blind_.addCard(std::move(card));
    });
  });
}

// started by Table constructor
void Game::start() {
  gereizt_ = 0;
  // reset static int counter in reizen
  reizen(true);  // reizen(bool reset)

  // for testing:
  player_1.isRobot_ = false;
  player_1.maxBieten_ = 216;
  player_2.maxBieten_ = 24;
  player_3.maxBieten_ = 30;

  // Bugfix: Do not iterate over a modified container!
  // trick to blind_
  std::vector<Card> cardsToMove = trick_.cards();

  for (Card& card : cardsToMove) {
    trick_.moveCardTo(std::move(card), blind_);
  }
  cardsToMove = {};

  // players tricks to blind_
  for (Player* player : playerList_) {
    for (CardVec& trick : player->tricks_) {
      std::vector<Card> cardsToMove = trick.cards();  // Copy cards first

      for (Card& card : cardsToMove) {
        trick.moveCardTo(std::move(card), blind_);
      }
    }
  }

  // handdecks to blind_
  for (Player* player : playerList_) {
    std::vector<Card> cardsToMove =
        player->handdeck_.cards();  // Copy cards first

    for (Card& card : cardsToMove) {
      player->handdeck_.moveCardTo(std::move(card), blind_);
    }
  }

  // skat to blind
  cardsToMove = skat_.cards();
  for (Card& card : cardsToMove) {
    skat_.moveCardTo(std::move(card), blind_);
  }
  cardsToMove = {};

  // rotate geberHoererSager
  std::ranges::rotate(geberHoererSagerPos_, geberHoererSagerPos_.begin() + 1);

  geben();
  emit started();
  // bieten();
  // druecken();
}

void Game::geben() {
  blind_.shuffle();
  // qDebug() << "Blind size after shuffling:" << blind_.cards().size();
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

  qDebug() << "blind" << blind_.cards().size();
  qDebug() << "skat" << skat_.cards().size();
  for (Player* player : playerList_)
    qDebug() << player->name() << player->handdeck_.cards().size();
}

int Game::reizen(
    bool reset, int dec) {
  static int counter = 0;

  // Reset when starting a new round
  if (reset) {
    counter = 0;
    return counter;
  }

  constexpr std::array<int, 57> angesagt = {
      18,  20,  22,  23,  24,  27,  30,  33,  35,  36,  40,  44,  45,  46,  50,
      55,  59,  60,  63,  66,  70,  72,  77,  80,  81,  84,  88,  90,  96,  99,
      100, 108, 110, 117, 120, 121, 126, 130, 132, 135, 140, 143, 144, 150, 153,
      156, 162, 165, 168, 170, 180, 187, 192, 198, 204, 210, 216};

  // Adjust counter within valid range
  counter = std::max(0, counter - dec);
  int index = std::min(counter, static_cast<int>(angesagt.size() - 1));

  counter++;  // Increase counter unless at the end
  return angesagt[index];
}

bool Game::hoeren(
    int hoererPos) {
  Player& hoerer = getPlayerByPos(hoererPos);

  // player_1.maxBieten = 216
  // if (!hoerer.isRobot()) {
  //   return true;  // human decides himself
  // }

  bool accepted = (gereizt_ <= hoerer.maxBieten_);

  return accepted;
}

bool Game::sagen(
    int sagerPos) {
  Player& sager = getPlayerByPos(sagerPos);

  // player_1.maxBieten = 216
  // if (!sager.isRobot()) {
  //   return true;  // human decides himself
  // }

  bool accepted = (gereizt_ < sager.maxBieten_);

  return accepted;
}

void Game::bieten() {
  Player* geber = &getPlayerByPos(0);
  Player* hoerer = &getPlayerByPos(1);
  Player* sager = &getPlayerByPos(2);

  int hoererPos = 1;
  int sagerPos = 2;

  QString antwortSager;
  QString antwortHoerer;

  while (hoeren(hoererPos) && sagen(sagerPos)) {
    /*if (sager->isRobot()) */
    gereizt_ = reizen();

    antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";

    if (gereizt_ <= sager->maxBieten_)
      antwortSager = QString::number(gereizt_);
    else
      antwortSager = "weg";

    emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

    // if (!sager->isRobot_ || !hoerer->isRobot_) return;
    return;
  }

  // isSolo nach sagen
  if (!hoeren(hoererPos)) {
    hoerer->isSolo_ = false;
    sager->isSolo_ = true;
    hoererPos = 2;  // sager wird weiterhoerer
  } else {
    hoerer->isSolo_ = true;
    sager->isSolo_ = false;
  }

  // weitersagen
  hoerer = &getPlayerByPos(hoererPos);
  sagerPos = 0;
  sager = geber;

  while (hoeren(hoererPos) && sagen(sagerPos)) {
    /*if (sager->isRobot())*/
    gereizt_ = reizen();

    antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";

    if (gereizt_ <= sager->maxBieten_)
      antwortSager = QString::number(gereizt_);
    else
      antwortSager = "weg";

    emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

    // if (!sager->isRobot_ || !hoerer->isRobot_) return;
    return;
  }

  // Final decision on solo player
  if (!hoeren(hoererPos)) {
    hoerer->isSolo_ = false;
    sager->isSolo_ = true;
  } else {
    hoerer->isSolo_ = true;
    sager->isSolo_ = false;
    emit gesagt(sager->id(), 0, "weg", "");
  }

  if (gereizt_ == 0 && hoerer->maxBieten_ == 0) {
    antwortSager = "weg";
    antwortHoerer = "weg";
    emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
    hoerer->isSolo_ = false;
  }

  if (gereizt_ == 0 && hoerer->maxBieten_ >= 18) {
    gereizt_ = reizen();
    antwortSager = QString::number(gereizt_);
    antwortHoerer = "weg";
    emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
    hoerer->isSolo_ = true;
  }

  qDebug() << "gereizt bis:" << gereizt_;
  for (auto& player : playerList_) {
    qDebug() << QString::fromStdString(player->name()) << player->isSolo_;
  }
}

void Game::druecken(
    int playerId) {}

int Game::spielwert() { return 0; }

bool Game::isCardValid(
    const Card& card) {
  if (trick_.cards().size() == 3) {
    trick_.cards().clear();
    emit clearTrickLayout();
  }

  // if (rule_ != Rule::Suit || rule_ != Rule::Grand || rule_ != Rule::Ramsch
  // ||
  //     rule_ != Rule::Null) {
  //   qDebug() << "Rule not set";
  //   return false;
  // }

  if (trick_.cards().empty()) {
    return true;
  }

  const auto& firstCard = trick_.cards().front();
  std::string requiredSuit = firstCard.suit();

  // Farbenspiel "♣", "♥", "♠", "♦"
  if (rule_ == Rule::Suit) {
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
        playerList_.front()->handdeck_.cards(), [&](const Card& c) {
          return (c.suit() == requiredSuit) && (c.rank() != "J");
        });

    // If the player has a card of the required suit, they must play them
    if (hasRequiredSuitInHand)
      return (card.suit() == requiredSuit) && (card.rank() != "J");
    else
      return true;  // No card of the required suit in hand, any card is valid
  }

  // Grand oder Ramsch
  else if (rule_ == Rule::Grand || rule_ == Rule::Ramsch) {
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
        playerList_.front()->handdeck_.cards(), [&](const Card& c) {
          return (c.suit() == requiredSuit) && (c.rank() != "J");
        });

    // If the player has a card of the required suit, they must play them
    if (hasRequiredSuitInHand)
      return (card.suit() == requiredSuit) && (card.rank() != "J");
    else
      return true;  // No card of the required suit in hand, any card is valid
  }

  // Null
  else if (rule_ == Rule::Null) {
    bool hasRequiredSuitInHand = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(),
        [&](const Card& c) { return c.suit() == requiredSuit; });

    // If the player has a card of the required suit, they must play it
    if (hasRequiredSuitInHand)
      return card.suit() == requiredSuit;
    else
      return true;
  }

  return false;
}

bool Game::isCardGreater(
    const Card& card) {
  if (trick_.cards().empty()) {
    qDebug() << "first Card always greater";
    return true;  // Erste gespielte Karte ist immer "größer"
  }

  const auto& firstCard = trick_.cards().front();

  // Trick ohne die letzte gespielte Karte
  auto trickWithoutLast = std::ranges::subrange(
      trick_.cards().begin(), std::prev(trick_.cards().end()));

  if (rule_ == Rule::Suit) {
    if (card.suit() == firstCard.suit() || card.suit() == trump_ ||
        card.rank() == "J") {
      qDebug() << "first Card:" << QString::fromStdString(firstCard.str());
      qDebug() << "card:" << QString::fromStdString(card.str());
      qDebug() << "trump_" << QString::fromStdString(trump_);
      return std::ranges::all_of(
          trickWithoutLast, [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule_ == Rule::Grand || rule_ == Rule::Ramsch) {
    if (card.suit() == firstCard.suit() || card.rank() == "J") {
      return std::ranges::all_of(
          trickWithoutLast, [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule_ == Rule::Null) {
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
    Card& card) {
  qDebug() << "card played" << QString::fromStdString(card.str());
  // Card Validation in table.cpp connect (...)

  // Move the card from hand to trick
  // playerList_.front()->handdeck_.moveCardTo(std::move(card), trick_);
  playerList_.front()->handdeck_.moveCardTo(std::move(card), trick_);

  //  Check if the card's power is greater than all cards in the trick
  if (isCardGreater(card)) {
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

  // showPoints();
}

Player& Game::getPlayerById(
    int id) {
  auto it = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == id;
  });

  if (it != playerList_.end()) {
    return **it;  // Dereference the iterator**
  } else {
    throw std::runtime_error("Player not found at position " +
                             std::to_string(id));
  }
}

// playerList_ toggles at each move
// geberHoererSagerPos_ toggles at each round
// first round geberHoererSagerPos_{0, 1, 2}, second round
// geberHoererSagerPos_{1, 2, 0}, ... returns the player at
// geberHoererSagerPos_[pos]
Player& Game::getPlayerByPos(
    int pos) {
  auto it = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == geberHoererSagerPos_[pos];
  });

  if (it != playerList_.end()) {
    return **it;  // Dereference the iterator**
  } else {
    throw std::runtime_error("Player not found at position " +
                             std::to_string(pos));
  }
}

Player* Game::getPlayerByIsSolo() {
  auto it = std::ranges::find_if(
      playerList_, [this](const Player* player) { return player->isSolo_; });

  return (it != playerList_.end()) ? *it : nullptr;
}

void Game::showPoints() {
  for (const auto& player : playerList_) {
    qDebug() << QString::fromStdString(player->name()) << " - Total Points: "
             << player->sumTricks() + skat_.value() * player->isSolo_;
  }
}

void Game::finishRound() {
  qDebug() << "finishing round ...\n";
  showPoints();
}
