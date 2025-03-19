#ifndef CARDVEC_H
#define CARDVEC_H

#include <bitset>
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
  // CardVec() : isCardFaceVisible_(false), cardFace_(CardFace::Closed) {}
  explicit CardVec(
      int length)
      : isCardFaceVisible_(false), cardFace_(CardFace::Closed) {
    cards_.reserve(length);
  }
  ~CardVec() = default;

  // Copy Constructor
  CardVec(
      const CardVec& other)
      : cards_(other.cards_),
        isCardFaceVisible_(other.isCardFaceVisible_),
        cardFace_(other.cardFace_) {}

  // Copy Assignment Operator
  CardVec& operator=(
      const CardVec& other) {
    if (this != &other) {
      cards_ = other.cards_;
      isCardFaceVisible_ = other.isCardFaceVisible_;
      cardFace_ = other.cardFace_;
    }
    return *this;
  }

  // Move Constructor
  CardVec(
      CardVec&& other) noexcept
      : cards_(std::move(other.cards_)),
        isCardFaceVisible_(other.isCardFaceVisible_),
        cardFace_(other.cardFace_) {}

  // Move Assignment Operator
  CardVec& operator=(
      CardVec&& other) noexcept {
    if (this != &other) {
      cards_ = std::move(other.cards_);
      isCardFaceVisible_ = other.isCardFaceVisible_;
      cardFace_ = other.cardFace_;
    }
    return *this;
  }

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
  std::vector<int> trumpPattern(const std::string& targetSuit);
  int countTrump(const std::string& targetSuit);
  int mitOhne(const std::string& targetSuit);

  void sortJacksSuits();

  int value();

  // Helperfunctions
  const QString print() const;
  const QString printRange(std::vector<Card> rng) const;
  std::string patternToString(const std::vector<int>& vec);
};

#endif  // CARDVEC_H
