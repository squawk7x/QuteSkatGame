#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <unordered_map>
#include <vector>

// Enum declarations
enum class CardFace { Open, Closed };
enum class CardSize { Normal, Small };
enum class LinkTo { Skat, Handdeck, SoloPlayer, Trick };
enum class Rule { Unset, Suit, Grand, Null, Ramsch };
enum class Reizen { Normal, Reset, Preview };
enum class Passen { Ja, Nein };

// Extern declarations for global constants
extern const std::vector<std::string> suits;
extern const std::vector<std::string> suitnames;
extern const std::vector<std::string> ranks;
extern const std::vector<std::string> ranknames;

extern const std::unordered_map<std::string, int> trumpValue;
extern const std::unordered_map<std::string, int> PowerPriorityNull;
extern const std::unordered_map<std::string, int> PowerPrioritySuit;
extern const std::unordered_map<std::string, int> SortPriorityRank;
extern const std::unordered_map<std::string, int> SortPrioritySuit;
extern const std::unordered_map<std::string, int> SortPriorityJacks;

#endif  // DEFINITIONS_H
