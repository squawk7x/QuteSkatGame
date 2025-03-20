#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <unordered_map>
#include <vector>

enum class CardFace { Open, Closed };
enum class CardSize { Normal, Small };
enum class LinkTo { Skat, Handdeck, Trick };
enum class Rule { Unset, Suit, Grand, Null, Ramsch };
enum class Reizen { Normal, Reset, Preview };
enum class Bieten { Ja, Nein };

// Use `inline` for `const` global variables (only in header)
inline const std::vector<std::string> suits = {"♣", "♠", "♥", "♦"};
inline const std::vector<std::string> suitnames = {"clubs", "spades", "hearts",
                                                   "diamonds"};
inline const std::vector<std::string> ranks = {"J", "A", "10", "K",
                                               "Q", "9", "8",  "7"};
inline const std::vector<std::string> ranknames = {"jack",  "ace", "10", "king",
                                                   "queen", "9",   "8",  "7"};

inline const std::unordered_map<std::string, int> trumpValue = {
    {"J", 24}, {"♣", 12}, {"♠", 11}, {"♥", 10}, {"♦", 9}};

inline const std::unordered_map<std::string, int> PowerPriorityNull = {
    {"7", 1}, {"8", 2}, {"9", 3}, {"10", 4},
    {"J", 5}, {"Q", 6}, {"K", 7}, {"A", 8}};

inline const std::unordered_map<std::string, int> PowerPrioritySuit = {
    {"7", 1}, {"8", 2}, {"9", 3}, {"10", 6},
    {"J", 8}, {"Q", 4}, {"K", 5}, {"A", 7}};

// Rank priority for sorting within a suit
inline const std::unordered_map<std::string, int> SortPriorityRank = {
    {"J", 0}, {"A", 1}, {"10", 2}, {"K", 3},
    {"Q", 4}, {"9", 5}, {"8", 6},  {"7", 7}};

// Suit priority for general sorting
inline const std::unordered_map<std::string, int> SortPrioritySuit = {
    {"♣", 0}, {"♥", 1}, {"♠", 2}, {"♦", 3}};

// Special suit priority for Jokers (J)
inline const std::unordered_map<std::string, int> SortPriorityJacks = {
    {"♣", 0}, {"♠", 1}, {"♥", 2}, {"♦", 3}};

#endif  // DEFINITIONS_H
