#include "bogzilla/packed_trie.hpp"

#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "bogzilla/solver.hpp"
#include "bogzilla/trie.hpp"
#include "fixtures.hpp"

using bogzilla::PackedTrie;
using bogzilla::Trie;

TEST(PackedTrie, Empty) {
  const auto trie = PackedTrie::from_words({});
  EXPECT_EQ(trie.word_count(), 0u);
  EXPECT_EQ(trie.node_count(), 1u);
  EXPECT_FALSE(trie.contains("a"));
  EXPECT_FALSE(trie.has_prefix(""));
}

TEST(PackedTrie, WordsAndPrefixes) {
  const auto trie = PackedTrie::from_words({"tied", "tie", "ties", "hot"});
  EXPECT_TRUE(trie.contains("tie"));
  EXPECT_TRUE(trie.contains("tied"));
  EXPECT_FALSE(trie.contains("ti"));
  EXPECT_TRUE(trie.has_prefix("ti"));
  EXPECT_FALSE(trie.has_prefix("tix"));
  EXPECT_EQ(trie.word_count(), 4u);
  // root + t,i,e + d + s + h,o,t
  EXPECT_EQ(trie.node_count(), 9u);
}

TEST(PackedTrie, NormalizesInput) {
  const auto trie = PackedTrie::from_words({"Sit", "SIT", "sit", "don't", "", "caf\xc3\xa9"});
  EXPECT_EQ(trie.word_count(), 1u);
  EXPECT_TRUE(trie.contains("sit"));
  EXPECT_TRUE(trie.contains("SIT"));
  EXPECT_FALSE(trie.contains("don't"));
}

TEST(PackedTrie, StepByStepWithSparseChildren) {
  // Children of the root are a, m, z: checks the popcount indexing skips absent letters.
  const auto trie = PackedTrie::from_words({"zoo", "ant", "mat"});
  for (const char* word : {"zoo", "ant", "mat"}) {
    EXPECT_TRUE(trie.is_word(trie.walk(PackedTrie::kRoot, word))) << word;
  }
  EXPECT_EQ(trie.child(PackedTrie::kRoot, 'b'), PackedTrie::kNone);
  EXPECT_EQ(trie.child(PackedTrie::kRoot, 'Z'), PackedTrie::kNone);
  EXPECT_EQ(trie.walk(PackedTrie::kRoot, "zoa"), PackedTrie::kNone);
}

// Same words in, same answers out as the baseline Trie, on the full fallback dictionary.
TEST(PackedTrie, MatchesTrieOnEnable) {
  std::ifstream in(BOGZILLA_DICTIONARY);
  std::vector<std::string> words;
  for (std::string word; in >> word;) words.push_back(word);
  ASSERT_GT(words.size(), 100000u);

  Trie trie;
  for (const auto& word : words) trie.insert(word);
  const auto packed = PackedTrie::from_words(words);

  EXPECT_EQ(packed.word_count(), trie.word_count());
  EXPECT_EQ(packed.node_count(), trie.node_count());
  for (const auto& word : words) {
    ASSERT_TRUE(packed.contains(word)) << word;
    // A word plus one letter is mostly a non-word; both tries must agree either way.
    const std::string longer = word + 'q';
    ASSERT_EQ(packed.contains(longer), trie.contains(longer)) << longer;
  }

  for (const auto& fixture :
       bogzilla::testing::load_board_fixtures(BOGZILLA_FIXTURES_DIR "/wordshake_boards.txt")) {
    const auto board = bogzilla::Board::from_tiles(fixture.tiles);
    ASSERT_TRUE(board);
    const auto a = bogzilla::solve(*board, trie);
    const auto b = bogzilla::solve(*board, packed);
    ASSERT_EQ(a.size(), b.size()) << board->to_string();
    for (std::size_t i = 0; i < a.size(); ++i) {
      EXPECT_EQ(a[i].word, b[i].word);
      EXPECT_EQ(a[i].path, b[i].path);
    }
  }
}
