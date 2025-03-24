#include "game.h"

#include <QDebug>
#include <QThread>
#include <iterator>
#include <ranges>

// #include "KI.h"
#include "helperFunctions.h"

namespace rng = std::ranges;

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

  start();
}

// started by Table constructor
void Game::start() {
  rule_ = Rule::Unset;
  gereizt_ = 0;
  reizen(Reizen::Reset);  // reset static int counter in reizen
  matrix.reset();

  // for testing:
  player_1.isRobot_ = false;
  player_1.maxBieten_ = 216;
  player_2.maxBieten_ = 0;
  player_3.maxBieten_ = 0;

  // Players' tricks to blind
  qDebug() << "Moving cards from players' tricks to blind.";
  int cardsMovedFromPlayerTricks = 0;

  for (Player* player : playerList_) {
    for (CardVec& trick : player->tricks_) {
      std::vector<Card> cardsToMove = trick.cards();

      for (Card& card : cardsToMove) {
        trick.moveCardTo(card, blind_);
        cardsMovedFromPlayerTricks++;
      }
    }
  }
  qDebug() << "Cards moved from players' tricks to blind:"
           << cardsMovedFromPlayerTricks;

  // for restart of a new round inbetween:
  for (Player* player : playerList_) {
    for (const Card& card : player->handdeck_.cards())
      player->handdeck_.moveTopCardTo(blind_);
  }

  for (const Card& card : skat_.cards()) skat_.moveTopCardTo(blind_);

  // rotate geberHoererSager
  std::ranges::rotate(geberHoererSagerPos_, geberHoererSagerPos_.begin() + 1);

  // rotate playerList_ (Hoerer plays first card)
  while (playerList_[0]->id() != geberHoererSagerPos_[1])
    std::ranges::rotate(playerList_, playerList_.begin() + 1);

  qDebug() << "Size of blind before geben:" << blind_.cards().size();

  geben();
}

void Game::geben() {
  blind_.shuffle();
  // qDebug() << "Blind size after shuffling:" << blind_.cards().size();
  // Distribuite cards 3 - skat(2) - 4 - 3
  for (Player* player : playerList_) {
    for (int i = 1; i <= 3; i++) blind_.moveTopCardTo(player->handdeck_);
  }

  blind_.moveTopCardTo(skat_);
  blind_.moveTopCardTo(skat_);
  urSkat_ = skat_;  // Copy for finishRound / Result

  for (Player* player : playerList_) {
    for (int i = 1; i <= 4; i++) blind_.moveTopCardTo(player->handdeck_);
  }
  for (Player* player : playerList_) {
    for (int i = 1; i <= 3; i++) blind_.moveTopCardTo(player->handdeck_);
  }

  emit gegeben();
  setMaxBieten();

  int hoererPos = 1;
  int sagerPos = 2;

  Player* hoerer = &getPlayerByPos(hoererPos);
  Player* sager = &getPlayerByPos(sagerPos);

  emit geboten(sager->id(), hoerer->id(), "sagt", "hört");
}

void Game::setMaxBieten() {
  // TODO: KI
  for (Player* player : playerList_) {
    CardVec& hand = player->handdeck_;

    std::pair favorite = hand.highestPairInMap(hand.JandSuitNumMap());
    int numJacks = hand.JandSuitNumMap()["J"];

    if (player->id() == 1) {
      player->maxBieten_ = 216;
    } else if (numJacks + favorite.second >= 5) {
      player->maxBieten_ = reizwert(player, favorite.first);
    } else
      player->maxBieten_ = 0;
    qDebug() << "Player" << player->id() << "bietet bis:" << player->maxBieten_;
  }
}

