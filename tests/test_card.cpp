#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "../src/card.h"  // Assuming card.h contains the class definition

// Test that Card constructors works correctly
TEST(
    CardTest, Constructors) {
  Card card1("♥", "Q");
  EXPECT_EQ(card1.suit(), "♥");
  EXPECT_EQ(card1.rank(), "Q");
  EXPECT_EQ(card1.suitname(), "hearts");
  EXPECT_EQ(card1.rankname(), "queen");
  EXPECT_EQ(card1.str(), "♥Q");
  EXPECT_EQ(card1.name(), "queen_of_hearts");
  EXPECT_EQ(card1.value(), 3);  // Queen has value 3

  Card card2("♠A");
  EXPECT_EQ(card2.suit(), "♠");
  EXPECT_EQ(card2.rank(), "A");
  EXPECT_EQ(card2.name(), "ace_of_spades");
  EXPECT_EQ(card2.value(), 11);  // Ace has value 11

  Card card3(std::pair("♣", "J"));
  EXPECT_EQ(card3.suit(), "♣");
  EXPECT_EQ(card3.rank(), "J");
  EXPECT_EQ(card3.name(), "jack_of_clubs");
  EXPECT_EQ(card3.value(), 2);  // J has value 2
}

// Test copy constructor
TEST(
    CardTest, CopyConstructor) {
  // Create a card object with specific suit and rank
  Card card1("♥", "10");
  Card card2(card1);  // create card2 by copy from card1

  // Check that card1 and card2 now have the same values
  EXPECT_EQ(card2.suit(), "♥");
  EXPECT_EQ(card2.rank(), "10");
  EXPECT_EQ(card2.value(), 10);

  EXPECT_EQ(card1.suit(), "♥");
  EXPECT_EQ(card1.rank(), "10");
  EXPECT_EQ(card1.value(), 10);
}

// Test copy assignment operator
TEST(
    CardTest, CopyAssignment) {
  // Create a card object with specific suit and rank
  Card card1("♦", "J");
  EXPECT_EQ(card1.suit(), "♦");
  EXPECT_EQ(card1.rank(), "J");
  EXPECT_EQ(card1.value(), 2);  // King has value 10

  // Use copy assignment to copy from card1 to card2
  Card card2("♥", "Q");  // Initialize card2 with different values first
  card2 = card1;         // Assign card1 to card2

  // Check that card1 and card2 now have the same values
  EXPECT_EQ(card2.suit(), "♦");
  EXPECT_EQ(card2.rank(), "J");
  EXPECT_EQ(card2.value(), 2);

  // card1 should remain unchanged
  EXPECT_EQ(card1.suit(), "♦");
  EXPECT_EQ(card1.rank(), "J");
  EXPECT_EQ(card1.value(), 2);
}

// Test move constructor
TEST(
    CardTest, MoveConstructor) {
  // Create a card object with specific suit and rank
  Card card1("♥", "10");
  Card card2(std::move(card1));  // create card2 by move from card1

  // Check that card1 is empty
  EXPECT_EQ(card1.suit(), "");
  EXPECT_EQ(card1.rank(), "");
  EXPECT_EQ(card1.value(), 0);

  // Check that card2 now have the values of card1
  EXPECT_EQ(card2.suit(), "♥");
  EXPECT_EQ(card2.rank(), "10");
  EXPECT_EQ(card2.value(), 10);
}

// Test move assignment operator
TEST(
    CardTest, MoveAssignment) {
  // Create a card object with specific suit and rank
  Card card1("♦", "A");
  EXPECT_EQ(card1.suit(), "♦");
  EXPECT_EQ(card1.rank(), "A");
  EXPECT_EQ(card1.value(), 11);  // Ace has value 11

  // Move card1 to card2 using move assignment operator
  Card card2 = std::move(card1);

  // Check the state of card1 (should be in a valid, unspecified state)
  EXPECT_EQ(card1.suit(), "");
  EXPECT_EQ(card1.rank(), "");
  EXPECT_EQ(card1.value(), 0);  // card1 should be in a valid state with 0 value

  // Check the state of card2 (should have the values of card1)
  EXPECT_EQ(card2.suit(), "♦");
  EXPECT_EQ(card2.rank(), "A");
  EXPECT_EQ(card2.value(), 11);  // card2 should retain the original value
}
