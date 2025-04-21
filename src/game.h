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
  CardVec urSkat_{2};
  CardVec skat_{2};
  CardVec trick_{3};
  CardVec cardsInGame_{32};
  CardVec othersCards_{22};
  // 22 if player not isSolo,
  // 20 if player isSolo (Solo knows his cards in skat)
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
  bool hand_{};
  bool schneider_{};
  bool schneiderAngesagt_{};
  bool schwarz_{};
  bool schwarzAngesagt_{};
  bool ouvert_{};

  bool gedrueckt_{};  // used in bieten
  int gereizt_{};
  int spielwert_{};
  int spielwertFinishRound_{};

  bool kontra_{};
  bool re_{};
  bool bock_{};

  // Fields for Decisions:
  Card trickCardFirst_{};
  Card trickCardStrongest_{};

  Card cardBySolo_{};
  bool hasSoloPlayed_{};
  bool hasSoloTrick_{};

  Matrix matrix_{};

  // constructor
  explicit Game(QObject* parent = nullptr);

  // public methods
  void init();
  void start();
  void geben();

  int reizen(Reizen reizen = Reizen::Normal);
  int spielwert(Player* player, Spielwert wert);
  void setMaxBieten();
  int counter(Reset reset = Reset::No);
  void bieten(Passen passen = Passen::Nein);
  void roboDruecken(Player* player);
  void druecken();

  void evaluateOthersCards();
  Card& cardByNull_KI();
  Card& cardByGrand_KI();
  Card& cardBySuit_KI();
  Card& cardByRamsch_KI();

  void autoplay();
  void playCard(const Card& card);
  void activateNextPlayer();

  void finishRound();

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
  void ruleAndTrump(Rule rule, std::string trump, bool hand = false,
                    bool schneiderAngesagt = false,
                    bool schwarzAngesagt = false, bool ouvert = false);
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