int Game::reizen(
    Reizen reizen) {
  static int counter = 0;

  constexpr std::array<int, 58> angesagt = {
      0,   18,  20,  22,  23,  24,  27,  30,  33,  35,  36,  40,  44,  45,  46,
      50,  55,  59,  60,  63,  66,  70,  72,  77,  80,  81,  84,  88,  90,  96,
      99,  100, 108, 110, 117, 120, 121, 126, 130, 132, 135, 140, 143, 144, 150,
      153, 156, 162, 165, 168, 170, 180, 187, 192, 198, 204, 210, 216};

  if (reizen == Reizen::Reset) {
    counter = 0;
    return angesagt[counter];
  }

  if (reizen == Reizen::Preview) {
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

bool Game::sagen(
    int sagerPos) {
  Player& sager = getPlayerByPos(sagerPos);

  bool accepted = (gereizt_ < sager.maxBieten_);

  return accepted;
}

void Game::bieten(
    Bieten bieten) {
  int hoererPos = 1;
  int sagerPos = 2;

  Player* hoerer = &getPlayerByPos(hoererPos);
  Player* sager = &getPlayerByPos(sagerPos);

  QString antwortSager, antwortHoerer;

  // sagen
  while (sagen(sagerPos) && hoeren(hoererPos)) {
    if (!sager->isRobot() && bieten == Bieten::Nein) {
      sager->maxBieten_ = 0;
      antwortSager = "passe";
      antwortHoerer = "hör ich mehr?";
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    if (!hoerer->isRobot() && bieten == Bieten::Nein) {
      hoerer->maxBieten_ = 0;
      antwortSager = QString::number(gereizt_);
      antwortHoerer = "weg";
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    gereizt_ = reizen();
    antwortSager = QString::number(gereizt_);
    antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
    return;
  }

  // Weitersagen
  if (!hoeren(hoererPos)) {
    hoerer->isSolo_ = false;
    sager->isSolo_ = true;
    hoererPos = 2;
  } else {
    hoerer->isSolo_ = true;
    sager->isSolo_ = false;
    hoererPos = 1;
  }

  hoerer = &getPlayerByPos(hoererPos);
  sagerPos = 0;
  sager = &getPlayerByPos(sagerPos);

  while (sagen(sagerPos) && hoeren(hoererPos)) {
    if (!sager->isRobot() && bieten == Bieten::Nein) {
      sager->maxBieten_ = 0;
      antwortSager = "passe";
      antwortHoerer = "mein Spiel";
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    if (!hoerer->isRobot() && bieten == Bieten::Nein) {
      hoerer->maxBieten_ = 0;
      antwortHoerer = "passe";
      antwortSager = QString::number(gereizt_);
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    gereizt_ = reizen();
    antwortSager = QString::number(gereizt_);
    antwortHoerer = hoeren(hoererPos) ? "ja" : "passe";

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
    return;
  }

  if (gereizt_ == 0 && sager->isRobot() && sager->maxBieten_ >= 18) {
    gereizt_ = reizen();
    sager->isSolo_ = true;
    hoerer->isSolo_ = false;
    antwortSager = QString::number(gereizt_);
    antwortHoerer = "passe";
  }

  else if (gereizt_ == 0 && hoerer->isRobot() && hoerer->maxBieten_ >= 18) {
    gereizt_ = reizen();
    sager->isSolo_ = false;
    hoerer->isSolo_ = true;
    antwortSager = "passe";
    antwortHoerer = QString::number(gereizt_);
  }

  // else if (hoeren(hoererPos)) {
  else if (gereizt_ <= hoerer->maxBieten_) {
    sager->isSolo_ = false;
    hoerer->isSolo_ = true;
    antwortSager = "passe";
    antwortHoerer = QString::number(gereizt_);
  }

  else if (gereizt_ <= sager->maxBieten_) {
    sager->isSolo_ = true;
    hoerer->isSolo_ = false;
    antwortSager = QString::number(gereizt_);
    antwortHoerer = "passe";
  }

  else if (gereizt_ == 0 && hoerer->maxBieten_ == 0 && sager->maxBieten_ == 0) {
    sager->isSolo_ = false;
    hoerer->isSolo_ = false;
    antwortSager = "passe";
    antwortHoerer = "passe";

    emit ruleAndTrump(Rule::Ramsch, "J");
  }

  emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
  qDebug() << "gereizt bis:" << gereizt_;

  Player* player = getPlayerByIsSolo();

  if (player) {
    qDebug() << QString::fromStdString(player->name()) << player->isSolo_;
    if (player->isRobot_)
      roboAufheben();  // hand decision made in aufheben
    else
      emit frageHand();
  }
}

int Game::reizwert(
    Player* player, const std::string& suit) {
  int reizwert;

  if (rule_ == Rule::Null) {
    if (ouvert_ && hand_)
      return 59;
    else if (ouvert_)
      return 46;
    else if (hand_)
      return 35;
    else
      return 23;
  } else if (rule_ == Rule::Ramsch) {
    return 0;
  }

  reizwert = (abs(player->handdeck_.mitOhne(suit)) + 1 + hand_ + ouvert_ +
              2 * schneiderAngesagt_ + 2 * schwarzAngesagt_) *
             trumpValue.at(suit);

  return reizwert;
}
void Game::roboAufheben() {
  qDebug() << "aufheben...";

  Player* player = getPlayerByIsSolo();

  if (player->isRobot_ && !hand_) {
    roboDruecken();
  }
}

void Game::roboDruecken() {
  qDebug() << "roboDruecken...";

  Player* player = getPlayerByIsSolo();

  if (player->isRobot()) {
    emit ruleAndTrump(
        Rule::Suit,
        player->handdeck_.highestPairInMap(player->handdeck_.JandSuitNumMap())
            .first);
    emit gedrueckt();
  }
}

void Game::druecken() {
  qDebug() << "druecken...";
  if (skat_.cards().size() == 2) {
    // TODO: KI Karten austauschen-druecken Robots
    // TODO after testing: LinkTo::Trick only for player 1

    Player* player = getPlayerByIsSolo();
    // Skat in Ramsch handled in finishRound
    qDebug() << "soloPlayer:" << player->name();
    // TODO: KI
    if (player) {
      player->tricks_.push_back(std::move(skat_));
    }
  }
}

int Game::spielwert(
    const std::string& suit) {
  return 0;
}

void Game::autoplay() {
  qDebug() << "autoplay() ...";
  // Rule must be set!
  if (rule_ != Rule::Unset && (!player_1.handdeck_.cards().empty() ||
                               !player_2.handdeck_.cards().empty() ||
                               !player_3.handdeck_.cards().empty())) {
    Player* player = playerList_.front();

    // if (player->isRobot()) {
    auto cards = playableCards(player->id());
    Card card = cards.front();
    if (isCardValid(card)) playCard(card);

    emit updateTrickLayout(card, player->id());
    emit updatePlayerLayout(player->id(), LinkTo::Trick);
    // }
  }
}

void Game::playCard(
    const Card& card) {
  if (isCardStronger(card)) {
    for (auto& player : playerList_) player->hasTrick_ = false;

    playerList_.front()->hasTrick_ = true;
    qDebug() << QString::fromStdString(playerList_.front()->name())
             << "has the trick now!";
  }

  qDebug() << "playCard(Card& card):" << QString::fromStdString(card.str());
  matrix.setField(card);
  matrix.print();

  playerList_.front()->handdeck_.moveCardTo(card, trick_);

  // Rotate playerlist
  activateNextPlayer();
}

void Game::activateNextPlayer() {
  if (trick_.cards().size() == 3) {
    Player* trickholder = getPlayerByHasTrick();

    trickholder->tricks_.push_back(trick_);

    qDebug() << "Trick moved to Trickholder"
             << QString::fromStdString(trickholder->name());

    if (playerList_.front()->handdeck_.cards().size() == 0) finishRound();

    auto trickholderIt = std::ranges::find(playerList_, trickholder);
    // Ensure iterator is valid before rotating
    if (trickholderIt != playerList_.end()) {
      std::ranges::rotate(playerList_, trickholderIt);
    }

    qDebug() << "Rotating to:"
             << QString::fromStdString(playerList_.front()->name());

  } else {
    // Rotate to the next player if the trick is not full
    // std::rotate(playerList_.begin(), playerList_.begin() + 1,
    //             playerList_.end());

    rng::rotate(playerList_, playerList_.begin() + 1);

    qDebug() << "Next player:"
             << QString::fromStdString(playerList_.front()->name());
  }

  // show playable cards for activated player
  auto cards = playableCards(playerList_.front()->id());
  qDebug() << "Playable cards:" << QString::fromStdString(cardsToString(cards));

  rng::sort(cards, [](Card& a, Card& b) { return a.hasMorePower(b); });
  qDebug() << "Proposed order 1:"
           << QString::fromStdString(cardsToString(cards));

  rng::sort(cards, [](Card& a, Card& b) { return a.hasMoreValue(b); });
  qDebug() << "Proposed order 2:"
           << QString::fromStdString(cardsToString(cards));

  // autoplay();
}

std::vector<Card> Game::playableCards(
    int playerId) {
  Player& player = getPlayerById(playerId);
  std::vector<Card> handCards = player.handdeck_.cards();

  // Use std::ranges to filter and collect into a vector
  std::vector<Card> playable;
  rng::copy(handCards | std::views::filter([this](const Card& card) {
              return isCardValid(card, true);  // preview = true
            }),
            std::back_inserter(playable));

  // qDebug() << "Playable Cards:"
  //          << QString::fromStdString(cardsToString(playable));

  return playable;
}

bool Game::isCardValid(
    const Card& card, bool preview) {
  if (trick_.cards().size() == 3 && preview) return true;
  // If the trick is full (3 cards), clear it and reset the layout

  if (trick_.cards().size() == 3) {
    trick_.cards().clear();
    emit clearTrickLayout();
  }

  // If trick is empty, any card can be played
  if (trick_.cards().empty()) {
    return true;
  }

  // First card in the trick determines the required suit
  const auto& firstCard = trick_.cards().front();
  std::string requiredSuit = firstCard.suit();

  if (rule_ == Rule::Suit) {
    // Check if the first card is a Jack ('J')
    bool firstCardIsJack = (firstCard.rank() == "J");

    // Check if the first card is a trump suit card
    bool firstCardIsTrump = (firstCard.suit() == trump_);

    // Check if the player has a Jack ('J') or trump suit card in hand
    bool hasJackOrTrump = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(), [this](const Card& c) {
          return c.rank() == "J" || c.suit() == trump_;
        });

    // If the first card is a Jack ('J'), enforce Jack/trump-following rule
    if (firstCardIsJack) {
      if (hasJackOrTrump) {
        return (card.rank() == "J" || card.suit() == trump_);
      }
      return true;  // No Jacks or trump cards in hand, any card can be played
    }

    // If the first card is a trump suit, allow Jacks ('J') to be played
    if (firstCardIsTrump) {
      if (hasJackOrTrump) {
        return (card.rank() == "J" || card.suit() == trump_);
      }
      return true;  // No Jacks or trump cards in hand, any card is valid
    }

    // Normal suit-following rules
    bool hasRequiredSuitInHand = std::ranges::any_of(
        playerList_.front()->handdeck_.cards(), [&](const Card& c) {
          return (c.suit() == requiredSuit) && (c.rank() != "J");
        });

    // If player has a card of the required suit, they must play it
    if (hasRequiredSuitInHand)
      return (card.suit() == requiredSuit) && (card.rank() != "J");

    // Otherwise, they can play any card
    return true;
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

bool Game::isCardStronger(
    const Card& card) {
  if (trick_.cards().empty()) {
    return true;  // First played card is always strongest
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

Player& Game::getPlayerById(
    int id) {
  auto it = std::ranges::find_if(playerList_, [&, this](const Player* player) {
    return player->id() == id;
  });

  if (it != playerList_.end()) {
    return **it;  // Dereference the iterator**
  } else {
    throw std::runtime_error("Player not found with id " + std::to_string(id));
  }
}

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

Player* Game::getPlayerByHasTrick() {
  auto trickholder = rng::find_if(playerList_, &Player::hasTrick_);

  if (trickholder != playerList_.end())
    return *trickholder;
  else {
    qDebug() << "Player not found with hasTrick_ == true";
    return nullptr;
  }
}

Player* Game::getPlayerByIsSolo() {
  auto it = std::ranges::find_if(
      playerList_, [this](const Player* player) { return player->isSolo_; });

  if (it != playerList_.end())
    return *it;
  else {
    qDebug() << "Player not found with isSolo_ == true";
    return nullptr;
  }
}

Player* Game::getPlayerByMostTricksPoints() {
  auto it = std::ranges::max_element(
      playerList_, [](const Player* a, const Player* b) {
        return a->tricksPoints_ < b->tricksPoints_;
      });

  if (it != playerList_.end()) {
    return *it;  // Return player with the most tricks points
  } else {
    qDebug() << "No player found with tricks points";
    return nullptr;  // If no player is found
  }
}

void Game::finishRound() {
  qDebug() << "finishing round ...\n";

  for (const auto& player : playerList_) {
    player->setTricksPoints();
    qDebug() << QString::fromStdString(player->name())
             << " - Total Points: " << QString::number(player->points());
  }

  if (rule_ != Rule::Ramsch)
    assert(player_1.tricksPoints_ + player_2.tricksPoints_ +
               player_3.tricksPoints_ ==
           120);

  // TODO Spielwert
  if (rule_ != Rule::Ramsch) {
    Player* player = getPlayerByIsSolo();
    int points = player->tricksPoints_;

    if (points > 60) {
      player->score_ += spielwert_;
      player->spieleGewonnen_++;
      qDebug() << player->name() << "gewinnt mit" << points << "zu"
               << 120 - points << "Augen";
    } else {
      player->score_ -= 2 * spielwert_;
      player->spieleVerloren_++;
      qDebug() << player->name() << "verliert mit" << points << "zu"
               << 120 - points << "Augen.";
    }
  }

  // TODO 2 Spieler mit gleicher Punktzahl
  if (rule_ == Rule::Ramsch) {
    Player* player = getPlayerByMostTricksPoints();
    qDebug() << player->name();
    player->score_ -= player->sumTricks();
    player->spieleVerloren_++;
  }

  qDebug() << "Blind size finish round:" << blind_.cards().size();

  emit resultat();
}
