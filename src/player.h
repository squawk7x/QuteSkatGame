#ifndef player_H
#define player_H

#include <QObject>
#include "cardvec.h"

class Player : public QObject {
  // Q_OBJECT

 private:
  int id_;
  std::string name_;

 public:
  CardVec handdeck_ = CardVec(12);  // incl. 2 Karten Austausch für druecken
  CardVec urHanddeck_ = CardVec{10};
  // CardVec urSkat_ = CardVec{2};
  CardVec skat_ = CardVec{2};
  std::vector<CardVec> tricks_{};

  Rule desiredRule_ = Rule::Unset;
  std::string desiredTrump_{};
  bool desiredHand_{};
  bool desiredSchneider_{};
  bool desiredSchneiderAngesagt_{};
  bool desiredSchwarz_{};
  bool desiredSchwarzAngesagt_{};
  bool desiredOuvert_{};

  // int spitzen_{};
  int maxBieten_{};
  int points_{};  // Spiel
  int spieleGewonnen_{};
  int spieleVerloren_{};
  int score_{};  // Turnier

  bool isRobot_{};
  bool isSolo_{};
  bool hasTrick_{};
  bool success_{};

  explicit Player(int id = 0, std::string name = "", bool isRobot = true,
                  int score = 0, CardVec handdeck = CardVec(10),
                  bool hasTrick = false);
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

  // public class methods

  void setDesiredGame();

  // Slots
 public slots:
};

#endif  // game_->player_H
