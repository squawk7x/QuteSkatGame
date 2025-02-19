#include "cardvec.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <unordered_map>

CardVec::CardVec(
    int length)
    : cards_{}, isCardFaceVisible_{false}, cardFace_{CardFace::Closed} {
  cards_.reserve(length);
}

CardVec::~CardVec() { clearCards(); }

// cardVec.h:
// template <typename T>
// void addCard(
//     T&& card) {
//   cards_.push_back(std::forward<T>(card));
// }

void CardVec::moveCardTo(
    const Card& card, CardVec& target) {
  auto it = std::ranges::find(cards_, card);  // Find the card
  if (it != cards_.end()) {
    target.addCard(std::move(*it));     // Move to target
    std::ranges::remove(cards_, card);  // Remove from source
  }
}

void CardVec::moveTopCardTo(
    CardVec& targetVec) {
  if (!cards_.empty()) {
    targetVec.addCard(
        std::move(cards_.back()));  // Move it into the target vector
    cards_.pop_back();              // Remove it from the original vector
  }
}

// Shuffle the cards
void CardVec::shuffle() {
  std::random_device rd;
  std::mt19937 gen(rd());  // New RNG instance with fresh seed
  std::ranges::shuffle(cards_, gen);
}

void CardVec::sortCardsByPattern() {
  static const std::vector<std::string> suit_order = {"♣", "♥", "♠", "♦"};
  static const std::vector<std::string> rank_order = {"J", "A", "10", "K",
                                                      "Q", "9", "8",  "7"};

  // Suit priority for general sorting
  static const std::unordered_map<std::string, int> suit_priority = {
      {"♣", 0}, {"♥", 1}, {"♠", 2}, {"♦", 3}};

  // Special suit priority for Jokers (J)
  static const std::unordered_map<std::string, int> suit_priority_for_J = {
      {"♣", 0}, {"♠", 1}, {"♥", 2}, {"♦", 3}};

  // Rank priority for sorting within a suit
  static const std::unordered_map<std::string, int> rank_priority = {
      {"J", 0}, {"A", 1}, {"10", 2}, {"K", 3},
      {"Q", 4}, {"9", 5}, {"8", 6},  {"7", 7}};

  // Step 1: Sort normally by suit and rank
  std::ranges::sort(cards_, [&](const Card& a, const Card& b) {
    if (suit_priority.at(a.suit()) != suit_priority.at(b.suit())) {
      return suit_priority.at(a.suit()) < suit_priority.at(b.suit());
    }
    return rank_priority.at(a.rank()) < rank_priority.at(b.rank());
  });

  // Step 2: Move all 'J' cards to the front, sorting by suit using
  // suit_priority_for_J
  std::stable_partition(cards_.begin(), cards_.end(),
                        [](const Card& card) { return card.rank() == "J"; });

  // Step 3: Sort the moved 'J' cards by their special suit priority
  auto j_end = std::find_if(cards_.begin(), cards_.end(), [](const Card& card) {
    return card.rank() != "J";
  });

  std::ranges::sort(cards_.begin(), j_end, [&](const Card& a, const Card& b) {
    return suit_priority_for_J.at(a.suit()) < suit_priority_for_J.at(b.suit());
  });
}

std::string CardVec::print() {
  std::string str;
  str.reserve(cards_.size() *
              3);  // Optimize memory allocation (assuming ~3 bytes per card)

  for (const Card& card : cards_) {
    str += card.str() + " ";  // Add a space for readability
  }

  return str;
}

void CardVec::clearCards() { cards_.clear(); }


std::vector<Card>& CardVec::cards() { return cards_; }

