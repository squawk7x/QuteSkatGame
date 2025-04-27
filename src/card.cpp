#include "card.h"

#include <iostream>
#include <memory>

#include "definitions.h"

Card::Card() : suit_(""), rank_("") {}

// Constructor: Card("♥", "Q")

/**
 * @brief Card::Card primary constructor e.g.: Card("♥", "Q")
 * @param suit any of these suits: {"♣", "♥", "♠", "♦"}
 * @param rank any of these ranks: {"J", "A", "10", "K", "Q", "9", "8",  "7"}
 */
Card::Card(
    const std::string& suit, const std::string& rank)
    : suit_{suit}, rank_{rank} {
  if (std::ranges::find(suits, suit_) != suits.end() &&
      std::ranges::find(ranks, rank_) != ranks.end()) {
    initCard();
  } else {
    std::cerr << "Invalid card: " << suit_ << rank_ << std::endl;
    suit_.clear();
    rank_.clear();
  }
}

/**
 * @brief Card::Card alternative constructor: e.g. Card("♥Q")
 * @param cardStr any combination of suit and rank as single string
 */
Card::Card(
    const std::string& cardStr) {
  // std::cout << cardStr.length();
  if (cardStr.length() >= 4) {
    suit_ = cardStr.substr(0, 3);
    rank_ = cardStr.at(3);
    if (rank_ == "1") rank_ = "10";

    if (std::ranges::find(suits, suit_) != suits.end() &&
        std::ranges::find(ranks, rank_) != ranks.end()) {
      initCard();
    } else {
      std::cerr << "Invalid card: " << cardStr << std::endl;
      suit_.clear();
      rank_.clear();
    }
  } else {
    std::cerr << "Invalid card string format: " << cardStr << std::endl;
  }
}

/**
 * @brief Card::Card alternative constructor: e.g. Card(std::pair("♥",
 * "Q"))
 * @param pair of suit and rank e.g. pair("♥", "Q")
 */
Card::Card(
    const std::pair<std::string, std::string>& pair)
    : Card(pair.first, pair.second) {}

/**
 * @brief Card::clone creates a deep copy of a card
 * @return returns a unique pointer to a card
 */
std::unique_ptr<Card> Card::clone() const {
  return std::make_unique<Card>(*this);  // Deep copy and unique ownership
}

/**
 * @brief Card::Card copy constructor
 * @param other
 */
Card::Card(
    const Card& other)
    : suit_(other.suit_), rank_(other.rank_) {
  initCard();
}

/**
 * @brief Card::operator = copy assignment operator
 * @param other
 * @return
 */
Card& Card::operator=(
    const Card& other) {
  if (this != &other) {
    suit_ = other.suit_;
    rank_ = other.rank_;
    initCard();
  }
  return *this;
}

/**
 * @brief Card::Card move constructor
 * @param other
 */
Card::Card(
    Card&& other) noexcept
    : suit_(std::move(other.suit_)),
      rank_(std::move(other.rank_)),
      pair_(std::move(other.pair_)),
      suitname_(std::move(other.suitname_)),
      rankname_(std::move(other.rankname_)),
      str_(std::move(other.str_)),
      name_(std::move(other.name_)),
      value_(other.value_),  // Copy primitive type
      power_(other.power_)   // Copy primitive type

{
  // Leave 'other' in a valid state
  other.suit_.clear();
  other.rank_.clear();
  other.suitname_.clear();
  other.rankname_.clear();
  other.str_.clear();
  other.name_.clear();
  other.value_ = 0;
  other.power_ = 0;
}

/**
 * @brief Card::operator = move assignment operator
 * @param other
 * @return
 */
Card& Card::operator=(
    Card&& other) noexcept {
  if (this != &other) {
    // Move resources from other to this object
    suit_ = std::move(other.suit_);
    rank_ = std::move(other.rank_);
    pair_ = std::move(other.pair_);
    suitname_ = std::move(other.suitname_);
    rankname_ = std::move(other.rankname_);
    str_ = std::move(other.str_);
    name_ = std::move(other.name_);
    value_ = other.value_;  // Just copy the value as it is a primitive type
    power_ = other.power_;

    // Leave 'other' in a valid state
    other.suit_.clear();
    other.rank_.clear();
    other.pair_ = {};  // Optional, but makes pair valid
    other.suitname_.clear();
    other.rankname_.clear();
    other.str_.clear();
    other.name_.clear();
    other.value_ = 0;
    other.power_ = 0;
  }
  return *this;
}

/**
 * @brief Card::operator == comparison operator ==
 * @param other
 * @return
 */
bool Card::operator==(
    const Card& other) const {
  return suit_ == other.suit_ && rank_ == other.rank_;
}

/**
 * @brief Card::operator <=> "space ship" operator <=>
 * @param other
 * @return
 */
std::strong_ordering Card::operator<=>(
    const Card& other) const {
  return value_ <=> other.value_;
}

