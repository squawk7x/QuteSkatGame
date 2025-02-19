// #pragma once
#ifndef CARD_H
#define CARD_H

#include <memory>
#include <string>
#include <vector>

// Definitions for th3e global card properties
extern std::vector<std::string> suits;
extern std::vector<std::string> ranks;
extern std::vector<std::string> suitnames;
extern std::vector<std::string> ranknames;

class Card {
 private:
  std::pair<std::string, std::string> pair_;
  std::string suit_;
  std::string rank_;
  std::string suitname_;
  std::string rankname_;
  std::string str_;
  std::string name_;
  int value_;

 public:
  // Constructors
  explicit Card(const std::string& suit, const std::string& rank);
  explicit Card(const std::string& cardStr);
  explicit Card(const std::pair<std::string, std::string>& pair);

  // Clone
  std::unique_ptr<Card> clone() const;

  // Rule of five
  Card(const Card& other);                 // Copy constructor
  Card& operator=(const Card& other);      // Copy assignment operator
  Card(Card&& other) noexcept;             // Move constructor
  Card& operator=(Card&& other) noexcept;  // Move assignment operator
  ~Card() = default;                       // Default Deconstructor

  // Operator overloads
  bool operator==(const Card& other) const;
  std::strong_ordering operator<=>(const Card& other) const;

  // Getters
  std::string suit() const;
  std::string rank() const;
  std::string suitname() const;
  std::string rankname() const;
  std::string str() const;
  std::string name() const;
  int value() const;

  // Setters
 private:
  void initCard();
  void setSuitname(const std::string& suit);
  void setRankname(const std::string& rank);
  void setStr();
  void setName();
  void setValue(const std::string& rank);
};

#endif  // CARD_H
