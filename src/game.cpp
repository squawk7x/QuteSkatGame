#include "game.h"

#include <QDebug>

Game::Game(
    QObject *parent)
    : QObject{parent} {}

void Game::initGame() {
  {
    std::ranges::for_each(suits, [&](const std::string& suit) {
      std::ranges::for_each(ranks, [&](const std::string& rank) {
        blind_.addCard(Card(suit, rank));
      });
    });
    blind_.shuffle();

    // Distribuite cards 3 - skat(2) - 4 - 3
    for (int i = 1; i <= 3; i++)
      for (Player* player : playerList_)
        blind_.moveTopCardTo(player->handdeck_);
    blind_.moveTopCardTo(skat_);
    blind_.moveTopCardTo(skat_);
    for (int i = 1; i <= 4; i++)
      for (Player* player : playerList_)
        blind_.moveTopCardTo(player->handdeck_);
    for (int i = 1; i <= 3; i++)
      for (Player* player : playerList_)
        blind_.moveTopCardTo(player->handdeck_);

    for (Player* player : playerList_) player->handdeck_.sortCardsByPattern();
  }
}

// Slots:
void Game::playCard(
    Card card) {
  auto& hand = playerList_[0]->handdeck_.cards();
  auto it = std::find(hand.begin(), hand.end(), card);

  if (it != hand.end()) {  // Card found
    if (trick_.cards().size() == 3) {
      playerList_[0]->tricks_.push_back(trick_);
      trick_.clearCards();
      emit(clearTrickLayout());
    }
    playerList_[0]->handdeck_.moveCardTo(card, trick_);
    qDebug() << "Card played: " << QString::fromStdString(card.str());
    qDebug() << "trick_: " << trick_.print();

    std::ranges::rotate(playerList_, playerList_.begin() + 1);
  } else {
    qDebug() << "Error: Card not found in your hand!";
  }
}
