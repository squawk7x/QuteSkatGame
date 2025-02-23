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

 public:
  CardVec handdeck_;
  CardVec skat_;  // active player will get the 'Skat'
  std::vector<CardVec> tricks_;
  bool hasTrick_;

  explicit Player(int id = 0, std::string name = "", bool isRobot = true,
                  int score = 0, CardVec handdeck = CardVec(10),
                  CardVec skat = CardVec(2), bool hasTrick = false);
  ~Player() = default;

  friend bool operator<(const Player &lhs, const Player &rhs);
  friend bool operator>(const Player &lhs, const Player &rhs);
  friend bool operator==(const Player &lhs, const Player &rhs);

  // Getters
  int id() const;
  std::string name() const;
  bool isRobot() const;
  int score() const;

  // Methods
  int pointsOnHand();

  // Setters
  void setName(const std::string &name);
  void setIsRobot(bool isRobot);

  // Slots
 public slots:
  // void onCountPoints();
};

#endif  // PLAYER_H
