#include "game.h"

#include <QDebug>
#include <QThread>
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
  // std::ranges::for_each(suits, [&](const std::string& suit) {
  //   std::ranges::for_each(ranks, [&](const std::string& rank) {
  //     Card card = Card(suit, rank);
  //     blind_.addCard(std::move(card));
  //   });
  // });

  start();
}

// started by Table constructor
void Game::start() {
  // for early restart:
  blind_.cards().clear();
  urSkat_.cards().clear();
  skat_.cards().clear();

  cardsInGame_.cards().clear();
  matrix_.reset();
  reizen(Reizen::Reset);  // reset static int counter in reizen

  // Game members reset
  rule_ = Rule::Unset;
  trump_ = "";
  hand_ = false;
  schneider_ = false;
  schneiderAngesagt_ = false;
  schwarz_ = false;
  schwarzAngesagt_ = false;
  ouvert_ = false;

  gedrueckt_ = false;
  gereizt_ = 0;
  spielwert_ = 0;
  kontra_ = false;
  re_ = false;
  bock_ = false;

  // Player members reset
  for (Player* player : playerList_) {
    player->tricks_.clear();
    player->skat_.cards().clear();

    // reset in Player::setDesiredGame()
    // player->desiredRule_ = Rule::Unset;
    // player->desiredTrump_ = "";
    // player->desiredHand_ = false;
    // player->desiredSchneider_ = false;
    // player->desiredSchneiderAngesagt_ = false;
    // player->desiredSchwarz_ = false;
    // player->desiredSchwarzAngesagt_ = false;
    // player->desiredOuvert_ = false;
  }

  player_1.isRobot_ = false;
  // player_1.maxBieten_ = 999;
  // player_2.maxBieten_ = 0;
  // player_3.maxBieten_ = 0;

  // new blind_
  std::ranges::for_each(suits, [&](const std::string& suit) {
    std::ranges::for_each(ranks, [&](const std::string& rank) {
      Card card = Card(suit, rank);
      blind_.addCard(std::move(card));
    });
  });

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
  qDebug() << "geben...";

  cardsInGame_ = blind_;
  cardsInGame_.sortCardsFor(Rule::Grand, "J");

  qDebug() << "cardsInGame_:";
  cardsInGame_.print();

  blind_.shuffle();

  for (Player* player : playerList_) {
    player->urHanddeck_.cards().clear();
    player->skat_.cards().clear();
  }

  // Distribuite cards 3 - skat(2) - 4 - 3
  for (Player* player : playerList_) {
    for (int i = 1; i <= 3; i++) blind_.moveTopCardTo(player->urHanddeck_);
  }

  urSkat_.cards().clear();
  blind_.moveTopCardTo(urSkat_);
  blind_.moveTopCardTo(urSkat_);
  skat_ = urSkat_;

  for (Player* player : playerList_) {
    for (int i = 1; i <= 4; i++) blind_.moveTopCardTo(player->urHanddeck_);
  }

  for (Player* player : playerList_) {
    for (int i = 1; i <= 3; i++) blind_.moveTopCardTo(player->urHanddeck_);
  }

  // Copies for start, replay, spielwert
  for (Player* player : playerList_) {
    player->handdeck_ = player->urHanddeck_;
  }

  emit gegeben();  // to Table
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

int Game::spielwert(
    Player* player, Spielwert wert) {
  CardVec cards{12};
  Rule rule;
  std::string trump;
  bool hand, schneider, schneiderAngesagt, schwarz, schwarzAngesagt, ouvert;
  int value, spitzen;

  // Reizen
  if (wert == Spielwert::Desired) {
    cards = player->handdeck_;

    rule = player->desiredRule_;
    trump = player->desiredTrump_;
    hand = player->desiredHand_;
    schneider = player->desiredSchneider_;
    schneiderAngesagt = player->desiredSchneiderAngesagt_;
    schwarz = player->desiredSchwarz_;
    schwarzAngesagt = player->desiredSchwarzAngesagt_;
    ouvert = player->desiredOuvert_;

    spitzen = cards.spitzen(rule, trump);
  }

  // finishRound
  else {
    cards = player->urHanddeck_;
    cards += urSkat_;

    rule = rule_;
    trump = trump_;
    hand = hand_;
    schneider = schneider_;
    schneiderAngesagt = schneiderAngesagt_;
    schwarz = schwarz_;
    schwarzAngesagt = schwarzAngesagt_;
    ouvert = ouvert_;

    spitzen = cards.spitzen(rule, trump);
  }

  if (rule == Rule::Grand || rule == Rule::Suit) {
    int stufenzahl = 0;

    stufenzahl += hand +               //
                  schneider +          //
                  schneiderAngesagt +  //
                  schwarz +            //
                  schwarzAngesagt +    //
                  ouvert;              //

    // Auch bei Hand zählt der Skat mit bei Spielwertberechnung
    value = (rule == Rule::Grand) ? 24 : trumpValue.at(trump);

    // spitzen = player->urHanddeck_.spitzen(rule, trump);
    player->urHanddeck_.print();
    qDebug() << "spitzen:" << spitzen << "\n"
             << "hand:" << hand << "\n"
             << "schneider:" << schneider << "angesagt:" << schneiderAngesagt
             << "\n"
             << "schwarz:" << schwarz << "schwarzAngesagt" << schwarzAngesagt
             << "\n"
             << "ouvert" << ouvert << "\n"
             << "value:" << value << "\n"
             << "Spielwert:" << (abs(spitzen) + 1 + stufenzahl) * value;

    return (abs(spitzen) + 1 + stufenzahl) * value;
  }

  else if (rule == Rule::Null) {
    if (ouvert && hand)
      return 59;
    else if (ouvert && not hand)
      return 46;
    else if (not ouvert && hand)
      return 35;
    else
      return 23;

  } else
    // player->maxBieten = 0;
    // Ramsch points set in finishRound();
    return 0;
}

void Game::setMaxBieten() {
  for (Player* player : playerList_) {
    if (not player->isRobot_) {
      player->maxBieten_ = 999;
    }

    else {
      player->setDesiredGame();
      player->maxBieten_ = spielwert(player, Spielwert::Desired);
    }

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

  if (gereizt_ == 0 && sager->maxBieten_ == 0 && hoerer->maxBieten_ >= 18/* &&
      hoerer->maxBieten_ != 999*/) {
    antwortSager = "passe";

    if (hoerer->isRobot()) {
      gereizt_ = reizen();
      antwortHoerer = QString::number(gereizt_);
    }

    else if (not hoerer->isRobot()) {
      antwortHoerer = QString::number(reizen(Reizen::Preview));
      // emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      // return;
    }

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
  }

  if (gereizt_ == 0 && hoerer->maxBieten_ == 0 && sager->maxBieten_ >= 18/* &&
      sager->maxBieten_ != 999*/) {
    antwortHoerer = "passe";

    if (sager->isRobot()) {
      gereizt_ = reizen();
      antwortSager = QString::number(gereizt_);
    }

    else if (not sager->isRobot()) {
      antwortSager = QString::number(reizen(Reizen::Preview));
      // emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
      // return;
    }

    emit geboten(sager->id(), hoerer->id(), antwortSager, antwortHoerer);
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

  qDebug() << "gereizt bis:" << gereizt_;

  // for (Player* player : playerList_) player->isSolo_ = false;

  // if (gereizt_ > 0) {
  Player* player = getPlayerByHighestBid();
  player->isSolo_ = true;

  qDebug() << QString::fromStdString(player->name()) + " isSolo:"
           << player->isSolo_;

  if (player->isRobot())
    roboDruecken(player);  // hand, ouvert wird in roboDruecken entschieden
  else
    emit frageHand();
  // }

  // else if (gereizt_ == 0) {
  //   rule_ = Rule::Ramsch;
  //   emit ruleAndTrump(rule_, "J");
  // }
}

void Game::roboDruecken(
    Player* player) {
  qDebug() << "roboDruecken...";

  if (gedrueckt_) return;

  std::map<std::string, int> map;

  emit ruleAndTrump(player->desiredRule_, player->desiredTrump_,
                    player->desiredHand_, player->desiredSchneiderAngesagt_,
                    player->desiredSchwarzAngesagt_, player->desiredOuvert_);

  // ===================== Hand ====================

  // hand / ouvert / schneiderAngesagt, schwarzAngesagt
  // entscheidet player in setDesiredGame()

  if (player->desiredHand_ == true) return;

  // =================   not hand_  =================

  // hand_ = false;
  skat_ = urSkat_;
  skat_.moveTopCardTo(player->handdeck_);
  skat_.moveTopCardTo(player->handdeck_);
  qDebug() << "Handdeck size robo =" << player->handdeck_.size();

  // emit ruleAndTrump(player->desiredRule_, player->desiredTrump_);

  // ===================== Null ==== =================

  if (rule_ == Rule::Null) {
    // Die höchsten Karten wegdrücken
    for (const std::string& rank : {"A", "K", "Q", "J", "10", "9", "8", "7"}) {
      for (const std::string& suit : {"♣", "♠", "♥", "♦"}) {
        for (const Card& card : player->handdeck_.cards())
          if (player->handdeck_.isCardInside(Card(suit, rank))) {
            if (skat_.size() < 2) player->handdeck_.moveCardTo(card, skat_);
            if (skat_.size() == 2) break;
          }
      }
    }
  }

  // ================= Grand / Suit =================

  else if (rule_ == Rule::Suit || rule_ == Rule::Grand) {
    // alle 12 Karten zählen für spitzen
    qDebug() << QString::fromStdString(player->name()) << "spielt mit / ohne:"
             << player->handdeck_.spitzen(player->desiredRule_,
                                          player->desiredTrump_);

    // TODO Decision Suit or Grand
    map = player->handdeck_.mapCards(Rule::Suit);
    printMap(map);

    std::pair<std::string, int> most = player->handdeck_.mostPairInMap(map);
    emit ruleAndTrump(Rule::Suit, trump_ = most.first, player->desiredHand_,
                      player->desiredSchneiderAngesagt_,
                      player->desiredSchwarzAngesagt_, player->desiredOuvert_);

    // CardVec candidates = CardVec(2);

    // 1. Search for individual 10s
    std::vector<Card> tens;
    for (const Card& card : player->handdeck_.cards()) {
      if (card.rank() == "10") {
        tens.push_back(card);
      }
    }

    for (const Card& ten : tens) {
      bool has_other_same_suit = std::ranges::any_of(
          player->handdeck_.cards(), [&ten](const Card& card) {
            return card.suit() == ten.suit() && card.rank() != "10" &&
                   card.rank() != "J";
          });

      if (!has_other_same_suit) {
        qDebug() << "druecken 1";
        if (skat_.size() < 2) {
          player->handdeck_.moveCardTo(ten, skat_);
        }
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

      // Ensure the card is not already in the candidates and that it fits
      // the criteria
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
  }

  gedrueckt_ = true;
  emit roboGedrueckt();
}

void Game::druecken() {
  qDebug() << "Game::druecken()...";

  if (rule_ == Rule::Ramsch) {
    skat_.cards().clear();
    return;
  }

  else if (skat_.size() == 2) {
    Player* player = getPlayerByIsSolo();

    player->skat_ = skat_;

    qDebug() << QString::fromStdString(player->name()) << "hat gedrückt:";
    for (const Card& card : player->skat_.cards())
      qDebug() << QString::fromStdString(card.str());

    // (game_->skat_.size() == 2) witd mit pbDruecken überprüft
    player->setPoints();

    qDebug() << QString::fromStdString(player->name()) << "drückt"
             << player->points() << "points";
    // Punkte setzen
    skat_.cards().clear();
  }
}

/*
 * empty trick
 *
 * 1 card played     hasSoloPlayed          not hasSoloPlayed
 *                                        (Player could be Solo)
 *
 * 2 cards played    hasSoloPlayed          not hasSoloPlayed
 *                  |             |                |
 *                hasTrick  not hasTrick      Player == Solo
 */

void Game::evaluateOthersCards() {
  Player* player = playerList_.front();

  othersCards_.cards().clear();

  // Welche Karten sind noch im Spiel?
  othersCards_ += playerList_[1]->handdeck_;
  othersCards_ += playerList_[2]->handdeck_;

  if (rule_ != Rule::Ramsch && not player->isSolo_) {
    Player* solo = getPlayerByIsSolo();
    othersCards_ += solo->skat_;
  } else if (rule_ == Rule::Ramsch)
    othersCards_ += urSkat_;

  qDebug() << "=== othersCards_ ===";
  othersCards_.print();

  othersCards_.evaluateCards(rule_, trump_, trickCardFirst_,
                             trickCardStrongest_, Order::Increase);
}

Card& Game::cardByNull_KI() {
  Player* player = playerList_.front();

  evaluateOthersCards();

  player->handdeck_.evaluateCards(rule_, trump_, trickCardFirst_,
                                  trickCardStrongest_, Order::Increase);

  // noch keine Karte ausgespielt
  if (trick_.cards().empty())
    // && not player->handdeck_.lowestRankCard_.isEmpty()) {
    return player->handdeck_.lowestRankCard_;
  // }

  // 1 card played
  else if (trick_.cards().size() == 1) {
    // Solo war Ausspieler, Solo hat Trick
    if (hasSoloPlayed_) {
      // versuche drunter
      if (not player->handdeck_.nextLowerPowerCard_.isEmpty())
        return player->handdeck_.nextLowerPowerCard_;
      // wenn bedienen dann mit höchster Power den Stich nehmen
      else if (not player->handdeck_.highestPowerCard_.isEmpty())
        return player->handdeck_.highestPowerCard_;
      // höchsten Rang abwerfen
      else
        return player->handdeck_.highestRankCard_;
    }
    if (not hasSoloPlayed_) {
      // Mitspieler hat Trick, drunter bleiben
      if (not player->handdeck_.nextLowerPowerCard_.isEmpty())
        return player->handdeck_.nextLowerPowerCard_;
      // nächst höhere power Karte
      else if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        return player->handdeck_.nextHigherPowerCard_;
      // //
      // else if (not player->handdeck_.highestPowerCard_.isEmpty())
      //   return player->handdeck_.highestPowerCard_;
      // abwerfen
      else
        return player->handdeck_.highestRankCard_;
    }
  }

  // 2 cards played
  else if (trick_.cards().size() == 2) {
    // player is not solo player
    if (hasSoloPlayed_) {
      if (hasSoloTrick_) {
        // versuche drunter
        if (not player->handdeck_.nextLowerPowerCard_.isEmpty())
          return player->handdeck_.nextLowerPowerCard_;
        // wenn bedienen dann mit höchster Power den Stich nehmen
        // else if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        //   return player->handdeck_.nextHigherPowerCard_;
        // höchsten Rang abwerfen bzw. Spiel gewonnen da keine Power Karte in
        // der Hand
        else
          return player->handdeck_.highestRankCard_;
      }
      if (not hasSoloTrick_) {
        // nimm Stich oder höchste Power Karte spielen
        if (not player->handdeck_.highestPowerCard_.isEmpty())
          return player->handdeck_.highestPowerCard_;
        // sonst abwerfen
        else
          return player->handdeck_.highestRankCard_;
      }
    }
    // player himself is solo player
    if (not hasSoloPlayed_) {
      // drunter bleiben
      if (not player->handdeck_.nextLowerPowerCard_.isEmpty())
        return player->handdeck_.nextLowerPowerCard_;
      // nächst höhere power Karte - verloren
      else if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        return player->handdeck_.nextHigherPowerCard_;
      // verloren
      else if (not player->handdeck_.highestPowerCard_.isEmpty())
        return player->handdeck_.highestPowerCard_;
      // abwerfen
      else
        return player->handdeck_.highestRankCard_;
    }
  }

  // Fallback
  return player->handdeck_.lowestRankCard_;
}

Card& Game::cardByGrand_KI() {
  Player* player = playerList_.front();

  evaluateOthersCards();

  player->handdeck_.evaluateCards(rule_, trump_, trickCardFirst_,
                                  trickCardStrongest_, Order::Decrease);

  if (trick_.cards().empty()) {
    // höchsten Buben anspielen wenn vorhanden
    if (not player->handdeck_.highestPowerCard_.isEmpty())
      return player->handdeck_.highestPowerCard_;
    // sonst höchsten Rang anspielen
    else
      return player->handdeck_.highestRankCard_;
  }

  // 1 card played
  else if (trick_.cards().size() == 1) {
    // Solo has the trick
    if (hasSoloPlayed_) {
      if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        return player->handdeck_.nextHigherPowerCard_;
      else
        return player->handdeck_.lowestRankCard_;
    }
    if (not hasSoloPlayed_) {
      // if stronger card in cardsInGame_
      if (not player->handdeck_.nextLowerPowerCard_.isEmpty())
        return player->handdeck_.nextLowerPowerCard_;
      // else
      return player->handdeck_.lowestValueCard_;
    }
  }

  else if (trick_.cards().size() == 2) {
    // actual player is not solo player
    if (hasSoloPlayed_) {
      if (hasSoloTrick_) {
        if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
          return player->handdeck_.nextHigherPowerCard_;
      }
      if (not hasSoloTrick_) {
        // if stronger card in cardsInGame_
        return player->handdeck_.highestValueCard_;
      }
    }

    // actual player is solo player
    if (not hasSoloPlayed_) {
      // versuche drüber
      if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        return player->handdeck_.nextHigherPowerCard_;
      // sonst Karte mit kleinstem Wert
      else
        return player->handdeck_.lowestValueCard_;
    }
  }
  // Fallback
  return player->handdeck_.highestRankCard_;
}

Card& Game::cardBySuit_KI() {
  Player* player = playerList_.front();

  evaluateOthersCards();

  player->handdeck_.evaluateCards(rule_, trump_, trickCardFirst_,
                                  trickCardStrongest_, Order::Decrease);

  if (trick_.cards().empty()) {
    // höchsten Buben anspielen wenn vorhanden
    if (not player->handdeck_.highestPowerCard_.isEmpty()) {
      if (player->handdeck_.highestPowerCard_.rank() == "J")
        return player->handdeck_.highestPowerCard_;
    }
    // sonst höchsten Rang anspielen der nicht trump und nicht 10 ist
    else if (player->handdeck_.highestRankCard_.suit() != trump_ &&
             player->handdeck_.highestRankCard_.rank() != "10")
      return player->handdeck_.highestRankCard_;
    else if (not player->handdeck_.lowestPowerCard_.isEmpty())
      return player->handdeck_.lowestPowerCard_;
    else
      return player->handdeck_.lowestValueCard_;
  }

  // 1 card played
  else if (trick_.cards().size() == 1) {
    // Solo has the trick
    if (hasSoloPlayed_) {
      if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        return player->handdeck_.nextHigherPowerCard_;
      else
        return player->handdeck_.lowestRankCard_;
    }
    if (not hasSoloPlayed_) {
      // if stronger card in cardsInGame_
      if (not player->handdeck_.lowestPowerCard_.isEmpty())
        return player->handdeck_.lowestPowerCard_;
      // else
      return player->handdeck_.lowestRankCard_;
    }
  }

  else if (trick_.cards().size() == 2) {
    // actual player is not solo player
    if (hasSoloPlayed_) {
      if (hasSoloTrick_) {
        if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
          return player->handdeck_.nextHigherPowerCard_;
      }
      if (not hasSoloTrick_) {
        // if stronger card in cardsInGame_
        return player->handdeck_.highestValueCard_;
      }
    }

    // actual player is solo player
    if (not hasSoloPlayed_) {
      // versuche drüber
      if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
        return player->handdeck_.nextHigherPowerCard_;
      // sonst Karte mit kleinstem Wert
      else
        return player->handdeck_.lowestValueCard_;
    }
  }
  // Fallback
  return player->handdeck_.lowestRankCard_;
}

// kein Solo player bei Ramsch
Card& Game::cardByRamsch_KI() {
  Player* player = playerList_.front();

  evaluateOthersCards();

  player->handdeck_.evaluateCards(rule_, trump_, trickCardFirst_,
                                  trickCardStrongest_, Order::Decrease);

  if (trick_.cards().empty()) {
    // Buben anspielen
    if (not player->handdeck_.highestPowerCard_.isEmpty() &&
        player->handdeck_.highestPowerCard_.rank() == "J")
      return player->handdeck_.highestPowerCard_;
    // sonst kleinsten Rang anspielen
    else
      return player->handdeck_.lowestRankCard_;
  }

  // 1 card played
  else if (trick_.cards().size() == 1 || trick_.cards().size() == 2) {
    // knapp drunter bleiben
    if (not player->handdeck_.nextLowerPowerCard_.isEmpty())
      return player->handdeck_.nextLowerPowerCard_;
    // knapp drüber gehen
    else if (not player->handdeck_.nextHigherPowerCard_.isEmpty())
      return player->handdeck_.nextHigherPowerCard_;
    // bei wenig Punkten im Stich Buben spielen
    // else if (trick_.cards().size() == 2 && trick_.points() <= 4 &&
    //          player->handdeck_.highestPowerCard_.rank() == "J")
    //   return player->handdeck_.highestPowerCard_;
    // höchste Punkte abwerfen
    else
      return player->handdeck_.highestValueCard_;
  }

  // Fallback
  return player->handdeck_.lowestRankCard_;
}

void Game::autoplay() {
  qDebug() << "autoplay() ...";

  // if (player->isRobot()) {

  if (rule_ != Rule::Unset && (!player_1.handdeck_.cards().empty() ||
                               !player_2.handdeck_.cards().empty() ||
                               !player_3.handdeck_.cards().empty())) {
    Player* player = playerList_.front();

    // to compare with player->handdeck_ cards in cardBy ... functions
    // qDebug() << "          === cardsInGame_ ===";
    // cardsInGame_.print();

    Card&& card = Card{};

    if (rule_ == Rule::Null)
      card = cardByNull_KI();
    else if (rule_ == Rule::Grand)
      card = cardByGrand_KI();
    else if (rule_ == Rule::Suit)
      card = cardBySuit_KI();
    else if (rule_ == Rule::Ramsch)
      card = cardByRamsch_KI();

    if (isCardValid(card)) playCard(std::move(card));

    emit updatePlayerLayout(player->id(), LinkTo::Trick);
    emit updateTrickLayout(card, player->id());
  }
  //}
}

void Game::playCard(
    const Card& card) {
  if (isCardStronger(card)) {
    for (auto& player : playerList_) player->hasTrick_ = false;

    playerList_.front()->hasTrick_ = true;
    qDebug() << QString::fromStdString(playerList_.front()->name())
             << "has the trick now!";
    trickCardStrongest_ = card;
    qDebug() << "hasTrickCard:"
             << QString::fromStdString(trickCardStrongest_.str());
  }

  qDebug() << "playCard(Card& card):" << QString::fromStdString(card.str());

  // cardsInGame (incl. skat)
  cardsInGame_.erase(card);
  cardsInGame_.print();

  // played cards
  matrix_.setField(card);
  matrix_.print();

  if (trick_.cards().empty()) trickCardFirst_ = card;

  if (playerList_.front()->isSolo_) {
    cardBySolo_ = card;
    hasSoloPlayed_ = true;
    hasSoloTrick_ = playerList_.front()->hasTrick_;
  }

  playerList_.front()->handdeck_.moveCardTo(card, trick_);

  // Rotate playerlist
  activateNextPlayer();
}

void Game::activateNextPlayer() {
  // Spiel beenden, sobald Player bei Null Stich gemacht hat

  if (trick_.size() == 3) {
    Player* trickholder = getPlayerByHasTrick();

    // Auch für Null für success in finishRound
    trickholder->tricks_.push_back(trick_);

    if (rule_ == Rule::Null && trickholder->isSolo_) {
      finishRound();
      return;
    }

    trickholder->setPoints();

    if (playerList_.front()->handdeck_.size() == 0) {
      finishRound();
      return;
    }

    auto trickholderIt = std::ranges::find(playerList_, trickholder);
    // Ensure iterator is valid before rotating
    if (trickholderIt != playerList_.end()) {
      std::ranges::rotate(playerList_, trickholderIt);
    }

    // Reset to empty Card;
    trickCardFirst_ = Card();
    cardBySolo_ = Card();
    hasSoloPlayed_ = false;
    hasSoloTrick_ = false;

  } else {
    rng::rotate(playerList_, playerList_.begin() + 1);
  }

  Player* player = playerList_.front();
  qDebug() << "Next player:" << QString::fromStdString(player->name());

  // autoplay();
}

bool Game::isCardValid(
    const Card& card) {
  if (trick_.size() == 3) {
    // trickCardFirst_ = Card();
    trick_.cards().clear();
    emit clearTrickLayout();
    return true;
  }

  Player* player = playerList_.front();
  player->handdeck_.evaluateCards(rule_, trump_, trickCardFirst_,
                                  trickCardStrongest_);

  std::vector<Card>& validCards = player->handdeck_.validCards_;

  if (std::ranges::find(validCards, card) != validCards.end()) {
    return true;
  }

  return false;
}

bool Game::isCardStronger(
    const Card& card) {
  if (trick_.cards().empty()) {
    return true;  // First played card is always strongest
  }

  if (rule_ == Rule::Suit) {
    // Jacks are always trump in "Suit" games
    bool isTrump = (card.suit() == trump_ || card.rank() == "J");
    bool followsSuit = (card.suit() == trickCardFirst_.suit());

    if (isTrump || followsSuit) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(rule_, trump_) > trickCard.power(rule_, trump_);
          });
    }
  } else if (rule_ == Rule::Grand) {
    // Grand: Only Jacks are trump
    bool isJack = (card.rank() == "J");
    bool followsSuit = (card.suit() == trickCardFirst_.suit());

    if (isJack || followsSuit) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(rule_, trump_) > trickCard.power(rule_, trump_);
          });
    }
  } else if (rule_ == Rule::Ramsch) {
    // Ramsch: Like Grand, Jacks are trump
    bool isJack = (card.rank() == "J");
    bool followsSuit = (card.suit() == trickCardFirst_.suit());

    if (isJack || followsSuit) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(rule_, trump_) > trickCard.power(rule_, trump_);
          });
    }
  } else if (rule_ == Rule::Null) {
    // Null: No trumps, lowest rank wins
    if (card.suit() == trickCardFirst_.suit()) {
      return std::ranges::all_of(
          trick_.cards(), [&card, this](const Card& trickCard) {
            return card.power(rule_, trump_) > trickCard.power(rule_, trump_);
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

  return nullptr;
}

Player* Game::getPlayerByMostTricksPoints() {
  auto it = std::ranges::max_element(
      playerList_,
      [](const Player* a, const Player* b) { return a->points_ < b->points_; });

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
    player->setPoints();
    qDebug() << QString::fromStdString(player->name())
             << " - Total Points: " << QString::number(player->points());
  }

  if (rule_ == Rule::Ramsch)
    assert(player_1.points_ + player_2.points_ + player_3.points_ +
               urSkat_.points() ==
           120);
  else if (rule_ == Rule::Grand || rule_ == Rule::Suit)
    assert(player_1.points_ + player_2.points_ + player_3.points_ == 120);

  else if (rule_ == Rule::Null)
    assert(1 == 1);  // TODO

  // Ramsch
  // TODO Schieberamsch
  if (rule_ == Rule::Ramsch) {
    Player* player = getPlayerByMostTricksPoints();

    // Durchmarsch
    if (player->tricks_.size() == 10) {
      player->success_ = true;
      spielwert_ = 120;
      spielwertFinishRound_ = 120;
      player->score_ += spielwert_;
      player->spieleGewonnen_++;
    }

    else {
      player->success_ = false;
      // player->points() += urSkat_.points(); // Skat an Verlierer
      spielwert_ = player->points();
      spielwertFinishRound_ = player->points();
      player->score_ -= spielwert_;
      player->spieleVerloren_++;
    }

    emit resultat();
    return;
  }

  Player* player = getPlayerByIsSolo();

  int multiplicator = 1;
  if (kontra_) multiplicator *= 2;
  if (re_) multiplicator *= 2;
  if (bock_) multiplicator *= 2;

  // Null
  if (rule_ == Rule::Null) {
    spielwert_ = spielwert(player, Spielwert::Played);
    qDebug() << "spielwert_: " << spielwert_;

    player->success_ =
        (player->tricks_.size() == 0);  // only Skat cards in tricks

    bool ueberreizt = gereizt_ > spielwert_;

    // Spielwert ist Wert bis zu dem gereizt wurde
    if (ueberreizt) {
      spielwertFinishRound_ = gereizt_;
      player->score_ -= 2 * gereizt_ * multiplicator;  // TODO acc Skatregeln
    }

    else if (not player->success_) {
      spielwertFinishRound_ = spielwert_;
      player->score_ -= 2 * spielwert_ * multiplicator;

    } else {
      spielwertFinishRound_ = spielwert_;
      player->score_ += spielwert_ * multiplicator;
    }

    player->success_ && not ueberreizt ? player->spieleGewonnen_++
                                       : player->spieleVerloren_++;
  }

  // Grand or Suit
  else if (rule_ == Rule::Grand || rule_ == Rule::Suit) {
    schneider_ = player->points_ >= 90;
    schwarz_ = player->tricks_.size() == 10;
    if (schwarzAngesagt_) schneiderAngesagt_ = true;

    spielwert_ = spielwert(player, Spielwert::Played);
    qDebug() << "spielwert_: " << spielwert_;

    player->success_ = player->points_ >= 61;
    if (schneiderAngesagt_) player->success_ = player->points_ >= 90;
    if (schwarzAngesagt_) player->success_ = (player->tricks_.size() == 10);
    if (rule_ == Rule::Grand && hand_ && ouvert_) {
      player->success_ = (player->tricks_.size() == 10);
      schneiderAngesagt_ = true;
      schwarzAngesagt_ = true;
    }

    bool ueberreizt = gereizt_ > spielwert_;

    // Spielwert ist ein Vielfaches vom Grundwert
    if (ueberreizt) {
      player->success_ = false;

      int stufenzahl = 1;
      int grundwert = (rule_ == Rule::Grand) ? 24 : trumpValue.at(trump_);

      spielwertFinishRound_ = grundwert;
      // Vielfaches vom Grundwert muß >= gereizt sein
      while (spielwertFinishRound_ < gereizt_) {
        stufenzahl++;
        spielwertFinishRound_ = stufenzahl * grundwert;
      }
      player->score_ -= 2 * spielwertFinishRound_ * multiplicator;
    }

    else if (not player->success_) {
      // hand_ = false;
      schneider_ = false;
      schneiderAngesagt_ = false;
      schwarz_ = false;
      schwarzAngesagt_ = false;

      // Spielwert wird ohne schneider / schwarz / angesagt gerechnet
      spielwert_ = spielwert(player, Spielwert::Played);
      spielwertFinishRound_ = spielwert_;
      player->score_ -= 2 * spielwert_ * multiplicator;
    }

    else {
      spielwertFinishRound_ = spielwert_;
      player->score_ += spielwert_ * multiplicator;
    }

    player->success_ && not ueberreizt ? player->spieleGewonnen_++
                                       : player->spieleVerloren_++;
  }

  emit resultat();
}
