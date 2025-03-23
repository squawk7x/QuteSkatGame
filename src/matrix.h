#ifndef MATRIX_H
#define MATRIX_H

#include <QDebug>
#include <QString>
#include <array>

#include "card.h"

class Matrix {
 public:
  // Dimensions
  static constexpr std::size_t ROWS = 4;
  static constexpr std::size_t COLS = 8;

  // Constructor: initializes the matrix with all false (not played)
  Matrix();

  // Mark a card as played using a Card object
  void setField(const Card& card);

  // Print the matrix to the debug output
  void print() const;

  // Reset the matrix (mark all cards as not played)
  void reset();

 private:
  std::array<std::array<bool, COLS>, ROWS> fields;
};

#endif  // MATRIX_H
