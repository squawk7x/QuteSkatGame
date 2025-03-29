#include "game.h"

#include <QDebug>
#include <QThread>
#include <iterator>
#include <random>
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
  gedrueckt_ = false;
  hand_ = false;
  ouvert_ = false;
  schneiderAngesagt_ = false;
  schwarzAngesagt_ = false;
  schneider_ = false;
  schwarz_ = false;
  reizen(Reizen::Reset);  // reset static int counter in reizen
  gereizt_ = 0;
  matrix.reset();

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
  sager = &getPlayerByPos(2);
  hoerer = &getPlayerByPos(1);

  // rotate playerList_ (Hoerer plays first card)
  while (playerList_[0]->id() != geberHoererSagerPos_[1])
    std::ranges::rotate(playerList_, playerList_.begin() + 1);

  qDebug() << "Size of blind before geben:" << blind_.size();

  geben();
}

void Game::geben() {
  blind_.shuffle();

  // qDebug() << "Blind size after shuffling:" << blind_.size();

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
  bieten();
}

int Game::reizen(
    Reizen reizen) {
  static int counter = 0;

  constexpr std::array<int, 73> gereizt = {
      0,   18,  20,  22,  23,  24,  27,  30,  33,  35,  36,  40,  44,  45,  46,
      48,  50,  54,  55,  59,  60,  63,  66,  70,  72,  77,  80,  81,  84,  88,
      90,  96,  99,  100, 108, 110, 117, 120, 121, 126, 130, 132, 135, 140, 143,
      144, 150, 153, 154, 156, 160, 162, 165, 168, 170, 171, 176, 180, 187, 189,
      190, 192, 198, 200, 204, 207, 209, 210, 216, 220, 228, 240, 264};

  if (reizen == Reizen::Reset) {
    counter = 0;
    return gereizt[counter];
  }

  if (reizen == Reizen::Last) {
    return gereizt[counter];
  }

  if (reizen == Reizen::Preview) {
    return (counter < gereizt.size() - 1) ? gereizt[counter + 1]
                                          : gereizt[counter];
  }

  if (counter < gereizt.size() - 1) {
    counter++;
  }

  return gereizt[counter];
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

  reizwert = (abs(player->handdeck_.spitzen(suit)) + 1 + hand_ + ouvert_ +
              2 * schneiderAngesagt_ + 2 * schwarzAngesagt_) *
             trumpValue.at(suit);

  return reizwert;
}

void Game::setMaxBieten() {
  // TODO: KI
  for (Player* player : playerList_) {
    CardVec& hand = player->handdeck_;

    std::pair favorite = hand.mostPairInMap(hand.mapCards(Rule::Suit));
    int numJacks = hand.mapCards(Rule::Grand)["J"];

    if (player->id() == 1) {
      player->maxBieten_ = 216;
    } else if (numJacks >= 1 && numJacks + favorite.second >= 5) {
      player->maxBieten_ = reizwert(player, favorite.first);
    } else
      player->maxBieten_ = 0;

    qDebug() << "Player" << player->id() << "bietet bis:" << player->maxBieten_;
  }
}

// helperfunction für bieten antwortHoerer (== 0: höre / > 0: ja)
int Game::counter(
    Reset reset) {
  int static counts = 0;
  if (reset == Reset::Yes) {
    counts = 0;
    return 0;
  }
  return counts++;
}

