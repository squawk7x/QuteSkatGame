#include <gtest/gtest.h>

#include "../src/game.h"
#include "../src/player.h"

class BiddingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create Players
    game_ = new Game();
  }

  void TearDown() override { delete game_; }

  Game* game_;
};

// Test case 1: game_->player_1 (40), game_->player_2 (30), game_->player_3 (20)
// -> max bid 33, winner: game_->player_1
TEST_F(
    BiddingTest, Scenario1) {
  game_->player_1.maxBieten_ = 40;
  game_->player_2.maxBieten_ = 30;
  game_->player_3.maxBieten_ = 20;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 33);
}

// Test case 2: game_->player_1 (20), game_->player_2 (30), game_->player_3 (40)
// -> max bid 33, winner: game_->player_3
TEST_F(
    BiddingTest, Scenario2) {
  game_->player_1.maxBieten_ = 20;
  game_->player_2.maxBieten_ = 30;
  game_->player_3.maxBieten_ = 40;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 33);
}

// Test case 3: game_->player_1 (30), game_->player_2 (40), game_->player_3 (20)
// -> max bid 30, winner: game_->player_2
TEST_F(
    BiddingTest, Scenario3) {
  game_->player_1.maxBieten_ = 30;
  game_->player_2.maxBieten_ = 40;
  game_->player_3.maxBieten_ = 20;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 30);
}

// Test case 4: game_->player_1 (0), game_->player_2 (20), game_->player_3 (0)
// -> max bid 18, winner: game_->player_2
TEST_F(
    BiddingTest, Scenario4) {
  game_->player_1.maxBieten_ = 0;
  game_->player_2.maxBieten_ = 20;
  game_->player_3.maxBieten_ = 0;

  game_->sagen();
  EXPECT_EQ(game_->gereizt_, 18);
}

// Test case 5: All players maxBieten_ = 0 -> max bid 0, no winner
TEST_F(
    BiddingTest, Scenario5) {
  game_->player_1.maxBieten_ = 0;
  game_->player_2.maxBieten_ = 0;
  game_->player_3.maxBieten_ = 0;

  game_->sagen();

  EXPECT_EQ(game_->gereizt_, 0);
}
