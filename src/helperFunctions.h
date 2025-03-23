#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <map>
#include <string>
#include <vector>

#include "card.h"

std::string cardsToString(std::vector<Card> cards);
std::string patternToString(const std::vector<int>& pattern);
void printMap(const std::map<std::string, int>& suitMap);

template <std::size_t ROWS, std::size_t COLS>
void printMatrix(
    const std::array<std::array<bool, COLS>, ROWS>& matrix) {
  qDebug() << "   J A 1 K Q 9 8 7";
  for (std::size_t i = 0; i < matrix.size(); ++i) {
    QString line = QString::fromStdString(suits[i] + " ");
    for (bool value : matrix[i]) {
      line.append(value ? "X " : "- ");
    }
    qDebug().noquote()
        << line;  // noquote() prevents additional formatting/newlines
  }
}
#endif  // HELPERFUNCTIONS_H
