#ifndef player_H
#define player_H

#include <QObject>
#include "cardvec.h"

class Player : public QObject {
  // Q_OBJECT

 private:
  int id_;
  std::string name_;

  int points_{};
  int score_{};

 public:
  bool isRobot_;
  CardVec handdeck_ = CardVec(12);      // including 2 Skat cards
  CardVec skat_ = CardVec(2);           // active player will get the Skat
  /*CardVec playable_ = CardVec(10);*/  // collection of playable cards

  bool hasTrick_{};
  std::vector<CardVec> tricks_{};
  int maxBieten_{};
  bool isSolo_{};

  explicit Player(int id = 0, std::string name = "", bool isRobot = true,
                  int score = 0, CardVec handdeck = CardVec(10),
                  CardVec skat = CardVec(2), bool hasTrick = false);
  ~Player() = default;

  // operator overloading
  friend bool operator<(const Player &lhs, const Player &rhs);
  friend bool operator>(const Player &lhs, const Player &rhs);
  friend bool operator==(const Player &lhs, const Player &rhs);

 private:
  // Setters
  void setName(const std::string &name);
  void setIsRobot(bool isRobot);

 public:
  // public setters
  void setPoints();

  // public getters
  int id() const;
  std::string name() const;
  bool isRobot() const;
  int score() const;
  int points() const;
  int sumTricks();

  // public class methods

  // Slots
 public slots:
};

#endif  // game_->player_H