/**
 * @brief Initializes the member fields of a Card object, e.g., for const Card&
 * card = Card("♥Q").
 *
 * @details
 * The following member fields are initialized:
 *
 * @section member_variables Class Member Variables
 * - `pair_` (std::pair<std::string, std::string>): A pair that stores the rank
 * and suit of the card.
 * - `suit_` (std::string): The suit of the card, initialized to "♥".
 * - `rank_` (std::string): The rank of the card, initialized to "Q".
 * - `suitname_` (std::string): The name of the suit (e.g., "hearts").
 * - `rankname_` (std::string): The name of the rank (e.g., "queen").
 * - `str_` (std::string): The string representation of the card, e.g., "♥Q".
 * - `name_` (std::string): The name used to identify the card image, e.g.,
 * "queen_of_hearts".
 * - `value_` (int): The value of the card (e.g. value_ of Q is initialized to
 * 3).
 * - `power_` (mutable int): The mutable power of the card, set according to the
 * enum class Rule { Unset, Suit, Grand, Null, Ramsch }.
 */
void Card::initCard() {
  if (suit_.empty() || rank_.empty()) return;

  pair_.first = suit_;
  pair_.second = rank_;
  setSuitname(suit_);
  setRankname(rank_);
  setStr();
  setName();
  setValue(rank_);
}

/**
 * @brief Card::setSuitname
 * @param suit
 *
 * - `suitname_` (std::string): The name of the suit (e.g., "hearts").
 */
void Card::setSuitname(
    const std::string& suit) {
  auto it = std::ranges::find(suits, suit);
  if (it != suits.end()) {
    suitname_ = suitnames[std::distance(suits.begin(), it)];
  }
}

/**
 * @brief Card::setRankname
 * @param rank
 *
 * - `rankname_` (std::string): The name of the rank (e.g., "queen").
 */

void Card::setRankname(
    const std::string& rank) {
  auto it = std::ranges::find(ranks, rank);
  if (it != ranks.end()) {
    rankname_ = ranknames[std::distance(ranks.begin(), it)];
  }
}
/**
 * @brief Card::setStr
 *
 * - `str_` (std::string): The string representation of the card, e.g., "♥Q".
 */
void Card::setStr() { str_ = suit_ + rank_; }

/**
 * @brief Card::setName
 *
 * - `name_` (std::string): The name used to identify the card image, e.g.,
 * "queen_of_hearts".
 */
void Card::setName() { name_ = rankname_ + "_of_" + suitname_ /* + ".png"*/; }

/**
 * @brief Card::setValue
 * @param rank
 *
 * - `value_` (int): The value of the card (e.g. value_ of Q is initialized to
 * 3).
 */
void Card::setValue(
    const std::string& rank) {
  if (rank == "A") {
    value_ = 11;
  } else if (rank == "10") {
    value_ = 10;
  } else if (rank == "K") {
    value_ = 4;
  } else if (rank == "Q") {
    value_ = 3;
  } else if (rank == "J") {
    value_ = 2;
  } else {
    value_ = 0;
  }
}

// public getters
std::string Card::suit() const { return suit_; }
std::string Card::rank() const { return rank_; }
std::string Card::suitname() const { return suitname_; }
std::string Card::rankname() const { return rankname_; }
std::string Card::str() const {
  if (suit_.empty() || rank_.empty()) {
    return "<invalid card>";
  }
  return str_;
}
std::string Card::name() const { return name_; }
bool Card::isEmpty() const { return (suit_ == "" && rank_ == ""); }

int Card::value() const { return value_; }

/**
 * @brief Card::power
 * @param rule
 * @param trumpSuit
 * @return the power of the card according to the
 * enum class Rule { Unset, Suit, Grand, Null, Ramsch } and the suit of trump
 * when Rule::Suit is set.
 */
int Card::power(
    Rule rule, const std::string& trumpSuit) const {
  if (rule == Rule::Suit || rule == Rule::Grand || rule == Rule::Ramsch) {
    auto it = PowerPriorityRanks.find(rank_);
    power_ = (it != PowerPriorityRanks.end()) ? it->second : 0;

    // Add 10 if the card belongs to the trump suit
    if (suit_ == trumpSuit) {
      power_ += 10;
    }

    if (rank_ == "J") {
      if (suit_ == "♦")
        power_ = 21;
      else if (suit_ == "♥")
        power_ = 22;
      else if (suit_ == "♠")
        power_ = 23;
      else if (suit_ == "♣")
        power_ = 24;
    }

  } else if (rule == Rule::Null) {
    auto it = PowerPriorityNull.find(rank_);
    power_ = (it != PowerPriorityNull.end()) ? it->second : 0;
  } else {
    power_ = 0;  // Default case if Rule is undefined
  }
  return power_;
}

/**
 * @brief Card::hasMoreValue
 * @param other
 * @return compares two cards and returns wether card has more value than other
 * card.
 */
bool Card::hasMoreValue(
    const Card& other) {
  if (this->value_ > other.value_) return true;
  return false;
}

/**
 * @brief Card::hasMorePower
 * @param other
 * @return compares two cards and returns wether card has more power than other
 * card.
 */
bool Card::hasMorePower(
    const Card& other) {
  if (this->power_ > other.power_) return true;
  return false;
}
