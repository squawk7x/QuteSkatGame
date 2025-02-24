#ifndef CARDVEC_H
#define CARDVEC_H

#include <vector>

#include "card.h"

enum class CardFace { Open, Closed };

class CardVec {
 private:
  std::vector<Card> cards_;
  bool isCardFaceVisible_;
  CardFace cardFace_;

 public:
  explicit CardVec(int length);
  ~CardVec();

  // Perfect Forwarding of card
  template <typename T>
  void addCard(
      T&& card) {
    cards_.push_back(std::forward<T>(card));
  }

  // public methods
  std::vector<Card>& cards();
  void shuffle();
  void moveCardTo(const Card& card, CardVec& targetVec);
  void moveTopCardTo(CardVec& targetVec);
  void moveCardVecTo(std::vector<CardVec>& targetVec);
  void sortByJandSuits();
  std::string pattern(const std::string& targetSuit);
  auto filterJplusSuit(const std::string& targetSuit);
  int mitOhne(const std::string& targetSuit);
  std::string print();
  int value();

  // virtual void removeCard(Card card);
  // virtual std::string cardsAsString() const;
  // void copyCardTo(const Card& card, CardVec* targetVec);
  // void copyTopCardTo(CardVec* targetVec);
  // void moveCardTo(Card card, CardVec* targetVec);
  // Card topCard();
  // bool isCardInCards(const Card& card);
  // bool isSuitInCards(const std::string& suit);
  // bool isRankInCards(const std::string& rank);
  // void sortByJandSuits(const std::vector<std::string>& pattern);
  // int countCardsOfRank(const std::string& rank) const;
  // std::string mostCommonSuit() const;
  // std::string suitOfRankWithMostPoints() const;
  // virtual void updateLayout();

  // // Getters
  // std::vector<Card>&

  // protected:
  // virtual void onCardClicked(const Card card);
  // virtual void onToggleCardsVisible(bool isVisible);
};

#endif  // CARDVEC_H
