#include "bogzilla/board.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using bogzilla::Board;

namespace {

std::vector<int> neighbors_of(const Board& board, int index) {
  const auto span = board.neighbors(index);
  std::vector<int> out(span.begin(), span.end());
  std::sort(out.begin(), out.end());
  return out;
}

}  // namespace

// A real Wordshake board: rows "tsdt", "rnop", "iodi", "pwya".
TEST(BoardParse, WordshakeBoard) {
  const auto board = Board::parse("tsdtrnopiodipwya");
  ASSERT_TRUE(board);
  EXPECT_EQ(board->size(), 4);
  EXPECT_EQ(board->tile_count(), 16);
  EXPECT_EQ(board->tile(0), "t");
  EXPECT_EQ(board->tile(6), "o");
  EXPECT_EQ(board->tile(15), "a");
}

TEST(BoardParse, IgnoresWhitespaceAndCase) {
  const auto board = Board::parse(" TSDT rnop\nIoDi\tpwya ");
  ASSERT_TRUE(board);
  EXPECT_EQ(board->to_string(), "tsdt rnop iodi pwya");
}

TEST(BoardParse, SupportedSizes) {
  EXPECT_EQ(Board::parse("abcdefghi")->size(), 3);
  EXPECT_EQ(Board::parse("abcdefghijklmnop")->size(), 4);
  EXPECT_EQ(Board::parse("abcdefghijklmnopqrstuvwxy")->size(), 5);
}

TEST(BoardParse, RejectsUnsupportedSizes) {
  EXPECT_FALSE(Board::parse(""));
  EXPECT_FALSE(Board::parse("abcd"));
  EXPECT_FALSE(Board::parse("abcdefghijklmno"));                       // 15
  EXPECT_FALSE(Board::parse("abcdefghijklmnopabcdefghijklmnopabcd"));  // 36
}

TEST(BoardParse, RejectsNonLetters) {
  EXPECT_FALSE(Board::parse("abcdefgh1"));
  EXPECT_FALSE(Board::parse("abcdefgh-"));
}

TEST(BoardParse, QBecomesQuTile) {
  const auto bare = Board::parse("qbcdefghi");
  const auto spelled = Board::parse("qubcdefghi");
  ASSERT_TRUE(bare);
  ASSERT_TRUE(spelled);
  EXPECT_EQ(bare->tile(0), "qu");
  EXPECT_EQ(spelled->tile(0), "qu");
  EXPECT_EQ(spelled->tile(1), "b");
  EXPECT_EQ(spelled->tile_count(), 9);
}

TEST(BoardParse, RoundTrips) {
  const auto board = Board::parse("qu s d t r n o p i o d i p w y a");
  ASSERT_TRUE(board);
  const auto again = Board::parse(board->to_string());
  ASSERT_TRUE(again);
  EXPECT_EQ(again->to_string(), board->to_string());
}

TEST(BoardFromTiles, AcceptsMultiLetterTiles) {
  const std::vector<std::string> tiles = {"Qu", "u", "th", "a", "b", "c", "d", "e", "f"};
  const auto board = Board::from_tiles(tiles);
  ASSERT_TRUE(board);
  EXPECT_EQ(board->tile(0), "qu");
  EXPECT_EQ(board->tile(1), "u");
  EXPECT_EQ(board->tile(2), "th");
}

TEST(BoardFromTiles, RejectsBadTiles) {
  const std::vector<std::string> empty_tile = {"", "b", "c", "d", "e", "f", "g", "h", "i"};
  const std::vector<std::string> digit = {"a", "b", "c", "d", "e", "f", "g", "h", "9"};
  EXPECT_FALSE(Board::from_tiles(empty_tile));
  EXPECT_FALSE(Board::from_tiles(digit));
}

TEST(BoardNeighbors, CountsOn4x4) {
  const auto board = Board::parse("abcdefghijklmnop");
  ASSERT_TRUE(board);
  EXPECT_EQ(board->neighbors(0).size(), 3u);   // corner
  EXPECT_EQ(board->neighbors(1).size(), 5u);   // edge
  EXPECT_EQ(board->neighbors(5).size(), 8u);   // inner
  EXPECT_EQ(board->neighbors(15).size(), 3u);  // corner
}

TEST(BoardNeighbors, ExactTiles) {
  const auto board = Board::parse("abcdefghijklmnop");
  ASSERT_TRUE(board);
  EXPECT_EQ(neighbors_of(*board, 0), (std::vector<int>{1, 4, 5}));
  EXPECT_EQ(neighbors_of(*board, 5), (std::vector<int>{0, 1, 2, 4, 6, 8, 9, 10}));
  EXPECT_EQ(neighbors_of(*board, 7), (std::vector<int>{2, 3, 6, 10, 11}));
}

TEST(BoardNeighbors, CenterOf3x3TouchesEverything) {
  const auto board = Board::parse("abcdefghi");
  ASSERT_TRUE(board);
  EXPECT_EQ(neighbors_of(*board, 4), (std::vector<int>{0, 1, 2, 3, 5, 6, 7, 8}));
}

TEST(BoardAdjacent, MatchesNeighbors) {
  const auto board = Board::parse("abcdefghijklmnopqrstuvwxy");
  ASSERT_TRUE(board);
  for (int a = 0; a < board->tile_count(); ++a) {
    const auto n = neighbors_of(*board, a);
    for (int b = 0; b < board->tile_count(); ++b) {
      const bool listed = std::find(n.begin(), n.end(), b) != n.end();
      EXPECT_EQ(board->adjacent(a, b), listed) << "a=" << a << " b=" << b;
    }
  }
}

TEST(BoardAdjacent, NoWrapAcrossRows) {
  const auto board = Board::parse("abcdefghijklmnop");
  ASSERT_TRUE(board);
  EXPECT_FALSE(board->adjacent(3, 4));  // end of row 0, start of row 1
  EXPECT_FALSE(board->adjacent(0, 0));
}
