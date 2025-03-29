#include "cardvec.h"

#include <QDebug>
#include <algorithm>
#include <random>
#include <ranges>
#include <unordered_map>

#include "definitions.h"
#include "helperFunctions.h"

// Constructor
CardVec::CardVec(
    int length)
    : isCardFaceVisible_(false), cardFace_(CardFace::Closed) {
  cards_.reserve(length);
}

// Copy Constructor
CardVec::CardVec(
    const CardVec& other)
    : cards_(other.cards_),
      isCardFaceVisible_(other.isCardFaceVisible_),
      cardFace_(other.cardFace_) {}

// Copy Assignment Operator
CardVec& CardVec::operator=(
    const CardVec& other) {
  if (this != &other) {
    cards_ = other.cards_;
    isCardFaceVisible_ = other.isCardFaceVisible_;
    cardFace_ = other.cardFace_;
  }
  return *this;
}

// Move Constructor
CardVec::CardVec(
    CardVec&& other) noexcept
    : cards_(std::move(other.cards_)),
      isCardFaceVisible_(other.isCardFaceVisible_),
      cardFace_(other.cardFace_) {}

// Move Assignment Operator
CardVec& CardVec::operator=(
    CardVec&& other) noexcept {
  if (this != &other) {
    cards_ = std::move(other.cards_);
    isCardFaceVisible_ = other.isCardFaceVisible_;
    cardFace_ = other.cardFace_;
  }
  return *this;
}

// public class methods
std::vector<Card>& CardVec::cards() { return cards_; }

int CardVec::size() const { return cards_.size(); }

