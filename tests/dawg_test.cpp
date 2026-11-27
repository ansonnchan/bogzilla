#include "bogzilla/dawg.hpp"

#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "bogzilla/packed_trie.hpp"
#include "bogzilla/solver.hpp"
#include "bogzilla/trie.hpp"
#include "fixtures.hpp"

using bogzilla::Board;
using bogzilla::Dawg;

TEST(Dawg, Empty) {
  const auto dawg = Dawg::from_words({});
  EXPECT_EQ(dawg.word_count(), 0u);
  EXPECT_FALSE(dawg.contains("a"));
  EXPECT_FALSE(dawg.has_prefix(""));
}

TEST(Dawg, WordsAndPrefixes) {
  const auto dawg = Dawg::from_words({"tied", "tie", "ties", "hot", "Hot", "don't"});
  EXPECT_EQ(dawg.word_count(), 4u);
  EXPECT_TRUE(dawg.contains("tie"));
  EXPECT_TRUE(dawg.contains("tied"));
  EXPECT_TRUE(dawg.contains("HOT"));
  EXPECT_FALSE(dawg.contains("ti"));
  EXPECT_TRUE(dawg.has_prefix("ti"));
  EXPECT_FALSE(dawg.contains("hots"));
}

TEST(Dawg, SharesEndings) {
  const std::vector<std::string> words = {"bat", "bats", "cat", "cats", "hat", "hats"};
  const auto trie = bogzilla::PackedTrie::from_words(words);
  const auto dawg = Dawg::from_words(words);
  // The "at"/"ats" tail is stored once instead of three times.
  EXPECT_LT(dawg.node_count(), trie.node_count());
  for (const auto& word : words) EXPECT_TRUE(dawg.contains(word)) << word;
  EXPECT_FALSE(dawg.contains("ba"));
  EXPECT_FALSE(dawg.contains("mats"));
}

// "bat" and "cat" end at the same Dawg node, so deduping by node would drop one of them.
TEST(Dawg, SolverKeepsWordsThatShareAnEndNode) {
  // b c x
  // a t x
  // x x x
  const auto board = Board::parse("bcxatxxxx");
  ASSERT_TRUE(board);
  const auto found = bogzilla::solve(*board, Dawg::from_words({"bat", "cat"}));
  ASSERT_EQ(found.size(), 2u);
  EXPECT_EQ(found[0].word, "bat");
  EXPECT_EQ(found[1].word, "cat");
}

// Same answers as the baseline Trie on the full fallback dictionary and every saved board.
TEST(Dawg, MatchesTrieOnEnable) {
  std::ifstream in(BOGZILLA_DICTIONARY);
  std::vector<std::string> words;
  for (std::string word; in >> word;) words.push_back(word);
  ASSERT_GT(words.size(), 100000u);

  bogzilla::Trie trie;
  for (const auto& word : words) trie.insert(word);
  const auto dawg = Dawg::from_words(words);

  EXPECT_EQ(dawg.word_count(), trie.word_count());
  EXPECT_LT(dawg.node_count(), trie.node_count());
  for (const auto& word : words) {
    ASSERT_TRUE(dawg.contains(word)) << word;
    const std::string longer = word + 'q';
    ASSERT_EQ(dawg.contains(longer), trie.contains(longer)) << longer;
    const std::string shorter = word.substr(0, word.size() - 1);
    ASSERT_EQ(dawg.contains(shorter), trie.contains(shorter)) << shorter;
  }

  for (const auto& fixture :
       bogzilla::testing::load_board_fixtures(BOGZILLA_FIXTURES_DIR "/wordshake_boards.txt")) {
    const auto board = Board::from_tiles(fixture.tiles);
    ASSERT_TRUE(board);
    const auto a = bogzilla::solve(*board, trie);
    const auto b = bogzilla::solve(*board, dawg);
    ASSERT_EQ(a.size(), b.size()) << board->to_string();
    for (std::size_t i = 0; i < a.size(); ++i) {
      EXPECT_EQ(a[i].word, b[i].word);
      EXPECT_EQ(a[i].path, b[i].path);
    }
  }
}
