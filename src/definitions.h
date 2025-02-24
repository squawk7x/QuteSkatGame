#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <unordered_map>
#include <vector>

// Declare global variables as extern
extern const std::vector<std::string> suits;
extern const std::vector<std::string> ranks;
extern const std::vector<std::string> ranknames;
extern const std::vector<std::string> suitnames;

extern const std::unordered_map<std::string, int> rankToPowerNull;
extern const std::unordered_map<std::string, int> rankToPowerSuit;

#endif  // DEFINITIONS_H
