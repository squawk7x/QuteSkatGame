#include <gtest/gtest.h>

#include "game.h"

class ReizenTest : public ::testing::Test {
 protected:
  void SetUp() override {
    game_ = new Game();
    game_->reizen(true);
  }

  void TearDown() override { delete game_; }

  Game* game_;
};

TEST_F(
    ReizenTest, ResetTest) {
  EXPECT_EQ(game_->reizen(true), 0);
}

TEST_F(
    ReizenTest, IncrementTest) {
  EXPECT_EQ(game_->reizen(), 18);
  EXPECT_EQ(game_->reizen(), 20);
  EXPECT_EQ(game_->reizen(), 22);
  EXPECT_EQ(game_->reizen(), 23);
}
TEST_F(
    ReizenTest, PreviewTest) {
  EXPECT_EQ(game_->reizen(), 18);
  EXPECT_EQ(game_->reizen(false, true), 20);  // preview = true
  EXPECT_EQ(game_->reizen(), 20);
}

TEST_F(
    ReizenTest, MaxBoundaryTest) {
  for (int i = 0; i < 60; ++i) {
    game_->reizen(false);
  }
  EXPECT_EQ(game_->reizen(), 216);  // Should cap at max value
}

int main(
    int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
