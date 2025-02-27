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

  jackDiamond.power("♠", Rule::Null);
  jackHeart.power("♠", Rule::Null);
  jackSpade.power("♠", Rule::Null);
  jackClub.power("♠", Rule::Null);
  aceDiamond.power("♠", Rule::Null);

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

  aceSpade.power(trumpSuit, Rule::Suit);
  tenSpade.power(trumpSuit, Rule::Suit);
  jackSpade.power(trumpSuit, Rule::Suit);
  queenHeart.power(trumpSuit, Rule::Suit);

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

  jackDiamond.power(trumpSuit = "J", Rule::Grand);
  jackHeart.power(trumpSuit = "J", Rule::Grand);
  jackSpade.power(trumpSuit = "J", Rule::Grand);
  jackClub.power(trumpSuit = "J", Rule::Grand);
  kingSpade.power(trumpSuit = "J", Rule::Grand);

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
