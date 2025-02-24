#include "player.h"

#include <QDebug>

Player::Player(
    int id, std::string name, bool isRobot, int score, CardVec handdeck,
    CardVec skat, bool hasTrick)
    : id_(id),
      name_(name),
      isRobot_(isRobot),
      solo_{},
      points_{},
      score_(score),
      handdeck_(handdeck),
      skat_(skat),
      hasTrick_(hasTrick) {
  // max 10 tricks to collect, reserve space
  tricks_.reserve(10);
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
// public class methods
//
void Player::setPoints() {
  for (CardVec &vec : tricks_) {
    points_ = vec.value();
  }
}
