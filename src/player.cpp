#include "player.h"

#include <QDebug>

Player::Player(
    int id, std::string name, bool isRobot, int score, CardVec handdeck)
    : id_(id),
      name_(name),
      isRobot_(isRobot),
      score_(score),
      handdeck_(handdeck) {}

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

// Getters
int Player::id() const { return id_; }
std::string Player::name() const { return name_; }
bool Player::isRobot() const { return isRobot_; }
int Player::score() const { return score_; }

CardVec &Player::handdeck() { return handdeck_; }  // ✅ Return by reference

// Methods
int Player::pointsOnHand() {
  int pointsOnHand = 0;
  for (const auto &card : std::as_const(handdeck_.cards())) {
    pointsOnHand += card.value();
  }
  return pointsOnHand;
}

// Setters
void Player::setName(
    const std::string &name) {
  name_ = name;
}

void Player::setIsRobot(
    bool isRobot) {
  isRobot_ = isRobot;
}
