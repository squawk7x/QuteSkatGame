// #pragma once
#ifndef CARD_H
#define CARD_H

// #include <QObject>
#include <QPushButton>
#include <memory>
#include <string>

#include "definitions.h"
// #include <unordered_map>
// #include <vector>

// Definitions for th3e global card properties
// extern std::vector<std::string> suits;
// extern std::vector<std::string> ranks;
// extern std::vector<std::string> suitnames;
// extern std::vector<std::string> ranknames;
// extern const std::unordered_map<std::string, int> PowerPrioritySuit;
// extern const std::unordered_map<std::string, int> PowerPriorityNull;

class Card : public QPushButton {
  Q_OBJECT

 private:
  // class member fields
  std::pair<std::string, std::string> pair_;
  std::string suit_;
  std::string rank_;
  std::string suitname_;
  std::string rankname_;
  std::string str_;
  std::string name_;
  int value_;
  mutable int power_;

 private:
  // Setters
  void initCard();
  void setSuitname(const std::string& suit);
  void setRankname(const std::string& rank);
  void setStr();
  void setName();
  void setValue(const std::string& rank);

 public:
  // Constructors
  // Card() = default;  // Default constructor
  explicit Card(const std::string& suit, const std::string& rank);
  explicit Card(const std::string& cardStr);
  explicit Card(const std::pair<std::string, std::string>& pair);
  ~Card() = default;  // Default Deconstructor

  Card(const Card& other);                 // Copy constructor
  Card& operator=(const Card& other);      // Copy assignment operator
  Card(Card&& other) noexcept;             // Move constructor
  Card& operator=(Card&& other) noexcept;  // Move assignment operator

  // Clone
  std::unique_ptr<Card> clone() const;

  // Operator overloads
  bool operator==(const Card& other) const;
  std::strong_ordering operator<=>(const Card& other) const;

 public:
  // Getters
  std::string suit() const;
  std::string rank() const;
  std::string suitname() const;
  std::string rankname() const;
  std::string str() const;
  std::string name() const;
  int value() const;
  int power(const std::string& trumpSuit = "", Rule rule = Rule::Suit) const;
  // void loadImage(bool isCardFaceVisible = true);
};

#endif  // CARD_H
