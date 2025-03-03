#include <gtest/gtest.h>

#include "../src/game.h"
#include "../src/player.h"

class BiddingTest : public ::testing::Test {
 protected:
  void SetUp() override { game_ = new Game(); }

  void TearDown() override { delete game_; }

  Game* game_;
};

// Test case 1: Player 1 wins with 33
TEST_F(
    BiddingTest, Scenario1) {
  game_->player_1.maxBieten_ = 40;
  game_->player_2.maxBieten_ = 30;
  game_->player_3.maxBieten_ = 20;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 33);
}

// Test case 2: Player 3 wins with 33
TEST_F(
    BiddingTest, Scenario2) {
  game_->player_1.maxBieten_ = 20;
  game_->player_2.maxBieten_ = 30;
  game_->player_3.maxBieten_ = 40;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 33);
}

// Test case 3: Player 2 wins with 30
TEST_F(
    BiddingTest, Scenario3) {
  game_->player_1.maxBieten_ = 30;
  game_->player_2.maxBieten_ = 40;
  game_->player_3.maxBieten_ = 20;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 30);
}

// Test case 4: Player 2 wins with 18
TEST_F(
    BiddingTest, Scenario4) {
  game_->player_1.maxBieten_ = 0;
  game_->player_2.maxBieten_ = 20;
  game_->player_3.maxBieten_ = 0;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 18);
}

// Test case 5: No player bids
TEST_F(
    BiddingTest, Scenario5) {
  game_->player_1.maxBieten_ = 0;
  game_->player_2.maxBieten_ = 0;
  game_->player_3.maxBieten_ = 0;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 0);
}
