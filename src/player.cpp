#include "player.h"

#include <QDebug>
#include <ranges>

Player::Player(
    int id, std::string name, bool isRobot, int score, CardVec handdeck,
    bool hasTrick)
    : id_(id), name_(name), isRobot_(isRobot) {
  // max 10 tricks to collect, reserve space
  tricks_.reserve(11);  // max: 10 tricks + Skat
}

// Operators
bool operator<(
    const Player &lhs, const Player &rhs) {
  return lhs.score_ < rhs.score_;
}

bool operator>(
    const Player &lhs, const Player &rhs) {
  return lhs.score_ > rhs.score_;
}

bool operator==(
    const Player &lhs, const Player &rhs) {
  return lhs.score_ == rhs.score_;
}

// private setters
void Player::setName(
    const std::string &name) {
  name_ = name;
}

void Player::setIsRobot(
    bool isRobot) {
  isRobot_ = isRobot;
}

// public getters
int Player::id() const { return id_; }
std::string Player::name() const { return name_; }
bool Player::isRobot() const { return isRobot_; }
int Player::score() const { return score_; }
int Player::points() const { return points_; }

// ============================   Null   ==============================

void Player::checkForNull() {
  qDebug() << QString::fromStdString(this->name()) << "checkForNull ...";

  bool isNullOk = true;    // must be Ok for all suits
  bool isHandOk = true;    // must be Ok for all suits
  bool isOuvertOk = true;  // must be Ok for all suits

  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> pattern = handdeck_.toPattern(Rule::Null, suit);
    // int sumAll8 = std::ranges::fold_left(pattern, 0, std::plus{});
    int sumAll8 = std::accumulate(pattern.begin(), pattern.end(), 0);

    auto last5 = pattern | std::views::drop(3);
    // int sumLast5 = std::ranges::fold_left(last5, 0, std::plus{});
    int sumLast5 = std::accumulate(last5.begin(), last5.end(), 0);
    // e.g. AKQJ1987
    // e.g. 10011100 Risiko fehlende 7 und fehlende 8, mit Ass
    isNullOk = isNullOk && (sumLast5 >= 3);
    qDebug() << "isNullOk:" << isNullOk;
    // e.g. AKQJ1987
    // e.g. 10011010 Risiko fehlende 7 oder fehlende 8, mit Ass
    isHandOk = isHandOk && (pattern[7] == 1 || pattern[6] == 1 || sumAll8 == 0);
    qDebug() << "isHandOk:" << isHandOk;

    // e.g. AKQJ1987
    // e.g. 10011001 Risiko eine Farbe muß eventuell weggedrückt werden
    isOuvertOk = isOuvertOk && (pattern[0] != 1 || sumAll8 <= 1);
    qDebug() << "isOuvertOk:" << isOuvertOk;

    if (not isNullOk) return;
    // return;
    // mit return => Abbruch Null Überprüfung
  }
  if (isNullOk) {
    desiredRule_ = Rule::Null;
    desiredHand_ = isHandOk;
    desiredOuvert_ = isOuvertOk;
  }

  // Testing:
  // desiredRule_ = Rule::Null;
  // desiredOuvert_ = false;
  // desiredHand_ = false;
}

