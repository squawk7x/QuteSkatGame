#include "definitions.h"

const std::vector<std::string> suits = {"♣", "♥", "♠", "♦"};
const std::vector<std::string> ranks = {"J", "A", "10", "K",
                                        "Q", "9", "8",  "7"};
const std::vector<std::string> ranknames = {"jack",  "ace", "10", "king",
                                            "queen", "9",   "8",  "7"};
const std::vector<std::string> suitnames = {"clubs", "hearts", "spades",
                                            "diamonds"};

const std::unordered_map<std::string, int> rankToPowerNull = {
    {"7", 1}, {"8", 2}, {"9", 3}, {"10", 4},
    {"J", 5}, {"Q", 6}, {"K", 7}, {"A", 8}};

const std::unordered_map<std::string, int> rankToPowerSuit = {
    {"7", 1}, {"8", 2}, {"9", 3}, {"10", 6},
    {"J", 8}, {"Q", 4}, {"K", 5}, {"A", 7}};
