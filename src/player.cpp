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

void Player::checkForNull() {
  qDebug() << this->name() << "checkForNull ...";

  bool isNullOk = true;

  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> nullPattern = handdeck_.toPattern(Rule::Null, suit);

    auto last5 = nullPattern | std::views::reverse | std::views::take(5);
    int sumLast5 = std::accumulate(last5.begin(), last5.end(), 0);

    // e.g. AKQJ1987
    // e.g. XXX11100
    isNullOk = isNullOk && sumLast5 >= 3;
    qDebug() << "isNullOk:" << isNullOk;
    if (not isNullOk) break;
  }
  if (isNullOk) desiredRule_ = Rule::Null;

  bool isNullOuvertOk = true;
  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> nullPattern = handdeck_.toPattern(Rule::Null, suit);

    auto last5 = nullPattern | std::views::reverse | std::views::take(5);
    int sumLast5 = std::accumulate(last5.begin(), last5.end(), 0);

    // e.g. AKQJ1987
    // e.g. XXX11001
    isNullOuvertOk =
        isNullOuvertOk && (sumLast5 >= 3 && nullPattern.back() == 1);
    qDebug() << "isNullOuvertOk:" << isNullOuvertOk;
    if (not isNullOk || isNullOuvertOk) break;
  }
  if (isNullOk && isNullOuvertOk) desiredOuvert_ = true;
}

void Player::checkForGrand() {
  qDebug() << this->name() << "checkForGrand ...";

  bool isGrandOk = true;
  bool isJackOk = true;
  bool isSuitOk = true;

  // e.g. JJJJ  A1KQ987
  // e.g. 1011  101XXXX
  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> pattern = handdeck_.toPattern(Rule::Grand, suit);

    /// JJJJ
    auto jackPattern = pattern | std::views::take(4);
    int numJacks = std::accumulate(jackPattern.begin(), jackPattern.end(), 0);
    std::vector<int> jackVec(jackPattern.begin(), jackPattern.end());

    isJackOk =
        isJackOk && (numJacks >= 3 && jackVec != std::vector<int>{0, 0, 1, 1});

    auto topSuitPattern = pattern | std::views::drop(4) |  // drop J J J J
                          std::views::take(3);             // consider A 10 K
    int numTopSuit =
        std::accumulate(topSuitPattern.begin(), topSuitPattern.end(), 0);
    std::vector<int> suitTopVec(topSuitPattern.begin(), topSuitPattern.end());

    auto suitPattern = pattern | std::views::drop(4);  // drop J J J J
    int numSuit = std::accumulate(suitPattern.begin(), suitPattern.end(), 0);
    std::vector<int> suitVec(suitPattern.begin(), suitPattern.end());

    isSuitOk = isSuitOk &&
               ((numTopSuit >= 2 &&
                 suitTopVec !=
                     std::vector<int>{
                         0, 1, 1}) ||  // 2 von den hohen aber nicht 10 und K
                numSuit <= 1);         // max 1 zum wegdrücken

    isGrandOk = isJackOk && isSuitOk;

    qDebug() << "isGrandOk:" << isGrandOk;
    if (not isGrandOk) break;
  }
  if (isGrandOk) desiredRule_ = Rule::Grand;
}

void Player::checkForSuit() {
  qDebug() << this->name() << "checkForSuit ...";

  bool isSuitOk = false;

  // e.g. JJJJ  A1KQ987
  // e.g. 1011  101XXXX
  for (const std::string &suit : {"♣", "♠", "♥", "♦"}) {
    std::vector<int> pattern = handdeck_.toPattern(Rule::Suit, suit);

    auto jackPattern = pattern | std::views::take(4);
    int numJacks = std::accumulate(jackPattern.begin(), jackPattern.end(), 0);
    std::vector<int> jackVec(jackPattern.begin(), jackPattern.end());

    auto suitPattern = pattern | std::views::drop(4);  // drop J J J J
    int numSuits = std::accumulate(suitPattern.begin(), suitPattern.end(), 0);

    std::vector<int> suitVec(suitPattern.begin(), suitPattern.end());

    isSuitOk = isSuitOk || (numJacks + numSuits >= 5);

    qDebug() << "isSuitOk:" << isSuitOk;
    if (isSuitOk) break;
  }
  if (isSuitOk) {
    desiredRule_ = Rule::Suit;
    std::pair favorite =
        handdeck_.mostPairInMap(handdeck_.mapCards(Rule::Suit));
    desiredTrump_ = favorite.first;
  }
}

void Player::setDesiredGame() {
  qDebug() << "setDesiredGame ...";

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
