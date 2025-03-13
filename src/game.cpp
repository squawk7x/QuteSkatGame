#include "game.h"

#include <QDebug>
#include <QThread>
#include <iterator>
#include <ranges>

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
  player_2.maxBieten_ = 18;
  player_3.maxBieten_ = 20;

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
  // autoplay();
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
    qDebug() << QString::fromStdString(player->name())
             << player->handdeck_.cards().size();
}

int Game::reizen(
    bool reset, bool preview) {
  static int counter = 0;

  constexpr std::array<int, 58> angesagt = {
      0,   18,  20,  22,  23,  24,  27,  30,  33,  35,  36,  40,  44,  45,  46,
      50,  55,  59,  60,  63,  66,  70,  72,  77,  80,  81,  84,  88,  90,  96,
      99,  100, 108, 110, 117, 120, 121, 126, 130, 132, 135, 140, 143, 144, 150,
      153, 156, 162, 165, 168, 170, 180, 187, 192, 198, 204, 210, 216};

  if (reset) {
    counter = 0;
    return angesagt[counter];
  }

  if (preview) {
    return (counter < angesagt.size() - 1) ? angesagt[counter + 1]
                                           : angesagt[counter];
  }

  if (counter < angesagt.size() - 1) {
    counter++;
  }

  return angesagt[counter];
}

bool Game::hoeren(int hoererPos) {
  Player& hoerer = getPlayerByPos(hoererPos);

  bool accepted = (gereizt_ <= hoerer.maxBieten_);

  return accepted;
}

bool Game::sagen(int sagerPos) {
  Player& sager = getPlayerByPos(sagerPos);

  bool accepted = (gereizt_ < sager.maxBieten_);

  return accepted;
}

void Game::bieten(bool passe) {
  int hoererPos = 1;
  int sagerPos = 2;

  Player* hoerer = &getPlayerByPos(hoererPos);
  Player* sager = &getPlayerByPos(sagerPos);

  QString antwortSager;
  QString antwortHoerer;

  while (hoeren(hoererPos) && sagen(sagerPos)) {
    if (!sager->isRobot() && passe == true) {
      sager->maxBieten_ = 0;
      antwortSager = "weg";
      antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";
      break;
    };

    if (!hoerer->isRobot() && passe == true) {
      hoerer->maxBieten_ = 0;
      antwortSager = QString::number(gereizt_);
      antwortHoerer = "passe";
      break;
    };

    gereizt_ = reizen();
    if (gereizt_ <= sager->maxBieten_) {
      antwortSager = QString::number(gereizt_);
      antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";
    } else {
      antwortSager = "weg";
      antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";
      break;
    }

    emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
    return;
  }
  // for break case:
  emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

  // isSolo nach sagen
  if (!hoeren(hoererPos)) {
    hoerer->isSolo_ = false;
    sager->isSolo_ = true;
    hoererPos = 2;  // sager wird weiterhoerer
  } else {
    hoerer->isSolo_ = true;
    sager->isSolo_ = false;
    hoererPos = 1;  // hoerer bleibt weiterhoerer
  }
  hoerer = &getPlayerByPos(hoererPos);

  // weitersagen
  sagerPos = 0;
  sager = &getPlayerByPos(sagerPos);

  while (hoeren(hoererPos) && sagen(sagerPos)) {
    if (!sager->isRobot() && passe == true) {
      sager->maxBieten_ = 0;
      antwortSager = "weg";
      antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";
      break;
    };

    if (!hoerer->isRobot() && passe == true) {
      hoerer->maxBieten_ = 0;
      antwortSager = QString::number(gereizt_);
      antwortHoerer = "passe";
      break;
    };

    gereizt_ = reizen();
    if (gereizt_ <= sager->maxBieten_) {
      antwortSager = QString::number(gereizt_);
      antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";
    } else {
      antwortSager = "weg";
      antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";
      break;
    }

    emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
    return;
  }
  // for break case:
  emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

  // Final decision on solo player
  if (!hoeren(hoererPos)) {
    hoerer->isSolo_ = false;
    sager->isSolo_ = true;
    antwortHoerer = "weg";
    antwortSager = QString::number(gereizt_);
  } else {
    hoerer->isSolo_ = true;
    sager->isSolo_ = false;
    antwortSager = "weg";
    antwortHoerer = QString::number(gereizt_);
  }

  if (gereizt_ == 0 && hoerer->maxBieten_ == 0) {
    // or all players isSolo = true
    hoerer->isSolo_ = false;
    antwortSager = "weg";
    antwortHoerer = "weg";
  }

  if (gereizt_ == 0 && hoerer->maxBieten_ >= 18) {
    hoerer->isSolo_ = true;
    gereizt_ = reizen();
    antwortSager = QString::number(gereizt_);
    antwortHoerer = "weg";
  }

  emit gesagt(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

  qDebug() << "gereizt bis:" << gereizt_;
  for (auto& player : playerList_) {
    qDebug() << QString::fromStdString(player->name()) << player->isSolo_;
  }
}

void Game::druecken(int playerId) {}

void Game::autoplay() {
  qDebug() << "autoplay() ...";

  if (!player_1.handdeck_.cards().empty() ||
      !player_2.handdeck_.cards().empty() ||
      !player_3.handdeck_.cards().empty()) {
    Player* player = playerList_.front();

    printCards(playableCards(player->id()));

    if (player->isRobot()) {
      Card card = playableCards(player->id()).front();

      playCard(card);

      emit refreshTrickLayout(card, player->id());
      emit refreshPlayerLayout(player->id());
    }
  }
}

int Game::spielwert() { return 0; }

bool Game::isCardValid(
    const Card& card) {
  if (trick_.cards().size() == 3) {
    trick_.cards().clear();
    emit clearTrickLayout();
  }

  // if (!(rule_ == Rule::Suit || rule_ == Rule::Grand || rule_ == Rule::Ramsch
  // ||
  //       rule_ == Rule::Null)) {
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

std::vector<Card> Game::playableCards(
    int playerId) {
  Player& player = getPlayerById(playerId);
  std::vector<Card> handCards = player.handdeck_.cards();

  // Use std::ranges to filter and collect into a vector
  std::vector<Card> playable;
  std::ranges::copy(handCards | std::views::filter([this](const Card& card) {
                      return isCardValid(card);
                    }),
                    std::back_inserter(playable));

  return playable;
}

bool Game::isCardGreater(
    const Card& card) {
  if (trick_.cards().empty()) {
    return true;  // First played card is always "greater"
  }

  const auto& firstCard = trick_.cards().front();

  if (rule_ == Rule::Suit) {
    // Jacks are always trump in "Suit" games
    bool isTrump = (card.suit() == trump_ || card.rank() == "J");
    bool followsSuit = (card.suit() == firstCard.suit());

    if (isTrump || followsSuit) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule_ == Rule::Grand) {
    // Grand: Only Jacks are trump
    bool isJack = (card.rank() == "J");
    bool followsSuit = (card.suit() == firstCard.suit());

    if (isJack || followsSuit) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule_ == Rule::Ramsch) {
    // Ramsch: Like Grand, Jacks are trump
    bool isJack = (card.rank() == "J");
    bool followsSuit = (card.suit() == firstCard.suit());

    if (isJack || followsSuit) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  } else if (rule_ == Rule::Null) {
    // Null: No trumps, lowest rank wins
    if (card.suit() == firstCard.suit()) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(trump_, rule_) > trickCard.power(trump_, rule_);
          });
    }
  }

  return false;
}

