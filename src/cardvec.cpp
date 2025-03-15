#include "cardvec.h"

#include <QDebug>
#include <algorithm>
#include <random>
#include <ranges>
#include <unordered_map>

#include "definitions.h"

// CardVec::CardVec(
//     int length)
//     : cards_{}, isCardFaceVisible_{false}, cardFace_{CardFace::Closed} {
//   cards_.reserve(length);
// }

// CardVec::~CardVec() {}

// public class methods
std::vector<Card>& CardVec::cards() { return cards_; }

void CardVec::shuffle() {
  std::random_device rd;
  std::mt19937 gen(rd());  // New RNG instance with fresh seed
  std::ranges::shuffle(cards_, gen);
}

bool CardVec::isCardInside(
    const Card& card) {
  if (std::find(cards().begin(), cards().end(), card) < cards().end()) {
    return true;
  }
  return false;
}

void CardVec::moveCardTo(
    const Card& card, CardVec& targetVec) {
  auto it = std::ranges::find(cards_, card);
  if (it != cards_.end()) {
    // Move the card first, then erase it safely
    Card movedCard = std::move(*it);          // Move to a temporary variable
    targetVec.addCard(std::move(movedCard));  // Move it to targetVec

    cards_.erase(it);  // Now safely remove the moved-from card
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

void CardVec::sortByJandSuits() {
  // Step 1: Sort normally by suit and rank
  std::ranges::sort(cards_, [&](const Card& a, const Card& b) {
    if (SortingPriorityForSuit.at(a.suit()) !=
        SortingPriorityForSuit.at(b.suit())) {
      return SortingPriorityForSuit.at(a.suit()) <
             SortingPriorityForSuit.at(b.suit());
    }
    return SortingPriorityForRank.at(a.rank()) <
           SortingPriorityForRank.at(b.rank());
  });

  // Step 2: Move all 'J' cards to the front, sorting by suit using
  // SortingPriorityFor_J
  std::stable_partition(cards_.begin(), cards_.end(),
                        [](const Card& card) { return card.rank() == "J"; });

  // Step 3: Sort the moved 'J' cards by their special suit priority
  auto j_end = std::find_if(cards_.begin(), cards_.end(), [](const Card& card) {
    return card.rank() != "J";
  });

  std::ranges::sort(cards_.begin(), j_end, [&](const Card& a, const Card& b) {
    return SortingPriorityFor_J.at(a.suit()) <
           SortingPriorityFor_J.at(b.suit());
  });
}

auto CardVec::filterJplusSuit(
    const std::string& targetSuit) {
  // Step 1: Filter all 'J' cards
  auto j_cards = cards_ | std::ranges::views::filter([](const Card& card) {
                   return card.rank() == "J";
                 });

  // Step 2: Filter cards of the specific suit (excluding 'J' cards)
  auto suit_cards =
      cards_ | std::ranges::views::filter([&targetSuit](const Card& card) {
        return card.suit() == targetSuit && card.rank() != "J";
      });

  // Combine both parts into a new sequence (first all 'J' cards, then all suit
  // cards)
  std::vector<Card> result;
  result.insert(result.end(), j_cards.begin(), j_cards.end());
  result.insert(result.end(), suit_cards.begin(), suit_cards.end());

  return result;
}

std::string CardVec::pattern(
    const std::string& targetSuit) {
  // Initialize the pattern to empty string
  std::string pat = "";

  // Step 1: Sort cards and filter Jacks and the specific suit (done by
  // sortByJandSuits)
  sortByJandSuits();
  auto JplusSuits = filterJplusSuit(targetSuit);

  // Step 2: Store the presence of Jacks for each suit
  std::unordered_map<std::string, int> jackPresence;
  for (const auto& card : JplusSuits) {
    if (card.rank() == "J") {
      jackPresence[card.suit()] = 1;
    }
  }

  // Step 3: Store the presence of other ranks for the specific suit
  std::unordered_map<std::string, int> rankPresence;
  for (const auto& card : JplusSuits) {
    if (card.rank() != "J" && card.suit() == targetSuit) {
      rankPresence[card.rank()] = 1;
    }
  }

  // Step 4: Append '1' or '0' to the pattern for Jacks (♣, ♠, ♥, ♦)
  pat += jackPresence["♣"] == 1 ? '1' : '0';
  pat += jackPresence["♠"] == 1 ? '1' : '0';
  pat += jackPresence["♥"] == 1 ? '1' : '0';
  pat += jackPresence["♦"] == 1 ? '1' : '0';

  // Step 5: Append '1' or '0' for ranks (A, 10, K, Q, 9, 8, 7)
  pat += rankPresence["A"] == 1 ? '1' : '0';
  pat += rankPresence["10"] == 1 ? '1' : '0';
  pat += rankPresence["K"] == 1 ? '1' : '0';
  pat += rankPresence["Q"] == 1 ? '1' : '0';
  pat += rankPresence["9"] == 1 ? '1' : '0';
  pat += rankPresence["8"] == 1 ? '1' : '0';
  pat += rankPresence["7"] == 1 ? '1' : '0';

  // Output the pattern using qDebug()
  qDebug() << QString::fromStdString(pat);

  return pat;
}

int CardVec::mitOhne(
    const std::string& targetSuit) {
  // Get the pattern for the target suit
  std::string pat = pattern(targetSuit);

  // Count leading '0's using ranges
  int count0 = std::ranges::distance(
      pat | std::ranges::views::take_while([](char c) { return c == '0'; }));

  // Count leading '1's using ranges
  int count1 = std::ranges::distance(
      pat | std::ranges::views::take_while([](char c) { return c == '1'; }));

  // Return the maximum of leading '0's and leading '1's
  qDebug() << "mitOhne " << std::max(count0, count1);
  return std::max(count0, count1);
}

int CardVec::value() {
  int sum{0};
  for (const Card& card : cards_) sum += card.value();
  return sum;
}

const QString CardVec::print() const {
  QString str;
  str.reserve(cards_.size() *
              3);  // Optimize memory allocation (assuming ~3 bytes per card)

  for (const Card& card : cards_) {
    str += QString::fromStdString(card.str()) +
           " ";  // Add a space for readability
  }

  return str;
}
