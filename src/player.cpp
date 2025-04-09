#include "player.h"

#include <QDebug>

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

void Player::setDesiredGame() {
  // TODO Null

  // TODO better KI
  std::pair favorite = handdeck_.mostPairInMap(handdeck_.mapCards(Rule::Suit));
  int numJacks = handdeck_.mapCards(Rule::Grand)["J"];

  if (numJacks >= 2 && numJacks + favorite.second >= 3) {
    desiredRule_ = Rule::Suit;
    desiredTrump_ = favorite.first;
    spitzen_ = abs(handdeck_.spitzen(desiredRule_, desiredTrump_));
  } else if (numJacks >= 3) {
    desiredRule_ = Rule::Grand;
    spitzen_ = abs(handdeck_.spitzen(desiredRule_));
  } else {
    desiredRule_ = Rule::Ramsch;
  }
}

// int Player::points() { return points_; }

// public class methods
void Player::setPoints() {
  points_ = 0;

  for (CardVec &trick : tricks_) {
    points_ += trick.points();
  }
}
