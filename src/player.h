#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>

#include "cardvec.h"

class Player : public QObject {
  // Q_OBJECT

 private:
  int id_;
  std::string name_;
  bool isRobot_;
  int score_;
  CardVec handdeck_;

 public:
  explicit Player(int id = 0, std::string name = "", bool isRobot = true,
                  int score = 0, CardVec handdeck = CardVec(10));
  ~Player() = default;

  friend bool operator<(const Player &lhs, const Player &rhs);
  friend bool operator>(const Player &lhs, const Player &rhs);
  friend bool operator==(const Player &lhs, const Player &rhs);

  // Getters
  int id() const;
  std::string name() const;
  bool isRobot() const;
  int score() const;
  CardVec &handdeck();  // ✅ Return by reference

  // Methods
  int pointsOnHand();

  // Setters
  void setName(const std::string &name);
  void setIsRobot(bool isRobot);

 public slots:
  // void onCountPoints(int shuffles = 1);
};

#endif  // PLAYER_H
