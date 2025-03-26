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
  void shuffle();
  bool isCardInside(const Card& card);
  void moveCardTo(const Card&, CardVec& targetVec);
  void moveTopCardTo(CardVec& targetVec);

  std::vector<Card> filterJacks();
  std::vector<Card> filterSuits(const std::string& targetSuit);
  std::vector<Card> filterJacksSuits(const std::string& targetSuit);
  // e.g. J J J J A 10 K Q 9 8 7 =>
  //      1 0 0 1 1  0 0 1 1 0 0
  std::vector<int> patternJacksAndSuit(const std::string& targetSuit);
  // e.g. 5
  int sumPatternJacksAndSuit(const std::string& targetSuit);
  // e.g. 5
  int countPatternJacksAndSuit(const std::string& targetSuit);
  // e.g.  {"J", 1}, {"♣", 3}, {"♠", 3}, {"♥", 1}, {"♦", 2}};
  std::map<std::string, int> JandSuitNumMap();
  // e.g. {"J", 4} or {"♣", 5}
  std::pair<std::string, int> highestPairInMap(
      const std::map<std::string, int>& suitMap);
  // e.g. J J J J
  //      0 1 1 0 => -1
  int mitOhne(const std::string& trump);

  void sortByJacksAndSuits();

  int value();
};

#endif  // CARDVEC_H
