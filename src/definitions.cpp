#include "definitions.h"

// Suits and suit names
const std::vector<std::string> suits = {"♣", "♥", "♠", "♦"};
const std::vector<std::string> suitnames = {"clubs", "hearts", "spades",
                                            "diamonds"};

// Ranks and rank names
const std::vector<std::string> ranks = {"J", "A", "10", "K",
                                        "Q", "9", "8",  "7"};
const std::vector<std::string> ranknames = {"jack",  "ace", "10", "king",
                                            "queen", "9",   "8",  "7"};

// Trump values H = Grand Hand J = Grand
const std::unordered_map<std::string, int> trumpValue = {
    {"♣", 12}, {"♠", 11}, {"♥", 10}, {"♦", 9}};

// Power priorities
const std::unordered_map<std::string, int> PowerPriorityNull = {
    {"7", 0}, {"8", 1}, {"9", 2}, {"10", 3},
    {"J", 4}, {"Q", 5}, {"K", 6}, {"A", 7}};

const std::unordered_map<std::string, int> PowerPriorityRanks = {
    {"7", 0}, {"8", 1}, {"9", 2}, {"10", 5},
    {"J", 7}, {"Q", 3}, {"K", 4}, {"A", 6}};

// Sorting priorities
const std::unordered_map<std::string, int> SortPriorityNull = {
    {"A", 1},  {"K", 2}, {"Q", 3}, {"J", 4},
    {"10", 5}, {"9", 6}, {"8", 7}, {"7", 8}};

const std::unordered_map<std::string, int> SortPriorityRanks = {
    {"A", 1}, {"10", 2}, {"K", 3}, {"Q", 4}, {"9", 5}, {"8", 6}, {"7", 7}};

const std::unordered_map<std::string, int> SortPrioritySuits = {
    {"♣", 1}, {"♥", 2}, {"♠", 3}, {"♦", 4}};

// Special suit priority for Jokers (J)
const std::unordered_map<std::string, int> SortPriorityJacks = {
    {"♣", 1}, {"♠", 2}, {"♥", 3}, {"♦", 4}};
