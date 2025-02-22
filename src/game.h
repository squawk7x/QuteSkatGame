#ifndef GAME_H
#define GAME_H

#include <QObject>

#include "cardvec.h"
#include "player.h"

class Game : public QObject {
  Q_OBJECT

  Rule rule_ = Rule::Suit;
  std::string trumpSuit_ = "♥";

 public:
  Player player_1{1, "Player-1", false};  // isRobot = false
  Player player_2{2, "Player-2"};
  Player player_3{3, "Player-3"};
  CardVec blind_{32};
  CardVec trick_{3};
  CardVec skat_{2};
  std::vector<Player*> playerList_{&player_1, &player_2, &player_3};

  explicit Game(QObject* parent = nullptr);

  void initGame();
  bool isCardValid(const Card& card);

 signals:
  void clearTrickLayout();

 public slots:
  void playCard(const Card& card);
};

#endif  // GAME_H
