#ifndef gameH
#define gameH

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
  // geberHoererSagerPos_[0][1][2] == id für Geber-Hörer-Sager
  std::vector<int> geberHoererSagerPos_{
      1, 2, 3};  // first rotation before round starts -> {3, 1, 2}
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
  void init();
  // void startGame();
  void start();

  int reizen(bool reset = false);
  void geben();
  bool hoeren(int hoererPos);
  bool sagen(int sagerPos);
  void bieten(bool passe = false);
  void druecken(int playerId);
  int spielwert();

  bool isCardValid(const Card& card);
  bool isCardGreater(const Card& card);
  void playCard(Card& card);
  void activateNextPlayer();

  Player& getPlayerById(int id);
  Player& getPlayerByPos(int pos);
  Player& getPlayerByIsSolo();

  void showPoints();
  void finishRound();
  // void newRound();

 signals:
  void started();
  void gesagt(int idSager, int idHoerer, QString antwortSager,
              QString antwortHoerer);
  void clearTrickLayout();
};

#endif  // gameH
