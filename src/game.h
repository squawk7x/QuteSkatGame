#ifndef gameH
#define gameH

#include <QObject>

// #include "KI.h"
#include "cardvec.h"
#include "definitions.h"
#include "helperFunctions.h"
#include "matrix.h"
#include "player.h"

class Game : public QObject {
  Q_OBJECT

 public:
  CardVec blind_{32};
  CardVec trick_{3};
  CardVec skat_{2};
  CardVec urSkat_{2};
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
  int reizwert_{};
  int spielwert_{};
  bool hand_{};
  bool ouvert_{};
  bool schneiderAngesagt_{};
  bool schneider_{};
  bool schwarzAngesagt_{};
  bool schwarz_{};

  Matrix matrix{};

  // constructor
  explicit Game(QObject* parent = nullptr);

  // public methods
  void init();
  void start();
  void geben();

  int reizen(Reizen reizen = Reizen::Normal);
  void setMaxBieten();
  bool hoeren(int hoererPos);
  bool sagen(int sagerPos);
  void bieten(Bieten bieten = Bieten::Ja);
  void aufheben();
  void druecken();
  int reizwert(Player* player, const std::string& suit = "");
  int spielwert(const std::string& suit = "");
  void autoplay();
  void finishRound();

  bool isCardValid(const Card& card, bool preview = false);
  std::vector<Card> playableCards(int playerId);
  bool isCardStronger(const Card& card);
  void playCard(const Card& card);
  void activateNextPlayer();

  Player& getPlayerById(int id);
  Player& getPlayerByPos(int pos);
  Player* getPlayerByHasTrick();
  Player* getPlayerByIsSolo();
  Player* getPlayerByMostTricksPoints();


 signals:
  void gegeben();
  void geboten(int idSager, int idHoerer, QString antwortSager,
               QString antwortHoerer);

  void frageHand();
  void ruleAndTrump(Rule rule, std::string trump);

  void updateSkatLayout(
      LinkTo dest = LinkTo::Skat);  // connects only id LinkTo::SoloPlayer
  void updatePlayerLayout(
      int playerId,
      LinkTo dest =
          LinkTo::Handdeck);  // connects only if LinkTo::Trick or LinkTo::Skat
  void updateTrickLayout(const Card& card, int playerId);
  void clearTrickLayout();

  void resultat();
};

#endif  // gameH