void CardVec::shuffle() {
  std::random_device rd;
  std::mt19937 gen(rd());
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

std::vector<Card> CardVec::filterJacks() {
  auto jacks = cards_ | std::ranges::views::filter([](const Card& card) {
                 return card.rank() == "J";
               });

  std::vector<Card> result;
  result.insert(result.end(), jacks.begin(), jacks.end());

  return result;
}

std::vector<Card> CardVec::filterSuits(
    const std::string& targetSuit, Rule rule) {
  std::vector<Card> result;

  // Apply different filtering conditions based on the rule
  if (rule != Rule::Null) {
    auto filtered =
        cards_ | std::ranges::views::filter([&targetSuit](const Card& card) {
          return card.suit() == targetSuit && card.rank() != "J";
        });

    // Sortiere nach card.rank() (aufsteigend)
    result.insert(result.end(), filtered.begin(), filtered.end());

    // Sortiere nach SortPriorityNull
    std::ranges::sort(result, [](const Card& a, const Card& b) {
      return SortPriorityNull.at(a.rank()) < SortPriorityNull.at(b.rank());
    });

  } else if (rule != Rule::Suit) {
    auto filtered =
        cards_ | std::ranges::views::filter([&targetSuit](const Card& card) {
          return card.suit() == targetSuit;
        });

    std::ranges::sort(result, [](const Card& a, const Card& b) {
      return a.rank() > b.rank();
    });
    // Sortiere nach card.value() (aufsteigend)
    std::ranges::sort(result, [](const Card& a, const Card& b) {
      return a.value() > b.value();
    });

    result.insert(result.end(), filtered.begin(), filtered.end());
  }

  return result;
}

std::vector<Card> CardVec::filterJacksAndSuits(
    const std::string& targetSuit) {
  std::vector<Card> result;

  // Store filtered results before inserting
  std::vector<Card> jacks = filterJacks();
  std::vector<Card> suits = filterSuits(targetSuit, Rule::Suit);

  result.insert(result.end(), jacks.begin(), jacks.end());
  result.insert(result.end(), suits.begin(), suits.end());

  return result;
}

std::vector<int> CardVec::toPattern(
    Rule rule, const std::string& targetSuit) {
  std::vector<int> pattern(11, 0);
  int i = 0;

  if (rule == Rule::Null) {
    for (const std::string& rank : {"A", "K", "Q", "J", "10", "9", "8", "7"}) {
      for (const Card& card : cards_)
        if (isCardInside(Card(targetSuit, rank))) {
          pattern[i] = 1;
        }
      i++;
    }
    return {pattern.begin(), pattern.begin() + 8};
  }

  // Grand and Suit
  for (const std::string& suit : {"♣", "♠", "♥", "♦"}) {
    for (const Card& card : cards_)
      if (isCardInside(Card(suit, "J"))) {
        pattern[i] = 1;
      }
    i++;
  }

  if (rule == Rule::Grand) {
    return {pattern.begin(), pattern.begin() + 4};
  }

  for (const std::string& rank : {"A", "10", "K", "Q", "9", "8", "7"}) {
    for (const Card& card : cards_)
      if (isCardInside(Card(targetSuit, rank))) {
        pattern[i] = 1;
      }
    i++;
  }

  qDebug() << QString::fromStdString(cardsToString(cards_));
  printContainer(pattern);
  return pattern;
}

// TODO calc separatly for suit and Grand
int CardVec::spitzen(
    const std::string& trump) {
  std::vector<int> pattern = toPattern(Rule::Suit, trump);

  auto mit = pattern | std::ranges::views::take_while(
                           [](int value) { return value != 0; });
  int count = std::ranges::count(mit, 1);  // mit => plus

  if (count == 0) {
    auto ohne = pattern | std::ranges::views::take_while(
                              [](int value) { return value != 1; });
    count = -std::ranges::count(ohne, 0);  // ohne => minus
  }

  qDebug() << "spitzen:" << count;

  return count;
}

int CardVec::sumTrump(
    const std::string& trump) {
  return std::ranges::fold_left(toPattern(Rule::Suit, trump), 0, std::plus<>());
}

std::map<std::string, int> CardVec::mapCards(
    Rule rule) {
  std::map<std::string, int> mapCards;
  // Suit or Null
  if (rule == Rule::Suit || rule == Rule::Null) {
    mapCards = {{"♣", 0}, {"♠", 0}, {"♥", 0}, {"♦", 0}};
    // Grand or Ramsch
  } else {
    mapCards = {{"J", 0}, {"♣", 0}, {"♠", 0}, {"♥", 0}, {"♦", 0}};
  }

  for (const Card& card : cards_) {
    if (rule == Rule::Null) {
      mapCards[card.suit()]++;
    }
    if (rule == Rule::Suit) {
      if (card.rank() == "J")
        continue;
      else
        mapCards[card.suit()]++;
    } else {
      if (card.rank() == "J") {
        mapCards["J"]++;
      } else {
        mapCards[card.suit()]++;
      }
    }
  }
  // printMap(mapCards);
  return mapCards;
}

std::pair<std::string, int> CardVec::mostPairInMap(
    const std::map<std::string, int>& cardMap) {
  if (cardMap.empty()) return {"", 0};

  auto most = std::ranges::max_element(
      cardMap, {}, &std::pair<const std::string, int>::second);

  return *most;
}

std::pair<std::string, int> CardVec::fewestPairInMap(
    const std::map<std::string, int>& cardMap) {
  if (cardMap.empty()) return {"", 0};

  auto fewest = std::ranges::min_element(
      cardMap, {}, &std::pair<const std::string, int>::second);

  return *fewest;
}

void CardVec::sortByRanks() {
  std::ranges::sort(cards_, [](const Card& a, const Card& b) {
    return std::tuple{
               a.rank() == "J" ? 0 : 1,  // Jacks first (0), others second (1)
               a.rank() == "J" ? SortPriorityJacks.at(a.suit())
                               : SortPrioritySuits.at(a.suit()),  // Suit order
               a.rank() == "J" ? 0 : SortPriorityRanks.at(a.rank())} <
           std::tuple{b.rank() == "J" ? 0 : 1,
                      b.rank() == "J" ? SortPriorityJacks.at(b.suit())
                                      : SortPrioritySuits.at(b.suit()),
                      b.rank() == "J" ? 0 : SortPriorityRanks.at(b.rank())};
  });
}

void CardVec::sortForNull() {
  std::ranges::sort(cards_, [](const Card& a, const Card& b) {
    return std::tuple{
               SortPrioritySuits.at(a.suit()),  // Suit order
               SortPriorityNull.at(a.rank())    // Rank order (Null priority)
           } < std::tuple{SortPrioritySuits.at(b.suit()),
                          SortPriorityNull.at(b.rank())};
  });
}

int CardVec::value() {
  int sum{0};
  for (const Card& card : cards_) sum += card.value();
  return sum;
}
