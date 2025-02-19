#include <algorithm>
#include <iostream>

#include "card.h"
#include "cardvec.h"

// std::vector<std::string> suits = {"♦", "♠", "♥", "♣"};
// std::vector<std::string> ranks = {"6", "7", "8", "9", "10", "J", "Q", "K",
// "A"};

using namespace std;

int main() {
  CardVec blind{32};

  CardVec hand1{10};
  CardVec hand2{10};
  CardVec hand3{10};
  CardVec skat{2};

  std::ranges::for_each(suits, [&](const std::string& suit) {
    std::ranges::for_each(ranks, [&](const std::string& rank) {
      blind.addCard(Card(suit, rank));
    });
  });

  blind.shuffle();

  // blind.print();
  std::cout << "\n";

  for (int i = 1; i <= 10; i++) {
    blind.moveTopCardTo(hand1);
    blind.moveTopCardTo(hand2);
    blind.moveTopCardTo(hand3);
  }
  hand1.sortCardsByPattern();
  hand2.sortCardsByPattern();
  hand3.sortCardsByPattern();

  blind.moveTopCardTo(skat);
  blind.moveTopCardTo(skat);

  // blind.moveCardTo(Card("♥", "Q"), hand1);

  std::cout << "Player 2: ";
  hand2.print();
  std::cout << "      ";
  std::cout << "Player 3: ";
  hand3.print();
  std::cout << "\n";
  std::cout << "\n";

  std::cout << "Skat: ";
  skat.print();
  std::cout << "\n";
  std::cout << "\n";
  std::cout << "Player 1: ";
  hand1.print();
  std::cout << "\n";

  blind.print();
  std::cout << "\n";

  return 0;
}
