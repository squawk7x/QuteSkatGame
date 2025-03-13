#ifndef CARDVEC_H
#define CARDVEC_H

#include <utility>
#include <vector>

#include "card.h"

enum class CardFace { Open, Closed };

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

  // Perfect Forwarding of card
  template <typename T>
  void addCard(
      T&& card) {
    cards_.push_back(std::forward<T>(card));
  }

  // void addCard(
  //     Card&& card) {
  //   // Add the card to the vector using move semantics
  //   cards_.push_back(std::move(card));
  //   qDebug() << "Card added to target vector:"
  //            << QString::fromStdString(card.str());
  // }

  // Public Methods
  std::vector<Card>& cards();
  void shuffle();
  bool isCardInside(const Card& card);
  void moveCardTo(Card&&, CardVec& targetVec);
  // void moveCardTo(Card&, CardVec& targetVec);
  // void moveCardTo(const Card& card, CardVec& targetVec);
  void moveTopCardTo(CardVec& targetVec);
  void sortByJandSuits();
  auto filterJplusSuit(const std::string& targetSuit);
  std::string pattern(const std::string& targetSuit);
  int mitOhne(const std::string& targetSuit);
  int value();
  const QString print() const;
};

#endif  // CARDVEC_H
