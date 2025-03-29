#include "helperFunctions.h"

#include <QDebug>

std::string cardsToString(
    const std::vector<Card>& cards) {
  std::string str;
  for (const Card& card : cards) {
    str += card.str() + " ";
  }
  return str;
}

std::string patternToString(
    const std::vector<int>& vec) {
  std::string str;
  for (int bit : vec) {
    str += (bit == 1) ? '1' : '0';
  }
  return str;
}
