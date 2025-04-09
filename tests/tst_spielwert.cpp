#include <gtest/gtest.h>

#include "../src/game.cpp"  // or better: include the header if possible
#include "../src/player.h"  // assuming Player is declared here

class SpielwertTest : public ::testing::Test {
 protected:
  Game game;
  Player player;

  void SetUp() override {
    // player.name_ = "TestPlayer";
    player.spitzen_ = 2;  // Jacks: 3 highest in a row
  }
};

TEST_F(
    SpielwertTest, GrandHandOuvertDesired) {
  player.desiredRule_ = Rule::Grand;
  player.desiredTrump_ = "";
  player.desiredHand_ = true;
  player.desiredSchneider_ = true;
  player.desiredSchneiderAngesagt_ = true;
  player.desiredSchwarz_ = true;
  player.desiredSchwarzAngesagt_ = true;
  player.desiredOuvert_ = true;

  int expected = (2 + 1 + 6) * 24;  // spitzen + 1 + stufen * base
  EXPECT_EQ(game.spielwert(&player, Spielwert::Desired), expected);
}

TEST_F(
    SpielwertTest, SuitGamePlayed) {
  // Setup game state (played state)
  game.rule_ = Rule::Suit;
  game.trump_ = "♥";  // must match trumpValue map
  game.hand_ = true;
  game.schneiderAngesagt_ = true;
  game.schwarzAngesagt_ = false;
  game.ouvert_ = false;
  game.schneider_ = true;
  game.schwarz_ = false;

  player.points_ = 91;
  // player.tricks_.resize(6);  // not schwarz
  player.spitzen_ = 3;

  // game.trump_["hearts"] = 10;

  int stufen = 1 + 1 + 1;  // hand + schneider + angesagt
  int expected = (3 + 1 + stufen) * 10;

  EXPECT_EQ(game.spielwert(&player, Spielwert::Played), expected);
}

TEST_F(
    SpielwertTest, NullHandOuvert) {
  game.rule_ = Rule::Null;
  game.hand_ = true;
  game.ouvert_ = true;

  int expected = 59;
  EXPECT_EQ(game.spielwert(&player, Spielwert::Played), expected);
}

TEST_F(
    SpielwertTest, NullSimple) {
  game.rule_ = Rule::Null;
  game.hand_ = false;
  game.ouvert_ = false;

  int expected = 23;
  EXPECT_EQ(game.spielwert(&player, Spielwert::Played), expected);
}
