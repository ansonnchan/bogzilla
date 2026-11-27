#include "bogzilla/solver.hpp"

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using bogzilla::Board;
using bogzilla::FoundWord;
using bogzilla::solve;
using bogzilla::Trie;

namespace {

Trie make_trie(const std::string& words) {
  Trie trie;
  std::istringstream in(words);
  std::string word;
  while (in >> word) trie.insert(word);
  return trie;
}

std::vector<std::string> words_of(const std::vector<FoundWord>& found) {
  std::vector<std::string> out;
  for (const auto& f : found) out.push_back(f.word);
  return out;
}

// A valid path spells the word, never reuses a tile, and only steps between neighbours.
void expect_valid_path(const Board& board, const FoundWord& found) {
  std::string spelled;
  std::set<int> used;
  for (std::size_t i = 0; i < found.path.size(); ++i) {
    const int tile = found.path[i];
    spelled += board.tile(tile);
    EXPECT_TRUE(used.insert(tile).second) << found.word << " reuses tile " << tile;
    if (i > 0) EXPECT_TRUE(board.adjacent(found.path[i - 1], tile)) << found.word;
  }
  EXPECT_EQ(spelled, found.word);
}

}  // namespace

// abc
// def
// ghi
TEST(Solver, FindsOnlyTraceableWords) {
  const auto board = Board::parse("abcdefghi");
  ASSERT_TRUE(board);
  const auto trie = make_trie("abe aei abcf ace aba ihe");

  const auto found = solve(*board, trie);
  // ace: a and c aren't neighbours. aba: would reuse a.
  EXPECT_EQ(words_of(found), (std::vector<std::string>{"abcf", "abe", "aei", "ihe"}));
  for (const auto& f : found) expect_valid_path(*board, f);
}

TEST(Solver, ReturnsPaths) {
  const auto board = Board::parse("abcdefghi");
  ASSERT_TRUE(board);
  const auto found = solve(*board, make_trie("aei"));
  ASSERT_EQ(found.size(), 1u);
  EXPECT_EQ(found[0].path, (std::vector<int>{0, 4, 8}));
}

TEST(Solver, MinLength) {
  const auto board = Board::parse("abcdefghi");
  ASSERT_TRUE(board);
  const auto trie = make_trie("ab abe");
  EXPECT_EQ(words_of(solve(*board, trie)), (std::vector<std::string>{"abe"}));
  EXPECT_EQ(words_of(solve(*board, trie, {.min_length = 2})),
            (std::vector<std::string>{"ab", "abe"}));
}

TEST(Solver, EachWordOnceDespiteManyPaths) {
  const auto board = Board::parse("aaaaaaaaa");
  ASSERT_TRUE(board);
  const auto found = solve(*board, make_trie("aaa"));
  ASSERT_EQ(found.size(), 1u);
  expect_valid_path(*board, found[0]);
}

TEST(Solver, QuTileCountsAsTwoLetters) {
  // qu i t
  // x  x x
  // x  x x
  const auto board = Board::parse("quitxxxxxx");
  ASSERT_TRUE(board);
  const auto found = solve(*board, make_trie("quit qui"));
  EXPECT_EQ(words_of(found), (std::vector<std::string>{"qui", "quit"}));
  for (const auto& f : found) expect_valid_path(*board, f);
}

TEST(Solver, EmptyTrieFindsNothing) {
  const auto board = Board::parse("abcdefghi");
  ASSERT_TRUE(board);
  EXPECT_TRUE(solve(*board, Trie{}).empty());
}

// Cross-check against a real game: every answer Wordshake listed for this board must be
// traceable, with a valid path.
TEST(Solver, FindsAllWordshakeAnswers) {
  const auto board = Board::parse("otie shis rres logd");
  ASSERT_TRUE(board);
  const auto trie = make_trie(
      "tis tie tho the sot sit sis sir she roe rho reg red ore ohs log its ire hot hos hit "
      "his hie her err erg ego tosh tiro tire ties tier tied this stir sits sirs sire shot "
      "shes shed roes rise rhos reis regs rego reds ores ohed ogre lore logs loge ires hots "
      "host hits hiss hire hies hied hers hero heir grit gore goes goer erst errs ergs ergo "
      "tires tired tiers throe thirl these their sties stied sires sired shred shirr shire "
      "shies shier shied sheds roger rises resit other orris ogres loris loges loess ither "
      "hires hired heirs grits gores gored goers toshes tiroes throes thesis theirs shreds "
      "shires series rogers resits others ogress lories hissed either orrises lorries "
      "lorises deities theorise theories");

  const auto found = solve(*board, trie);
  EXPECT_EQ(found.size(), trie.word_count());
  for (const auto& f : found) expect_valid_path(*board, f);
}
