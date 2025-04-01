#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <QDebug>
#include <array>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "card.h"

template <typename Container>
void printContainer(
    const Container& container, const std::string& separator = ", ") {
  if (container.empty()) {
    qDebug() << "{}";
    return;
  }

  std::ostringstream oss;
  oss << "{ ";

  auto it = container.begin();
  oss << it->str();  // Convert Card to string manually
  ++it;

  for (; it != container.end(); ++it) {
    oss << separator << it->str();
  }

  oss << " }";
  qDebug() << QString::fromStdString(oss.str());
}

inline void printMap(
    const std::map<std::string, int>& cardMap) {
  QString output = "{ ";
  for (const auto& [suit, count] : cardMap) {
    output += QString("(%1, %2) ").arg(QString::fromStdString(suit)).arg(count);
  }
  output += "}";
  qDebug() << "cardMap:" << output;
}

template <std::size_t ROWS, std::size_t COLS>
void printMatrix(
    const std::array<std::array<bool, COLS>, ROWS>& matrix) {
  qDebug() << "   J A 1 K Q 9 8 7";
  for (std::size_t i = 0; i < matrix.size(); ++i) {
    QString line = QString::fromStdString(suits[i] + " ");
    for (bool value : matrix[i]) {
      line.append(value ? "X " : "- ");
    }
    qDebug().noquote() << line;
  }
}

// Function declarations (implementation in .cpp file)
std::string cardsToString(const std::vector<Card>& cards);
std::string patternToString(const std::vector<int>& pattern);

#endif  // HELPERFUNCTIONS_H
