#ifndef GAME_H
#define GAME_H

#include <QObject>

#include "cardvec.h"
#include "player.h"

class Game : public QObject {
  Q_OBJECT

 public:
  CardVec blind_{32};
  CardVec trick_{3};
  CardVec skat_{2};
  Player player_1{1, "Player-1", false};  // isRobot = false
  Player player_2{2, "Player-2", true};
  Player player_3{3, "Player-3", true};
  std::vector<Player*> playerList_{&player_1, &player_2, &player_3};
  std::vector<int> ghs_{1, 2, 3};  // ghs_{[0][1][2]} id für geben-hoeren-sagen
  int soloSpieler_id{};
  Rule rule_{};
  std::string trump_{};
  int gereizt_{};
  int spielwert_{};
  bool hand_{};
  bool ouvert_{};
  bool schneider_{};
  bool schwarz_{};

  // constructor
  explicit Game(QObject* parent = nullptr);

  // public methods
  void initGame();

  void startGame();
  int bieten();
  void geben();
  QString hoeren(int hoererPos = 1, int angesagt = 0);
  void reizen(int sagerPos = 2, int hoererPos = 1, int ansagen = 0);
  int spielwert();

  bool isCardValid(const Card& card, Rule rule = Rule::Suit);
  bool isCardGreater(const Card& card, Rule rule = Rule::Suit);
  void playCard(const Card& card);
  void activateNextPlayer();
  void showPoints();
  void finishRound();

 signals:
  void gesagt();
  void clearTrickLayout();
};

#endif  // GAME_H
