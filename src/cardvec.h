#ifndef CARDVEC_H
#define CARDVEC_H

#include <utility>
#include <vector>

#include "card.h"

class CardVec {
 private:
  std::vector<Card> cards_;
  bool isCardFaceVisible_;
  CardFace cardFace_;

 public:
  std::vector<Card> validCards_{};

  Card lowestPowerCard_{};
  Card highestPowerCard_{};
  Card nextLowerPowerCard_{};
  Card nextHigherPowerCard_{};

  Card lowestRankCard_{};
  Card highestRankCard_{};
  Card nextLowerRankCard_{};
  Card nextHigherRankCard_{};

  Card lowestValueCard_{};
  Card highestValueCard_{};
  Card nextLowerValueCard_{};
  Card nextHigherValueCard_{};

  // Constructors & Destructor
  explicit CardVec(int length);
  ~CardVec() = default;

  // Copy Constructor
  CardVec(const CardVec& other);

  // Copy Assignment Operator
  CardVec& operator=(const CardVec& other);

  // Move Constructor
  CardVec(CardVec&& other) noexcept;

  // Move Assignment Operator
  CardVec& operator=(CardVec&& other) noexcept;

  // explicit CardVec(const std::vector<Card>& cards) : cards_(cards)  {}

  // Universal Forwarding of card
  template <typename T>
  void addCard(
      T&& card) {
    cards_.push_back(std::forward<T>(card));
  }

  // Public Methods
  std::vector<Card>& cards();
  // std::vector<Card>& valids();
  int size() const;
  void shuffle();
  bool isCardInside(const Card& card);
  void moveCardTo(const Card&, CardVec& targetVec);
  void moveTopCardTo(CardVec& targetVec);
  void erase(Card card);  // used in cardsInGame_

  std::vector<Card> filterJacks();
  std::vector<Card> filterSuits(const std::string& targetSuit, Rule rule);
  std::vector<Card> filterJacksAndSuits(const std::string& targetSuit);

  // e.g. J J J J   A 10 K Q 9 8 7 =>
  //      1 0 0 1   1  0 0 1 1 0 0
  std::vector<int> toPattern(Rule rule, const std::string& targetSuit);

  // e.g. J J J J
  //      0 1 1 0 => -1
  int spitzen(const std::string& trump);

  // e.g. J J J J   A 10 K Q 9 8 7 =>
  //      1 0 0 1   1  0 0 1 1 0 0
  int sumTrump(const std::string& trump);

  // e.g.  {"J", 1}, {"♣", 3}, {"♠", 3}, {"♥", 1}, {"♦", 2}};
  std::map<std::string, int> mapCards(Rule rule);

  // e.g. {"J", 4} or {"♣", 5}
  std::pair<std::string, int> mostPairInMap(
      const std::map<std::string, int>& cardMap);

  std::pair<std::string, int> fewestPairInMap(
      const std::map<std::string, int>& cardMap);

  void sortCardsFor(Rule rule, const std::string& trumpSuit);

  void setValidCards(Rule rule, const std::string& trumpSuit,
                     const Card& trickCardFirst, const Card& trickCardStrongest,
                     Order order = Order::Increase);

  void sortCardsByPower(std::vector<Card>& cards, Rule rule,
                        const std::string& trumpSuit, Order order);

  void sortCardsByPowerPriorityRule(std::vector<Card>& cards, Rule rule,
                                    Order order);

  void sortCardsByValue(std::vector<Card>& cards, Order order);

  void setPowerCards(Rule rule, const std::string& trumpSuit,
                     const Card& trickCardStrongest, Order order);

  void setRankCards(Rule rule, const Card& trickCardStrongest, Order order);

  void setValueCards(const Card& trickCardStrongest, Order order);

  int points();
  void print();
};

#endif  // CARDVEC_H
