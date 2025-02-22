#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "card.h"

TEST(
    CardTest, PowerForNullRule) {
  Card jackDiamond("♦", "J");
  Card jackHeart("♥", "J");
  Card jackSpade("♠", "J");
  Card jackClub("♣", "J");
  Card aceDiamond("♦", "A");

  jackDiamond.setPower("J", "♠", Rule::Null);
  jackHeart.setPower("J", "♠", Rule::Null);
  jackSpade.setPower("J", "♠", Rule::Null);
  jackClub.setPower("J", "♠", Rule::Null);
  aceDiamond.setPower("A", "♠", Rule::Null);

  EXPECT_EQ(jackDiamond.power(), 5);
  EXPECT_EQ(jackHeart.power(), 5);
  EXPECT_EQ(jackSpade.power(), 5);
  EXPECT_EQ(jackClub.power(), 5);
  EXPECT_EQ(aceDiamond.power(), 8);  // Aces follow rankToPowerNull
}

TEST(
    CardTest, PowerForSuitRule) {
  std::string trumpSuit = "♠";  // Spades are trump
  Card aceSpade("♠", "A");
  Card tenSpade("♠", "10");
  Card jackSpade("♠", "J");
  Card queenHeart("♥", "Q");

  aceSpade.setPower("A", trumpSuit, Rule::Suit);
  tenSpade.setPower("10", trumpSuit, Rule::Suit);
  jackSpade.setPower("J", trumpSuit, Rule::Suit);
  queenHeart.setPower("Q", trumpSuit, Rule::Suit);

  EXPECT_EQ(aceSpade.power(), 7 + 10);  // 7 (base) + 10 (trump bonus)
  EXPECT_EQ(tenSpade.power(), 6 + 10);  // 6 (base) + 10 (trump bonus)
  EXPECT_EQ(jackSpade.power(), 23);     // 8 (base) + 10 (trump bonus)
  EXPECT_EQ(queenHeart.power(), 4);     // Normal rankToPowerSuit (not trump)
}

TEST(
    CardTest, PowerForGrandRule) {
  std::string trumpSuit = "";  // Irrelevant in Grand (only J matters)
  Card jackDiamond("♦", "J");
  Card jackHeart("♥", "J");
  Card jackSpade("♠", "J");
  Card jackClub("♣", "J");
  Card kingSpade("♠", "K");

  jackDiamond.setPower("J", trumpSuit = "", Rule::Grand);
  jackHeart.setPower("J", trumpSuit = "", Rule::Grand);
  jackSpade.setPower("J", trumpSuit = "", Rule::Grand);
  jackClub.setPower("J", trumpSuit = "", Rule::Grand);
  kingSpade.setPower("K", trumpSuit = "", Rule::Grand);

  EXPECT_EQ(jackDiamond.power(), 21);  // Jacks always strong
  EXPECT_EQ(jackHeart.power(), 22);
  EXPECT_EQ(jackSpade.power(), 23);
  EXPECT_EQ(jackClub.power(), 24);
  EXPECT_EQ(kingSpade.power(), 5);  // Normal rankToPowerSuit (no trump bonus)
}

// Run all tests
int main(
    int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
