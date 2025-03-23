#include "matrix.h"

Matrix::Matrix() {
  for (auto& row : fields) row.fill(false);
}

void Matrix::setField(
    const Card& card) {
  // Use ranges to find the suit and rank indices
  auto suitIt = std::ranges::find(suits, card.suit());
  auto rankIt = std::ranges::find(ranks, card.rank());
  auto suitIndex = std::distance(suits.begin(), suitIt);
  auto rankIndex = std::distance(ranks.begin(), rankIt);

  if (suitIndex < suits.size() && rankIndex < ranks.size())
    fields[suitIndex][rankIndex] = true;
}

bool Matrix::isFieldSet(
    const Card& card) {
  // Use ranges to find the suit and rank indices
  auto suitIt = std::ranges::find(suits, card.suit());
  auto rankIt = std::ranges::find(ranks, card.rank());
  auto suitIndex = std::distance(suits.begin(), suitIt);
  auto rankIndex = std::distance(ranks.begin(), rankIt);

  if (suitIndex < suits.size() && rankIndex < ranks.size())
    return fields[suitIndex][rankIndex];
  return false;
}

void Matrix::print() const {
  // Build and print the header row with rank names
  QString header = "   ";
  for (const auto& rank : ranks)
    header.append(QString::fromStdString(rank).left(1) + " ");
  qDebug().noquote() << header;

  // Print each row with the suit name at the beginning
  for (std::size_t i = 0; i < ROWS; ++i) {
    QString line = QString::fromStdString(suits[i]) + " ";
    for (std::size_t j = 0; j < COLS; ++j) {
      line.append(fields[i][j] ? "X " : "- ");
    }
    qDebug().noquote() << line;
  }
}

void Matrix::reset() {
  for (auto& row : fields) row.fill(false);
}