void Player::checkForGrand() {
  qDebug() << QString::fromStdString(this->name()) << "checkForGrand ...";

  bool isGrandOk = false;
  bool isJackOk = false;
  bool isSuitOk = true;    // must be Ok for all suits
  bool isHandOk = true;    // must be Ok for all suits
  bool isOuvertOk = true;  // must be Ok for all suits

  // e.g. JJJJ  A1KQ987
  // e.g. 1011  101XXXX
  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> pattern = handdeck_.toPattern(Rule::Suit, suit);

    // Take JJJJ into account
    auto jackView = pattern | std::views::take(4);
    int numJacks = std::accumulate(jackView.begin(), jackView.end(), 0);
    std::vector<int> jackVector(jackView.begin(), jackView.end());

    isJackOk = (numJacks >= 2 && jackVector != std::vector<int>{0, 0, 1, 1} ||
                numJacks >= 1 && jackVector[0] == 1);

    // take A 10 into account or size of suit
    auto suitView = pattern | std::views::drop(4);  // drop JJJJ
    int numSuits = std::accumulate(suitView.begin(), suitView.end(), 0);
    std::vector<int> suitVector(suitView.begin(), suitView.end());

    auto top2view = suitView | std::views::take(2);  // consider A 10
    std::vector<int> top2vector(top2view.begin(), top2view.end());

    isSuitOk =
        isSuitOk && (top2vector == std::vector<int>{1, 0} ||
                     top2vector == std::vector<int>{1, 1} || numSuits >= 4);

    // lange
    isHandOk = isHandOk && numJacks >= 3 && (numSuits == 0 || numSuits >= 5);

    auto top4view = suitView | std::views::take(4);  // consider A 10
    std::vector<int> top4vector(top4view.begin(), top4view.end());

    isOuvertOk =
        isOuvertOk && numJacks >= 3 &&
        // die 2 höchsten Buben
        (jackVector != std::vector<int>{0, 1, 1, 1} &&
         jackVector != std::vector<int>{1, 0, 1, 1}) &&
        // von einer Farbe keine Karte oder die höchsten
        (numSuits == 0 ||
         (numSuits == 1 && top4vector == std::vector<int>{1, 0, 0, 0}) ||
         (numSuits == 2 && top4vector == std::vector<int>{1, 1, 0, 0}) ||
         (numSuits == 3 && top4vector == std::vector<int>{1, 1, 1, 0}) ||
         (numSuits >= 4 && top4vector == std::vector<int>{1, 1, 1, 1}));

    isGrandOk = isJackOk && isSuitOk;

    qDebug() << "isGrandOk:" << isGrandOk;
    if (not isGrandOk) {
      return;
    }
  }

  if (isGrandOk) {
    desiredRule_ = Rule::Grand;
    desiredTrump_ = "J";
    desiredHand_ = isHandOk;

    // Ouvert requires Hand
    if (isOuvertOk) {
      desiredHand_ = true;
      desiredOuvert_ = isOuvertOk;
      desiredSchneider_ = true;
      desiredSchneiderAngesagt_ = true;
      desiredSchwarz_ = true;
      desiredSchwarzAngesagt_ = true;
    }
  }
}

void Player::checkForSuit() {
  qDebug() << QString::fromStdString(this->name()) << "checkForSuit ...";

  bool isSuitOk = false;   // only for this suit
  bool isHandOk = true;    // must apply to all suits
  bool isOuvertOk = true;  // only for this suit

  // e.g. JJJJ  A1KQ987
  // e.g. 1011  101XXXX
  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> pattern = handdeck_.toPattern(Rule::Suit, suit);

    auto jackView = pattern | std::views::take(4);
    int numJacks = std::accumulate(jackView.begin(), jackView.end(), 0);
    std::vector<int> jackVector(jackView.begin(), jackView.end());

    auto suitView = pattern | std::views::drop(4);  // drop JJJJ
    int numSuits = std::accumulate(suitView.begin(), suitView.end(), 0);

    std::vector<int> suitVector(suitView.begin(), suitView.end());

    isSuitOk = isSuitOk || (numJacks >= 1 && numJacks + numSuits >= 5) ||
               (numSuits >= 6 && suitView[0] == 1 && suitView[1] == 1);

    // check for all suits
    isHandOk = isHandOk && (numSuits == 0 || numSuits >= 3);

    // check for all suits / 2-Farbenspiel
    isOuvertOk = isOuvertOk && (numSuits == 0 || numSuits >= 4);

    // if (isSuitOk) break;  // not return!
  }
  qDebug() << "isSuitOk:" << isSuitOk;

  if (isSuitOk) {
    desiredRule_ = Rule::Suit;

    std::pair favorite =
        handdeck_.mostPairInMap(handdeck_.mapCards(Rule::Suit));
    desiredTrump_ = favorite.first;

    desiredHand_ = isHandOk;

    if (isOuvertOk) {
      desiredHand_ = true;
      desiredOuvert_ = true;
    }
  }
}

void Player::setDesiredGame() {
  qDebug() << "setDesiredGame ...";

  desiredRule_ = Rule::Unset;
  desiredTrump_ = "";
  desiredHand_ = false;
  desiredOuvert_ = false;
  desiredSchneider_ = false;
  desiredSchneiderAngesagt_ = false;
  desiredSchwarz_ = false;
  desiredSchwarzAngesagt_ = false;

  if (desiredRule_ == Rule::Unset) checkForNull();
  if (desiredRule_ == Rule::Unset) checkForGrand();
  if (desiredRule_ == Rule::Unset) checkForSuit();
  if (desiredRule_ == Rule::Unset) desiredRule_ = Rule::Ramsch;
}

// int Player::points() { return points_; }

// public class methods
void Player::setPoints() {
  points_ = 0;

  // if (isSolo_) points_ += skat_.points();
  points_ += skat_.points();

  for (CardVec &trick : tricks_) {
    points_ += trick.points();
  }
}
