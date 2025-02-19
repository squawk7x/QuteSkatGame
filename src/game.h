#ifndef GAME_H
#define GAME_H

#include <QObject>

#include "cardvec.h"
#include "player.h"

class Game : public QObject {
  Q_OBJECT

 public:
  CardVec blind_{32};
  Player player_1{1, "Player-1", false};  // isRobot = false
  Player player_2{2, "Player-2"};
  Player player_3{3, "Player-3"};
  CardVec skat_{2};

  explicit Game(QObject *parent = nullptr);

  void initGame();

 signals:
};

#endif  // GAME_H
