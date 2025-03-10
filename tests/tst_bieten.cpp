#include <gtest/gtest.h>

#include "game.h"  // Include your Game class header

class GameTest : public ::testing::Test {
 protected:
  void SetUp() override { game = new Game(); }

  void TearDown() override { delete game; }

  Game* game;
};

// Helper function to set player bidding limits

// Test Case 1: One player bids, others pass
TEST_F(
    GameTest, OnlyOnePlayerBids) {
  game->player_1.maxBieten_ = 216;
  game->player_2.maxBieten_ = 0;
  game->player_3.maxBieten_ = 0;

  game->bieten(false);  // No manual pass, let the function execute

  EXPECT_EQ(game->player_1.isSolo_, true);
  EXPECT_EQ(game->player_2.isSolo_, false);
  EXPECT_EQ(game->player_3.isSolo_, false);
}

// Test Case 2: Two players participate, third passes
TEST_F(
    GameTest, TwoPlayersBidding) {
  game->player_1.maxBieten_ = 0;
  game->player_2.maxBieten_ = 24;
  game->player_3.maxBieten_ = 30;

  game->bieten(false);

  EXPECT_EQ(game->player_1.isSolo_, false);
  EXPECT_EQ(game->player_2.isSolo_, false);
  EXPECT_EQ(game->player_3.isSolo_, true);
}

// Test Case 3: No one bids
TEST_F(
    GameTest, NoPlayersBidding) {
  game->player_1.maxBieten_ = 0;
  game->player_2.maxBieten_ = 0;
  game->player_3.maxBieten_ = 0;

  game->bieten(false);

  EXPECT_EQ(game->player_1.isSolo_, false);
  EXPECT_EQ(game->player_2.isSolo_, false);
  EXPECT_EQ(game->player_3.isSolo_, false);
}

// Test Case 4: Players pass manually
TEST_F(
    GameTest, PlayersPassManually) {
  game->player_1.maxBieten_ = 30;
  game->player_2.maxBieten_ = 30;
  game->player_3.maxBieten_ = 30;

  game->bieten(true);  // Pass manually

  EXPECT_EQ(game->player_1.isSolo_, false);
  EXPECT_EQ(game->player_2.isSolo_, true);
  EXPECT_EQ(game->player_3.isSolo_, false);
}
