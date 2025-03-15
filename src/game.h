#ifndef gameH
#define gameH

#include <QObject>

#include "cardvec.h"
#include "player.h"

// Helperfunction
void printCards(
    const auto& coll) {
  for (const auto& elem : coll) {
    qDebug() << QString::fromStdString(elem.str()) << ' ';
  }
}

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
  // geberHoererSagerPos_[0][1][2] == player.id
  std::vector<int> geberHoererSagerPos_{1, 2, 3};
  // first rotation before 1st round starts -> first round {2, 3, 1}
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

  int reizen(bool reset = false, bool preview = false);
  void geben();
  bool hoeren(int hoererPos);
  bool sagen(int sagerPos);
  void bieten(bool passe = false);
  void druecken();
  void autoplay();
  int spielwert();

  bool isCardValid(const Card& card);
  std::vector<Card> playableCards(int playerId);
  bool isCardGreater(const Card& card);
  void playCard(const Card& card);
  void activateNextPlayer();

  Player& getPlayerById(int id);
  Player& getPlayerByPos(int pos);
  Player* getPlayerByHasTrick();
  Player* getPlayerByIsSolo();
  // Player& getPlayerByIsSolo();

  void showPoints();
  void finishRound();
  // void newRound();

 signals:
  void gegeben();
  void geboten(int idSager, int idHoerer, QString antwortSager,
               QString antwortHoerer);
  void frageHand();
  void enableDruecken();

  void clearTrickLayout();
  void refreshSkatLayout(bool hand = false);
  void refreshTrickLayout(const Card& card, int playerId);
  void refreshPlayerLayout(int playerId, MoveTo dest = MoveTo::Trick);
};

#endif  // gameH
