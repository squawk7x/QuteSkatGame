#include "helperFunctions.h"

#include <QDebug>
#include <QObject>

std::string cardsToString(
    std::vector<Card> cards) {
  std::string str;

  for (const Card& card : cards) {
    str += card.str() + " ";
  }

  return str;
}

std::string patternToString(
    const std::vector<int>& vec) {
  std::string str;

  for (int bit : vec) {
    str += ((bit == 1) ? '1' : '0');
  }

  return str;
}

void printMap(
    const std::map<std::string, int>& suitMap) {
  QString output = "{ ";

  for (const auto& [suit, count] : suitMap) {
    output += QString("(%1, %2) ").arg(QString::fromStdString(suit)).arg(count);
  }

  output += "}";

  #ifdef DEBUG
  qDebug() << "suitMap:" << output;
  #endif

}
