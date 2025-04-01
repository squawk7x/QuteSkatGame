#ifndef CARDVEC_H
#define CARDVEC_H

#include <utility>
#include <vector>

#include "card.h"

class CardVec {
 private:
  std::vector<Card> cards_;
  // std::vector<Card> valids_;
  bool isCardFaceVisible_;
  CardFace cardFace_;

 public:
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
  void sortCardsByPower(Rule rule, const std::string& trumpSuit, Order order);

  std::vector<Card> validCards(Rule rule, const std::string& trumpSuit,
                               const Card& trickFirstCard);

  // Card& cardPowerJustBelow(Rule rule, const std::string& trumpSuit, const
  // Card& refCard); Card& cardPowerJustAbove(Rule rule, const Card& card);

  int points();
  void print();
};

#endif  // CARDVEC_H
