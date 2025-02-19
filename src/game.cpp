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
    for (int i = 1; i <= 3; i++) {
      blind_.moveTopCardTo(player_1.handdeck());
      blind_.moveTopCardTo(player_2.handdeck());
      blind_.moveTopCardTo(player_3.handdeck());
    }

    blind_.moveTopCardTo(skat_);
    blind_.moveTopCardTo(skat_);

    for (int i = 1; i <= 4; i++) {
      blind_.moveTopCardTo(player_1.handdeck());
      blind_.moveTopCardTo(player_2.handdeck());
      blind_.moveTopCardTo(player_3.handdeck());
    }
    for (int i = 1; i <= 3; i++) {
      blind_.moveTopCardTo(player_1.handdeck());
      blind_.moveTopCardTo(player_2.handdeck());
      blind_.moveTopCardTo(player_3.handdeck());
    }
  }
}
