#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <map>
#include <string>
#include <vector>

#include "card.h"

std::string cardsToString(std::vector<Card> cards);
std::string patternToString(const std::vector<int>& pattern);
void printMap(const std::map<std::string, int>& suitMap);

#endif  // HELPERFUNCTIONS_H
