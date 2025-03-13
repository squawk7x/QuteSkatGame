#include "card.h"

#include <iostream>
#include <memory>

#include "definitions.h"

// Constructor: Card("♥", "Q")
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

// Constructor: Card("♥Q")
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

// Constructor: Card(std::pair("♥", "Q"))
Card::Card(
    const std::pair<std::string, std::string>& pair)
    : Card(pair.first, pair.second) {}

// Clone
std::unique_ptr<Card> Card::clone() const {
  return std::make_unique<Card>(*this);  // Deep copy and unique ownership
}

// Copy constructor
Card::Card(
    const Card& other)
    : suit_(other.suit_), rank_(other.rank_) {
  initCard();
}

// Copy assignment
Card& Card::operator=(
    const Card& other) {
  if (this != &other) {
    suit_ = other.suit_;
    rank_ = other.rank_;
    initCard();
  }
  return *this;
}

// Move constructor
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

// Move assignment (Fixed Recursive Call)
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

// Comparison Operators
bool Card::operator==(
    const Card& other) const {
  return suit_ == other.suit_ && rank_ == other.rank_;
}

std::strong_ordering Card::operator<=>(
    const Card& other) const {
  return value_ <=> other.value_;
}

// Private Methods
void Card::initCard() {
  if (suit_.empty() || rank_.empty()) return;

  pair_.first = suit_;
  pair_.second = rank_;
  setSuitname(suit_);
  setRankname(rank_);
  setStr();
  setName();
  setValue(rank_);

  // loadImage();

  // setStyleSheet(
  //     "QPushButton { padding: 2px; font-size: 18px; border-radius: 5px; }");
  // adjustSize();
}

// private setters
void Card::setSuitname(
    const std::string& suit) {
  auto it = std::ranges::find(suits, suit);
  if (it != suits.end()) {
    suitname_ = suitnames[std::distance(suits.begin(), it)];
  }
}

void Card::setRankname(
    const std::string& rank) {
  auto it = std::ranges::find(ranks, rank);
  if (it != ranks.end()) {
    rankname_ = ranknames[std::distance(ranks.begin(), it)];
  }
}

void Card::setStr() { str_ = suit_ + rank_; }
void Card::setName() { name_ = rankname_ + "_of_" + suitname_ /* + ".png"*/; }

// Card values for german Skat game
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
std::string Card::str() const { return str_; }
std::string Card::name() const { return name_; }
int Card::value() const { return value_; }

// public methods
int Card::power(
    const std::string& trumpSuit, Rule rule) const {
  if (rule == Rule::Suit || rule == Rule::Grand || rule == Rule::Ramsch) {
    auto it = PowerPrioritySuit.find(rank_);
    power_ = (it != PowerPrioritySuit.end()) ? it->second : 0;

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

// void Card::loadImage(
//     bool isCardFaceVisible) {
//   setStr();  // Update the string representation of the card

//   QString imagePath;
//   QString rankname = QString::fromStdString(rankname_);
//   QString suitname = QString::fromStdString(suitname_);

//   // Use arg() with QString
//   // imagePath = QString(":res/cards/%1_of_%2.png").arg(rankname, suitname);

//   // Determine the image path based on visibility
//   if (isCardFaceVisible) {
//     imagePath = QString(":res/cards/%1_of_%2.png").arg(rankname, suitname);
//   } else {
//     imagePath = QString(":res/cards/backside_blue.png");
//   }

//   // setProperty("card-type", cardType_ == CardType::Small ? "Small" :
//   // "Normal");

//   // QPixmap pixmap(imagePath);  // Load the image as a QPixmap

//   // if (!pixmap.isNull() && cardType_ == CardType::Normal) {
//   //   setText("");             // Remove any text
//   //   setIcon(QIcon(pixmap));  // Set the icon

//   //   QSize buttonSize = QSize(500, 726) * 0.2;
//   //   setIconSize(buttonSize);
//   // } else {
//   //   setIcon(QIcon());  // Clear any existing icon
//   //   setText(isCardFaceVisible && isEnabled() ? str_ : "▓▓");
//   // }

//   // Notify Qt to reapply the stylesheet
//   // style()->unpolish(this);
//   // style()->polish(this);
// }