void Game::bieten(
    Passen passen) {
  int geberPos = 0;
  int hoererPos = 1;
  int sagerPos = 2;

  hoerer = &getPlayerByPos(hoererPos);
  sager = &getPlayerByPos(sagerPos);

  QString antwortSager, antwortHoerer;

  // Sagen
  while (gereizt_ <= sager->maxBieten_) {
    if (gereizt_ > hoerer->maxBieten_ ||
        !hoerer->isRobot() && passen == Passen::Ja) {
      hoerer->maxBieten_ = 0;
      antwortHoerer = "passe";
      antwortSager = QString::number(gereizt_);
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    if (gereizt_ == sager->maxBieten_ ||
        !sager->isRobot() && passen == Passen::Ja) {
      sager->maxBieten_ = 0;
      antwortSager = "passe";
      antwortHoerer = QString::number(gereizt_);
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    // (!sager->isRobot()) handled via pbBieten in table.cpp
    if (sager->isRobot()) gereizt_ = reizen();

    if (sager->isRobot())
      antwortSager = QString::number(gereizt_);

    else if (!sager->isRobot())
      antwortSager = QString::number(reizen(Reizen::Preview));

    antwortHoerer = (gereizt_ <= hoerer->maxBieten_) ? "ja" : "passe";
    if (gereizt_ == 0 && !hoerer->isRobot()) antwortHoerer = "höre";

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

    return;
  }

  if (gereizt_ > hoerer->maxBieten_) {
    hoererPos = 2;
  } else {
    hoererPos = 1;
  }

  hoerer = &getPlayerByPos(hoererPos);
  sagerPos = geberPos;  // Geber ist Weitersager
  sager = &getPlayerByPos(sagerPos);

  // Weitersagen
  while (gereizt_ <= sager->maxBieten_) {
    if (gereizt_ > hoerer->maxBieten_ ||
        !hoerer->isRobot() && passen == Passen::Ja) {
      hoerer->maxBieten_ = 0;
      antwortHoerer = "passe";
      antwortSager = QString::number(gereizt_);
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    if (gereizt_ == sager->maxBieten_ ||
        !sager->isRobot() && passen == Passen::Ja) {
      sager->maxBieten_ = 0;
      antwortSager = "passe";
      antwortHoerer = QString::number(gereizt_);
      emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      break;
    }

    if (sager->isRobot()) gereizt_ = reizen();

    // (!sager->isRobot()) handled via pbBieten in table.cpp
    if (sager->isRobot())
      antwortSager = QString::number(gereizt_);

    else if (!sager->isRobot())
      antwortSager = QString::number(reizen(Reizen::Preview));

    antwortHoerer = (gereizt_ <= hoerer->maxBieten_) ? "ja" : "passe";
    if (counter() == 0 && !hoerer->isRobot()) antwortHoerer = "höre";
    counter();

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

    return;
  }

  for (Player* player : playerList_) player->isSolo_ = false;

  if (gereizt_ == 0 && hoerer->maxBieten_ == 0 && sager->maxBieten_ == 0 ||
      gereizt_ == 0 && hoerer->maxBieten_ == 0 && passen == Passen::Ja ||
      gereizt_ == 0 && sager->maxBieten_ == 0 && passen == Passen::Ja) {
    antwortSager = "passe";
    antwortHoerer = "passe";
    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);

    rule_ = Rule::Ramsch;
    emit ruleAndTrump(rule_, "J");

    qDebug() << "Ramsch!";

    return;
  }

  if (gereizt_ == 0 && sager->maxBieten_ == 0 && hoerer->maxBieten_ >= 18) {
    antwortSager = "passe";

    if (hoerer->isRobot()) gereizt_ = reizen();

    if (hoerer->isRobot())
      antwortSager = QString::number(gereizt_);

    else if (!hoerer->isRobot())
      antwortHoerer = QString::number(reizen(Reizen::Preview));

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
  }

  if (gereizt_ == 0 && hoerer->maxBieten_ == 0 && sager->maxBieten_ >= 18) {
    antwortHoerer = "passe";

    if (sager->isRobot()) gereizt_ = reizen();

    if (sager->isRobot())
      antwortSager = QString::number(gereizt_);

    else if (!sager->isRobot())
      antwortSager = QString::number(reizen(Reizen::Preview));

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
  }

  qDebug() << "gereizt bis:" << gereizt_;

  for (Player* player : playerList_) player->isSolo_ = false;

  if (rule_ != Rule::Ramsch) {
    Player* player = getPlayerByHighestBid();
    player->isSolo_ = true;

    qDebug() << QString::fromStdString(player->name()) + " isSolo:"
             << player->isSolo_;

    if (player->isRobot())
      roboDruecken(player);  // hand wird in roboDruecken entschieden
    else
      emit frageHand();
  }
}

bool Game::isNullOk(
    Player* player) {
  std::vector<int> pattern(8);

  for (const std::string& suit : {"♣", "♠", "♥", "♦"}) {
    pattern = player->handdeck_.toPattern(Rule::Null, suit);

    // Ouvert bei e.g 0 0 0 1 0 1 0 1
    if (pattern.back() != 1) ouvert_ = false;

    int zeroCount = 0;
    bool foundFirstOne = false;

    // ! ok 0 0 1 1 1 0 0 1
    for (int value : pattern) {
      if (value == 1) {
        if (foundFirstOne && zeroCount > 1) return false;
        zeroCount = 0;
        foundFirstOne = true;
      } else if (foundFirstOne) {
        zeroCount++;
      }
    }
  }
  return true;
}

void Game::roboDruecken(
    Player* player) {
  qDebug() << "roboDruecken...";
  std::map<std::string, int> map;

  if (gedrueckt_) return;

  if (rule_ == Rule::Ramsch) {
    return;
  }

  // ===================== Null ====================
  hand_ = true;

  if (isNullOk(player)) {
    if (gereizt_ <= 59 && hand_ && ouvert_) emit ruleAndTrump(Rule::Null, "");
    if (gereizt_ <= 35 && hand_) emit ruleAndTrump(Rule::Null, "");

    gedrueckt_ = true;
    emit roboGedrueckt();
    return;
  }

  hand_ = false;

  skat_.moveTopCardTo(player->handdeck_);
  skat_.moveTopCardTo(player->handdeck_);
  qDebug() << "Handdeck size robo =" << player->handdeck_.size();

  // Die höchsten Karten wegdrücken
  for (const std::string& rank : {"A", "10", "K", "Q", "9", "8", "7"}) {
    for (const std::string& suit : {"♣", "♠", "♥", "♦"}) {
      for (const Card& card : player->handdeck_.cards())
        if (player->handdeck_.isCardInside(Card(suit, rank))) {
          if (skat_.size() < 2) player->handdeck_.moveCardTo(card, skat_);
          if (skat_.size() == 2) break;
        }
    }
  }

  if (isNullOk(player)) {
    if (gereizt_ <= 23) emit ruleAndTrump(Rule::Null, "");
    if (gereizt_ <= 46 && ouvert_) emit ruleAndTrump(Rule::Null, "");

    gedrueckt_ = true;
    emit roboGedrueckt();
    return;
  }

  // Karten vom Skat wieder zurück in Handdeck
  skat_.moveTopCardTo(player->handdeck_);
  skat_.moveTopCardTo(player->handdeck_);
  qDebug() << "Handdeck size robo =" << player->handdeck_.size();

  // ================= Suit / Grand =================
  // Suit or Grand

  // TODO Decision Suit or Grand
  map = player->handdeck_.mapCards(Rule::Suit);
  printMap(map);
  std::pair<std::string, int> most = player->handdeck_.mostPairInMap(map);
  emit ruleAndTrump(Rule::Suit, trump_ = most.first);

  if (rule_ == Rule::Suit || rule_ == Rule::Grand) {
    // CardVec candidates = CardVec(2);

    // 1. Search for individual 10s
    auto tens =
        player->handdeck_.cards() | std::views::filter([](const Card& card) {
          return card.rank() == "10";
        });

    for (const Card& ten : tens) {
      bool has_other_same_suit = std::ranges::any_of(
          player->handdeck_.cards(), [&ten](const Card& card) {
            return card.suit() == ten.suit() && card.rank() != "10" &&
                   card.rank() != "J";
          });

      if (!has_other_same_suit) {
        qDebug() << "druecken 1";
        if (skat_.size() < 2) player->handdeck_.moveCardTo(ten, skat_);
      }
    }

    // 2. Find a suit with exactly 2 cards but no Ace
    for (const auto& [suit, count] : player->handdeck_.mapCards(Rule::Suit)) {
      if (count <= 2) {
        auto pair_cards = player->handdeck_.cards() |
                          std::views::filter([&](const Card& card) {
                            return card.suit() == suit && card.rank() != "A" &&
                                   card.rank() != "J";
                          });

        // Check if exactly 2 cards exist and no Ace is present
        std::vector<Card> selected_cards(pair_cards.begin(), pair_cards.end());
        if (selected_cards.size() == 2) {
          qDebug() << "Druecken: Pair found in suit "
                   << QString::fromStdString(suit);

          // Move both cards to skat
          qDebug() << "druecken 2";
          for (const Card& card : selected_cards) {
            if (skat_.size() < 2) {
              player->handdeck_.moveCardTo(card, skat_);
            }
          }
          break;
        }
      }
    }

    // 3. Now add low cards from the weakest suit, but only if it's the only
    // card left in the suit and not Ace or Jack
    std::pair<std::string, int> fewest = player->handdeck_.fewestPairInMap(map);

    for (const Card& card : player->handdeck_.cards()) {
      if (skat_.size() >= 2) break;
      // Check if the card belongs to the weakest suit and it is the only card
      // of that suit (excluding Ace and Jack)
      if (card.suit() == fewest.first && card.rank() != "A" &&
          card.rank() != "J") {
        // Check if it's the only card of the same suit
        int same_suit_count = std::ranges::count_if(
            player->handdeck_.cards(), [&card](const Card& other_card) {
              return other_card.suit() == card.suit() &&
                     other_card.rank() != "J" && other_card.rank() != "A";
            });

        if (same_suit_count == 0) {
          qDebug() << "druecken 3";
          if (skat_.size() < 2) player->handdeck_.moveCardTo(card, skat_);
        }
      }
    }

    // Sort the cards by their power in descending order
    std::vector<Card> sorted_cards = player->handdeck_.cards();

    std::sort(
        sorted_cards.begin(), sorted_cards.end(),
        [](const Card& a, const Card& b) { return a.value() > b.value(); });

    // 4. Iterate over the sorted cards
    for (const Card& card : sorted_cards) {
      if (skat_.size() >= 2) break;

      // Ensure the card is not already in the candidates and that it fits the
      // criteria
      if (std::find(skat_.cards().begin(), skat_.cards().end(), card) ==
              skat_.cards().end() &&
          card.suit() != trump_ && card.rank() != "J" &&
          card.suit() == fewest.first && card.rank() != "A") {
        qDebug() << "druecken 4";
        if (skat_.size() < 2) player->handdeck_.moveCardTo(card, skat_);
      }
    }

    // 5. If still fewer than 2 candidates, just pick any 2 cards
    // Sort the cards by their power in ascending order (lowest first)
    sorted_cards = player->handdeck_.cards();
    std::sort(
        sorted_cards.begin(), sorted_cards.end(),
        [](const Card& a, const Card& b) { return a.value() > b.value(); });

    // Add cards to candidates until there are 2 cards
    while (skat_.size() < 2) {
      for (const Card& card : sorted_cards) {
        if (std::find(skat_.cards().begin(), skat_.cards().end(), card) ==
                skat_.cards().end() &&
            card.suit() != trump_ && card.rank() != "J" && card.rank() != "A") {
          qDebug() << "druecken 5";
          if (skat_.size() < 2) player->handdeck_.moveCardTo(card, skat_);
          if (skat_.size() == 2) break;
        }
      }
    }

    qDebug() << "Robo hat gedrückt:";
    for (const Card& card : skat_.cards())
      qDebug() << QString::fromStdString(card.str());

    gedrueckt_ = true;
    emit roboGedrueckt();
  }
}

void Game::druecken() {
  qDebug() << "druecken...";

  Player* player = getPlayerByIsSolo();

  if (player && skat_.size() == 2) {
    qDebug() << "soloPlayer:" << player->name();

    player->tricks_.push_back(std::move(skat_));
  }
}

void Game::setSpielwertGereizt() {
  if (rule_ == Rule::Suit || rule_ == Rule::Grand) {
    Player* player = getPlayerByIsSolo();

    mitOhne_ = player->handdeck_.spitzen(trump_);

    int counter = 0;

    if (ouvert_) counter++;
    if (schneiderAngesagt_) counter++;  // Schneider angesagt
    if (schwarzAngesagt_) counter++;    // Schwarz angesagt

    if (rule_ == Rule::Suit)
      spielwertGereizt_ =
          (abs(mitOhne_) + 1 + hand_ + counter) * trumpValue.at(trump_);

    else if (rule_ == Rule::Grand) {
      // Grand Hand 36
      if (hand_ && !ouvert_)
        spielwertGereizt_ = (abs(mitOhne_) + 1 + counter) * 36;
      // Grand ouvert immer hand schneider schwarz
      else if (ouvert_)
        spielwertGereizt_ = (abs(mitOhne_) + 1 + 4) * 24;
      // Grand 24
      else
        spielwertGereizt_ = (abs(mitOhne_) + 1 + counter) * 24;
    }
  }

  else if (rule_ == Rule::Null) {
    if (!hand_ && !ouvert_)
      spielwertGereizt_ = 23;
    else if (hand_ && !ouvert_)
      spielwertGereizt_ = 35;
    else if (ouvert_ && !hand_)
      spielwertGereizt_ = 46;
    else if (ouvert_ && hand_)
      spielwertGereizt_ = 59;
  }

  else if (rule_ == Rule::Ramsch) {
    // Spielwert Gespielt: Verlierer mit max Trickpoints Minuspunkte
  }
}

void Game::setSpielwertGespielt() {
  if (rule_ == Rule::Suit || rule_ == Rule::Grand) {
    Player* player = getPlayerByIsSolo();
    int counter = 0;

    // hand_ separatly evaluated for suit and Grand
    if (ouvert_) counter++;
    if (player->tricksPoints_ >= 90) counter++;  // Schneider erreicht
    if (schneiderAngesagt_) counter++;           // Schneider angesagt
    if (player->tricks_.size() == 11)
      counter++;                      // Schwarz erreicht (11 = 10 + skat)
    if (schwarzAngesagt_) counter++;  // Schwarz angesagt +2!

    // TODO use handdeck/cards from player
    if (rule_ == Rule::Suit)
      spielwertGespielt_ =
          (abs(mitOhne_) + 1 + hand_ + counter) * trumpValue.at(trump_);

    if (rule_ == Rule::Grand) {
      // Grand Hand 36
      if (hand_ && !ouvert_)
        spielwertGereizt_ = (abs(mitOhne_) + 1 + counter) * 36;
      // Grand ouvert immer hand schneider schwarz
      else if (ouvert_)
        spielwertGereizt_ = (abs(mitOhne_) + 1 + 6) * 24;
      // Grand 24
      else
        spielwertGereizt_ = (abs(mitOhne_) + 1 + counter) * 24;
    }
  }

  else if (rule_ == Rule::Null) {
    if (!hand_ && !ouvert_)
      spielwertGespielt_ = 23;
    else if (hand_ && !ouvert_)
      spielwertGespielt_ = 35;
    else if (ouvert_ && !hand_)
      spielwertGespielt_ = 46;
    else if (ouvert_ && hand_)
      spielwertGespielt_ = 59;
  }

  else if (rule_ == Rule::Ramsch) {
    Player* player = getPlayerByMostTricksPoints();
    spielwertGespielt_ = player->tricksPoints_;
  }
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

    // Create a random number generator
    std::random_device rd;
    std::mt19937 g(rd());
    rng::shuffle(cards, g);

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
  if (trick_.size() == 3) {
    Player* trickholder = getPlayerByHasTrick();

    trickholder->tricks_.push_back(trick_);

    qDebug() << "Trick moved to Trickholder"
             << QString::fromStdString(trickholder->name());

    if (playerList_.front()->handdeck_.size() == 0) finishRound();

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

  qDebug() << "Playable Cards:"
           << QString::fromStdString(cardsToString(playable));

  return playable;
}

bool Game::isCardValid(
    const Card& card, bool preview) {
  if (trick_.size() == 3 && preview) return true;
  // If the trick is full (3 cards), clear it and reset the layout

  if (trick_.size() == 3) {
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
  auto soloPlayer = rng::find_if(playerList_, &Player::isSolo_);

  if (soloPlayer != playerList_.end())
    return *soloPlayer;
  else {
    qDebug() << "Player not found with isSolo_ == true";

    return nullptr;
  }
}

Player* Game::getPlayerByHighestBid() {
  if (playerList_.empty()) return nullptr;

  auto highestBidder = std::ranges::max_element(
      playerList_, {},     // Default comparator (operator<)
      &Player::maxBieten_  // Projection: compares based on maxBieten_
  );

  if (highestBidder != playerList_.end() && (*highestBidder)->maxBieten_ > 0)
    return *highestBidder;

  qDebug() << "No player has placed a bid.";

  return nullptr;
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

  setSpielwertGespielt();
  bool ueberreizt = gereizt_ < spielwertGespielt_;
  qDebug() << "Spielwert Gespielt: " << spielwertGespielt_;

  for (const auto& player : playerList_) {
    player->setTricksPoints();
    qDebug() << QString::fromStdString(player->name())
             << " - Total Points: " << QString::number(player->points());
  }
  // TODO Kontra / Re
  if (rule_ == Rule::Grand || rule_ == Rule::Suit) {
    Player* player = getPlayerByIsSolo();
    int points = player->tricksPoints_;

    // Grand hand ouvert
    if (rule_ == Rule::Grand && hand_ && ouvert_) {
      // Spieler muß scharz erreichen um zu gewinnen
      if (player->tricks_.size() == 11) {  // 10 tricks + skat
        player->score_ += spielwertGespielt_;
        player->spieleGewonnen_++;
      } else {
        player->score_ -= 2 * spielwertGespielt_;
        player->spieleVerloren_++;
      }
      return;
    }

    if (points > 60 && !ueberreizt) {
      player->score_ += spielwertGespielt_;
      player->spieleGewonnen_++;
    } else {
      if (ueberreizt)
        player->score_ -= 2 * gereizt_;  // TODO acc Skatregeln
      else
        player->score_ -= 2 * spielwertGespielt_;
      player->spieleVerloren_++;
    }

    if (rule_ == Rule::Null) {
      Player* player = getPlayerByIsSolo();

      if (player->tricks_.empty() && gereizt_ <= spielwertGespielt_) {
        player->score_ += spielwertGespielt_;
        player->spieleGewonnen_++;
      } else {
        if (ueberreizt)
          player->score_ -= 2 * gereizt_;  // TODO acc Skatregeln
        else
          player->score_ -= 2 * spielwertGespielt_;
        player->spieleVerloren_++;
      }
    }

    // TODO 2 Spieler mit gleicher Punktzahl
    if (rule_ == Rule::Ramsch) {
      Player* player = getPlayerByMostTricksPoints();

      player->score_ -= player->sumTricks();
      player->spieleVerloren_++;

      // TODO Durchmarsch
    }

    if (rule_ != Rule::Ramsch) {
      assert(player_1.tricksPoints_ + player_2.tricksPoints_ +
                 player_3.tricksPoints_ ==
             120);
    }

    emit resultat();
  }
}
