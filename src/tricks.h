#ifndef TRICKS_H
#define TRICKS_H

#include "cardvec.h"

class Tricks {
 public:
  std::vector<CardVec> tricks_{};

  Tricks();

  void print();
};

#endif  // TRICKS_H
