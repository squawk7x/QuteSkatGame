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
  Rule rule_{};
  std::string trumpSuit_{};
  bool ouvert_{};
  bool hand_{};
  bool schneider_{};
  bool schwarz_{};
  bool alleinspieler{};

  // constructor
  explicit Game(QObject* parent = nullptr);

  // public methods
  void initGame();
  bool isCardInHand(const Card& card);
  bool isCardValid(const Card& card, Rule rule = Rule::Suit);
  bool isCardGreater(const Card& card, Rule rule = Rule::Suit);
  void activateNextPlayer();
  void showPoints();
  void finishRound();

 signals:
  void clearTrickLayout();
  void setCardToPower(const std::string& suit, Rule rule);

  // Slots
 public slots:
  void playCard(const Card& card);
};

#endif  // GAME_H
