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
  validCards_.reserve(10);
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

void CardVec::clone(
    CardVec other) {
  cards_ = other.cards_;
  isCardFaceVisible_ = other.isCardFaceVisible_;
  cardFace_ = other.cardFace_;
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

// std::vector<Card>& CardVec::valids() { return valids_; }

int CardVec::size() const { return cards_.size(); }

void CardVec::shuffle() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::ranges::shuffle(cards_, gen);
}

bool CardVec::isCardInside(
    const Card& card) {
  return std::find(cards().begin(), cards().end(), card) != cards().end();
}

bool CardVec::isRankInside(
    const std::string& rank) {
  return std::any_of(cards().begin(), cards().end(),
                     [&](const Card& card) { return card.rank() == rank; });
}

bool CardVec::isSuitInside(
    const std::string& suit) {
  return std::any_of(cards().begin(), cards().end(),
                     [&](const Card& card) { return card.suit() == suit; });
}

Card& CardVec::getCardWithRank(
    const std::string& rank) {
  for (Card& card : cards_) {
    if (card.rank() == rank) return card;
  }
  throw std::runtime_error("Card with given rank not found");
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

void CardVec::erase(
    Card card) {
  cards_.erase(std::remove_if(cards_.begin(), cards_.end(),
                              [&](const Card& c) { return c == card; }),
               cards_.end());
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

  sortCardsFor(rule, targetSuit);

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
    Rule rule, const std::string& trump) {
  // pattern(4) Rule::Grand:  J J J J => 1 1 1 1
  // pattern(11) Rule::Suit:  J J J J A K Q 10 9 8 7 => 1 1 1 1 1 1 1 1 1 1 1
  std::vector<int> pattern = toPattern(rule, trump);

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

void CardVec::sortCardsFor(
    Rule rule, const std::string& trumpSuit) {
  std::vector<std::string> suits = {"♣", "♥", "♠", "♦"};

  std::unordered_map<std::string, int> suitPriority;
  for (size_t i = 0; i < suits.size(); ++i) {
    suitPriority[suits[i]] = i + 1;
  }

  if (rule == Rule::Null) {
    std::ranges::sort(cards_, [&](const Card& a, const Card& b) {
      return std::tuple{suitPriority.at(a.suit()),
                        SortPriorityNull.at(a.rank())} <
             std::tuple{suitPriority.at(b.suit()),
                        SortPriorityNull.at(b.rank())};
    });
    return;
  }

  if (rule == Rule::Suit) {
    while (suits[0] != trumpSuit) {
      std::rotate(suits.begin(), suits.begin() + 1, suits.end());
    }
    for (size_t i = 0; i < suits.size(); ++i) {
      suitPriority[suits[i]] = i + 1;
    }
  }

  // - Then by rank for non-Jacks
  std::ranges::sort(cards_, [&](const Card& a, const Card& b) {
    return std::tuple{a.rank() == "J" ? 0 : 1,
                      a.rank() == "J" ? SortPriorityJacks.at(a.suit())
                                      : suitPriority.at(a.suit()),
                      a.rank() == "J" ? 0 : SortPriorityRanks.at(a.rank())} <
           std::tuple{b.rank() == "J" ? 0 : 1,
                      b.rank() == "J" ? SortPriorityJacks.at(b.suit())
                                      : suitPriority.at(b.suit()),
                      b.rank() == "J" ? 0 : SortPriorityRanks.at(b.rank())};
  });
}

void CardVec::sortCardsByPower(
    std::vector<Card>& cards, Rule rule, const std::string& trumpSuit,
    Order order) {
  auto filter = [&](const Card& a, const Card& b) {
    int powerA = a.power(rule, trumpSuit);
    int powerB = b.power(rule, trumpSuit);
    return order == Order::Increase ? powerA < powerB : powerA > powerB;
  };

  std::ranges::sort(cards, filter);
}

void CardVec::sortCardsByValue(
    std::vector<Card>& cards, Order order) {
  auto filter = [&](const Card& a, const Card& b) {
    int valueA = a.value();
    int valueB = b.value();

    return order == Order::Increase ? valueA <= valueB : valueA > valueB;
  };

  std::ranges::sort(cards, filter);
}

void CardVec::sortCardsByPowerPriorityRule(
    std::vector<Card>& cards, Rule rule, Order order) {
  auto filter = [&](const Card& a, const Card& b) {
    const auto& rankMap =
        (rule == Rule::Null) ? PowerPriorityNull : PowerPriorityRanks;

    int rankA = rankMap.at(a.rank());
    int rankB = rankMap.at(b.rank());

    return order == Order::Increase ? rankA < rankB : rankA > rankB;
  };

  std::ranges::sort(cards, filter);
}

/*
 *    cards_ => validCards
 */
void CardVec::setValidCards(
    Rule rule, const std::string& trumpSuit, const Card& trickCardFirst,
    const Card& trickCardStrongest, Order order) {
  qDebug() << "CardVec::setValidCards...";
  validCards_ = {};

  if (trickCardFirst.isEmpty()) {
    // If trick is empty, all cards are valid
    validCards_ = cards_;
    // sortCardsByPower(validCards_, rule, trumpSuit, order);
    printContainer(validCards_);
    setPowerCards(rule, trumpSuit, trickCardStrongest, order);
    setRankCards(rule, trickCardStrongest, order);
    setValueCards(trickCardStrongest, order);
    return;
  }

  std::string requiredSuit = trickCardFirst.suit();

  // ---- Rule: Suit (Trump Game) ----
  if (rule == Rule::Suit) {
    if (trickCardFirst.rank() == "J") requiredSuit = trumpSuit;

    bool hasTrump = std::ranges::any_of(cards_, [&](const Card& c) {
      return c.rank() == "J" || c.suit() == trumpSuit;
    });

    bool hasSuit = std::ranges::any_of(cards_, [&](const Card& c) {
      return c.rank() != "J" && c.suit() == requiredSuit;
    });

    // Filtering function
    auto isValidCard = [&](const Card& card) -> bool {
      if (hasTrump && requiredSuit == trumpSuit)
        return card.rank() == "J" || card.suit() == requiredSuit;

      if (hasSuit && requiredSuit != trumpSuit)
        return card.suit() == requiredSuit && card.rank() != "J";

      return false;
    };

    std::ranges::copy_if(cards_, std::back_inserter(validCards_), isValidCard);
  }

  // ---- Rule: Grand / Ramsch (Only Jacks are Trumps) ----
  else if (rule == Rule::Grand || rule == Rule::Ramsch) {
    if (trickCardFirst.rank() == "J") requiredSuit = "";

    bool hasJack = std::ranges::any_of(
        cards_, [](const Card& c) { return c.rank() == "J"; });

    bool hasSuit = std::ranges::any_of(cards_, [&](const Card& c) {
      return c.rank() != "J" && c.suit() == requiredSuit;
    });

    auto isValidCard = [&](const Card& card) -> bool {
      if (hasJack && trickCardFirst.rank() == "J") return card.rank() == "J";

      if (hasSuit) return card.suit() == requiredSuit && card.rank() != "J";

      return false;
    };

    // Apply filtering
    std::ranges::copy_if(cards_, std::back_inserter(validCards_), isValidCard);
  }

  // ---- Rule: Null (No Trumps, Must Follow Suit) ----
  else if (rule == Rule::Null) {
    bool hasSuit = std::ranges::any_of(
        cards_, [&](const Card& c) { return c.suit() == requiredSuit; });

    // Filtering function
    auto isValidCard = [&](const Card& card) -> bool {
      return hasSuit ? card.suit() == requiredSuit
                     : true;  // If hasSuit, follow suit, else play any card
    };

    // Apply filtering
    std::ranges::copy_if(cards_, std::back_inserter(validCards_), isValidCard);
  }

  // If no valid cards were found, allow any card to be played
  if (validCards_.empty()) validCards_ = cards_;

  // sortCardsByPower(validCards_, rule, trumpSuit, order);

  // sortCardsByPower(validCards_, rule, trumpSuit, order);
  printContainer(validCards_);

  setPowerCards(rule, trumpSuit, trickCardStrongest, order);
  setRankCards(rule, trickCardStrongest, order);
  setValueCards(trickCardStrongest, order);
}

void CardVec::setPowerCards(
    Rule rule, const std::string& trumpSuit, const Card& trickCardStrongest,
    Order order) {
  qDebug() << "setPowerCards...";

  lowestPowerCard_ = Card();
  highestPowerCard_ = Card();
  nextLowerPowerCard_ = Card();
  nextHigherPowerCard_ = Card();

  std::vector<Card> filteredCards;

  std::copy_if(validCards_.begin(), validCards_.end(),
               std::back_inserter(filteredCards), [&](const Card& card) {
                 return card.suit() == trickCardStrongest.suit() ||
                        card.suit() == trumpSuit && rule != Rule::Null &&
                            rule != Rule::Grand ||
                        card.rank() == "J" && rule != Rule::Null;
               });

  qDebug() << "validCards with Power:";
  printContainer(filteredCards);

  if (filteredCards.empty()) {
    qDebug() << "No power cards after filtering.";
    return;
  }

  sortCardsByPower(filteredCards, rule, trumpSuit, Order::Increase);

  qDebug() << "sorted by Power:";
  printContainer(filteredCards);

  // Sort by power

  lowestPowerCard_ = filteredCards.front();
  qDebug() << "lowestPowerCard_"
           << QString::fromStdString(lowestPowerCard_.str());

  highestPowerCard_ = filteredCards.back();
  qDebug() << "highestPowerCard_"
           << QString::fromStdString(highestPowerCard_.str());

  if (trickCardStrongest != Card()) {
    int reference = trickCardStrongest.power(rule, trumpSuit);

    nextLowerPowerCard_ = {};
    nextHigherPowerCard_ = {};

    // Find the next lower card by power
    for (const auto& card : filteredCards) {
      int cardPower = card.power(rule, trumpSuit);

      if (cardPower < reference) {
        nextLowerPowerCard_ = card;
      } else {
        break;
      }
    }

    qDebug() << "nextLowerPowerCard_"
             << QString::fromStdString(nextLowerPowerCard_.str());

    sortCardsByPower(filteredCards, rule, trumpSuit, Order::Decrease);

    // Find the next higher card by power
    for (const auto& card : filteredCards) {
      int cardPower = card.power(rule, trumpSuit);

      if (cardPower > reference) {
        nextHigherPowerCard_ = card;
      } else {
        break;
      }
    }

    qDebug() << "nextHigherPowerCard_"
             << QString::fromStdString(nextHigherPowerCard_.str());
  }
}

void CardVec::setRankCards(
    Rule rule, const Card& trickCardStrongest, Order order) {
  qDebug() << "setRankCards...";

  lowestRankCard_ = Card();
  highestRankCard_ = Card();
  nextLowerRankCard_ = Card();
  nextHigherRankCard_ = Card();

  sortCardsByPowerPriorityRule(validCards_, rule, Order::Increase);

  qDebug() << "sorted by Rank power:";
  printContainer(validCards_);

  lowestRankCard_ = validCards_.front();
  qDebug() << "lowestRankCard_"
           << QString::fromStdString(lowestRankCard_.str());

  highestRankCard_ = validCards_.back();
  qDebug() << "highestRankCard_"
           << QString::fromStdString(highestRankCard_.str());

  if (trickCardStrongest != Card()) {
    int reference = PowerPriorityRanks.at(trickCardStrongest.rank());

    nextLowerRankCard_ = {};
    nextHigherRankCard_ = {};

    for (const auto& card : validCards_) {
      int cardRank = PowerPriorityRanks.at(card.rank());

      if (cardRank < reference) {
        nextLowerRankCard_ = card;
      } else {
        // nextLowerRankCard_ = Card();
        break;
      }
    }
    qDebug() << "nextLowerRankCard_"
             << QString::fromStdString(nextLowerRankCard_.str());

    sortCardsByPowerPriorityRule(validCards_, rule, Order::Decrease);

    for (const auto& card : validCards_) {
      int cardRank = PowerPriorityRanks.at(card.rank());

      if (cardRank > reference) {
        nextHigherRankCard_ = card;
      } else {
        // nextHigherRankCard_ = Card();
        break;
      }
    }
    qDebug() << "nextHigherRankCard_"
             << QString::fromStdString(nextHigherRankCard_.str());
  }
}

void CardVec::setValueCards(
    const Card& trickCardStrongest, Order order) {
  qDebug() << "setValueCards...";

  lowestValueCard_ = Card();
  highestValueCard_ = Card();
  nextLowerValueCard_ = Card();
  nextHigherValueCard_ = Card();

  sortCardsByValue(validCards_, Order::Increase);
  printContainer(validCards_);

  lowestValueCard_ = validCards_.front();
  qDebug() << "lowestValueCard_"
           << QString::fromStdString(lowestValueCard_.str());

  highestValueCard_ = validCards_.back();
  qDebug() << "highestValueCard_"
           << QString::fromStdString(highestValueCard_.str());

  if (trickCardStrongest != Card()) {
    int reference = trickCardStrongest.value();

    nextLowerValueCard_ = {};
    nextHigherValueCard_ = {};

    for (const auto& card : validCards_) {
      int cardValue = card.value();

      if (cardValue < reference) {
        nextLowerValueCard_ = card;
      } else {
        // nextLowerValueCard_ = Card();
        break;
      }
    }
    qDebug() << "nextLowerValueCard_"
             << QString::fromStdString(nextLowerValueCard_.str());

    sortCardsByValue(validCards_, Order::Decrease);

    for (const auto& card : validCards_) {
      int cardValue = card.value();

      if (cardValue > reference) {
        nextHigherValueCard_ = card;
      } else {
        // nextHigherValueCard_ = Card();
        break;
      }
    }
    qDebug() << "nextHigherValueCard_"
             << QString::fromStdString(nextHigherValueCard_.str());
  }
}

int CardVec::points() {
  int sum{0};
  for (const Card& card : cards_) sum += card.value();
  return sum;
}

void CardVec::print() {
  std::string str;
  for (const Card& card : cards_) {
    str += card.str() + " ";
  }
  qDebug() << QString::fromStdString(str);
}
