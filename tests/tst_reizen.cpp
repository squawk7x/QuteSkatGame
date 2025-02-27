#include <gtest/gtest.h>

#include "game.h"

class GameTest : public ::testing::Test {
 protected:
  void SetUp() override {
    game = new Game();

    game->player_1.maxBieten_ = 24;
    game->player_2.maxBieten_ = 24;
    game->player_3.maxBieten_ = 24;

    // game->playerList_.push_back(&game->player_1);
    // game->playerList_.push_back(&game->player_2);
    // game->playerList_.push_back(&game->player_3);
  }

  // void TearDown() override { delete game; }

  Game* game;
};

TEST_F(
    GameTest, BiddingStopsCorrectly) {
  game->player_1.maxBieten_ = 24;
  game->player_2.maxBieten_ = 24;
  game->player_3.maxBieten_ = 24;

  game->sagen(0, 1, 18);  // Player 1 starts, Player 2 is the first responder

  // Ensure no player bid higher than their max
  EXPECT_LE(game->gereizt_, game->player_1.maxBieten_);
  EXPECT_LE(game->gereizt_, game->player_2.maxBieten_);
  EXPECT_LE(game->gereizt_, game->player_3.maxBieten_);

  // Check that the highest bid is correctly stored
}
