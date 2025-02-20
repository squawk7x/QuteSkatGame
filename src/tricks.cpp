#include "tricks.h"

Tricks::Tricks() {}

void Tricks::print() {
  for (CardVec trick : tricks_) trick.print();
}