// Slots:
void Game::playCard(
    Card& card) {
  qDebug() << "playCard(Card& card):" << QString::fromStdString(card.str());

  //  Check if the card's power is greater than all cards in the trick
  if (isCardGreater(card)) {
    // Reset the trick status for all players
    for (auto& player : playerList_) player->hasTrick_ = false;
    // Mark the current player as having the trick
    playerList_.front()->hasTrick_ = true;

    qDebug() << QString::fromStdString(playerList_.front()->name())
             << "has the trick now!";
  }

  // Move the card from hand to trick
  playerList_.front()->handdeck_.moveCardTo(std::move(card), trick_);

  qDebug() << "trick: " << trick_.print();

  // Rotate playerlist
  activateNextPlayer();
}

void Game::activateNextPlayer() {
  // qDebug() << "trick_:";
  // trick_.print();

  if (trick_.cards().size() == 3) {
    // Find the player who has the trick
    auto trickholder =
        std::find_if(playerList_.begin(), playerList_.end(),
                     [](const auto& player) { return player->hasTrick_; });

    std::rotate(playerList_.begin(), trickholder, playerList_.end());
    qDebug() << "Rotating to:"
             << QString::fromStdString(playerList_.front()->name());

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

  // qDebug() << "playable cards:";
  // playableCards(playerList_.front()->id()).print();

  // showPoints();
  // autoplay();
}

Player& Game::getPlayerById(int id) {
  auto it = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == id;
  });

  if (it != playerList_.end()) {
    return **it;  // Dereference the iterator**
  } else {
    throw std::runtime_error("Player not found with id " + std::to_string(id));
  }
}

Player& Game::getPlayerByPos(int pos) {
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

  if (it != playerList_.end())
    return *it;
  else {
    qDebug() << "Player not found with isSolo == true";
    return nullptr;
  }
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
