#ifndef gameH
#define gameH

#include <QObject>

// #include "KI.h"
#include "cardvec.h"
#include "definitions.h"
// #include "helperFunctions.h"
#include "matrix.h"
#include "player.h"

class Game : public QObject {
  Q_OBJECT

 public:
  CardVec blind_{32};
  CardVec trick_{3};
  CardVec skat_{2};
  CardVec cardsInGame_{30};
  CardVec urSkat_{2};
  Player player_1{1, "Player-1", false};  // isRobot = false
  Player player_2{2, "Player-2", true};
  Player player_3{3, "Player-3", true};
  Player* sager{};
  Player* hoerer{};
  std::vector<Player*> playerList_{&player_1, &player_2, &player_3};
  // geberHoererSagerPos_[0][1][2] == player.id
  std::vector<int> geberHoererSagerPos_{1, 2, 3};
  // first rotation before 1st round starts -> first round {2, 3, 1}
  int soloSpieler_id{};
  Rule rule_{};
  std::string trump_{};
  int gereizt_{};
  int mitOhne_{};
  int reizwert_{};
  int spielwertGereizt_{};
  int spielwertGespielt_{};
  bool gedrueckt_{};
  bool hand_{};
  bool ouvert_{};
  bool schneiderAngesagt_{};
  bool schneider_{};
  bool schwarzAngesagt_{};
  bool schwarz_{};

  Card trickCardFirst_{};
  Card trickCardStrongest_{};
  int pointsSolo_{};
  int pointsOpponents_{};
  Matrix matrix{};

  // constructor
  explicit Game(QObject* parent = nullptr);

  // public methods
  void init();
  void start();
  void geben();

  int reizen(Reizen reizen = Reizen::Normal);
  int reizwert(Player* player, const std::string& suit = "");
  void setMaxBieten();
  int counter(Reset reset = Reset::No);
  void bieten(Passen passen = Passen::Nein);
  // void roboAufheben();
  void roboDruecken(Player* player);
  void druecken();
  void setSpielwertGereizt();
  void setSpielwertGespielt();

  bool nullComparator(const Card& a, const Card& b);
  bool ramschComparator(const Card& a, const Card& b);
  bool grandComparator(const Card& a, const Card& b);
  bool suitComparator(const Card& a, const Card& b);
  void autoplay();
  void playCard(const Card& card);
  void activateNextPlayer();

  void finishRound();

  bool isNullOk(Player* player);
  bool isCardValid(const Card& card);
  bool isCardStronger(const Card& card);

  Player& getPlayerById(int id);
  Player& getPlayerByPos(int pos);
  Player* getPlayerByHasTrick();
  Player* getPlayerByIsSolo();
  Player* getPlayerByHighestBid();
  Player* getPlayerByMostTricksPoints();


 signals:
  void gegeben();
  void geboten(int idSager, int idHoerer, QString antwortSager,
               QString antwortHoerer);

  void frageHand();
  void ruleAndTrump(Rule rule, std::string trump);
  void roboGedrueckt();

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
