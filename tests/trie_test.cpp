#include "bogzilla/trie.hpp"

#include <sstream>
#include <string>

#include <gtest/gtest.h>

using bogzilla::Trie;

TEST(Trie, StartsEmpty) {
  const Trie trie;
  EXPECT_EQ(trie.word_count(), 0u);
  EXPECT_EQ(trie.node_count(), 1u);
  EXPECT_FALSE(trie.contains("a"));
  EXPECT_FALSE(trie.has_prefix(""));
}

TEST(Trie, InsertAndContains) {
  Trie trie;
  EXPECT_TRUE(trie.insert("tie"));
  EXPECT_TRUE(trie.contains("tie"));
  EXPECT_FALSE(trie.contains("ti"));
  EXPECT_FALSE(trie.contains("ties"));
  EXPECT_FALSE(trie.contains(""));
}

TEST(Trie, PrefixesAreNotWordsUntilInserted) {
  Trie trie;
  trie.insert("tied");
  EXPECT_TRUE(trie.has_prefix("t"));
  EXPECT_TRUE(trie.has_prefix("tie"));
  EXPECT_TRUE(trie.has_prefix("tied"));
  EXPECT_FALSE(trie.has_prefix("tiedx"));
  EXPECT_FALSE(trie.contains("tie"));

  trie.insert("tie");
  EXPECT_TRUE(trie.contains("tie"));
}

TEST(Trie, DuplicatesCountOnce) {
  Trie trie;
  EXPECT_TRUE(trie.insert("sit"));
  EXPECT_FALSE(trie.insert("sit"));
  EXPECT_FALSE(trie.insert("SIT"));
  EXPECT_EQ(trie.word_count(), 1u);
}

TEST(Trie, CaseInsensitive) {
  Trie trie;
  trie.insert("Theories");
  EXPECT_TRUE(trie.contains("theories"));
  EXPECT_TRUE(trie.contains("THEORIES"));
  EXPECT_TRUE(trie.has_prefix("ThEo"));
}

TEST(Trie, RejectsNonLetters) {
  Trie trie;
  EXPECT_FALSE(trie.insert(""));
  EXPECT_FALSE(trie.insert("don't"));
  EXPECT_FALSE(trie.insert("ice cream"));
  EXPECT_FALSE(trie.insert("caf\xc3\xa9"));  // café in UTF-8
  EXPECT_EQ(trie.word_count(), 0u);
  EXPECT_EQ(trie.node_count(), 1u);
  EXPECT_FALSE(trie.contains("don't"));
}

TEST(Trie, SharesPrefixNodes) {
  Trie trie;
  trie.insert("tie");
  trie.insert("tied");
  trie.insert("ties");
  // root + t,i,e + d + s
  EXPECT_EQ(trie.node_count(), 6u);
  EXPECT_EQ(trie.word_count(), 3u);
}

TEST(Trie, StepByStep) {
  Trie trie;
  trie.insert("hot");

  const auto h = trie.child(Trie::kRoot, 'h');
  ASSERT_NE(h, Trie::kNone);
  EXPECT_FALSE(trie.is_word(h));

  const auto ho = trie.child(h, 'o');
  const auto hot = trie.child(ho, 't');
  ASSERT_NE(hot, Trie::kNone);
  EXPECT_TRUE(trie.is_word(hot));

  EXPECT_EQ(trie.child(h, 'x'), Trie::kNone);
  EXPECT_EQ(trie.child(h, 'O'), Trie::kNone);
}

TEST(Trie, WalkFollowsMultiLetterTiles) {
  Trie trie;
  trie.insert("quit");
  const auto qu = trie.walk(Trie::kRoot, "qu");
  ASSERT_NE(qu, Trie::kNone);
  EXPECT_TRUE(trie.is_word(trie.walk(qu, "it")));
  EXPECT_EQ(trie.walk(Trie::kRoot, "qa"), Trie::kNone);
}

// Answers Wordshake listed for the board "otie shis rres logd".
TEST(Trie, LoadsWordshakeAnswers) {
  const std::string answers =
      "tis tie tho the sot sit sis sir she roe rho reg red ore ohs log its ire hot hos hit "
      "his hie her err erg ego tosh tiro tire ties tier tied this stir sits sirs sire shot "
      "shes shed roes rise rhos reis regs rego reds ores ohed ogre lore logs loge ires hots "
      "host hits hiss hire hies hied hers hero heir grit gore goes goer erst errs ergs ergo "
      "tires tired tiers throe thirl these their sties stied sires sired shred shirr shire "
      "shies shier shied sheds roger rises resit other orris ogres loris loges loess ither "
      "hires hired heirs grits gores gored goers toshes tiroes throes thesis theirs shreds "
      "shires series rogers resits others ogress lories hissed either orrises lorries "
      "lorises deities theorise theories";

  Trie trie;
  std::istringstream in(answers);
  std::string word;
  std::size_t count = 0;
  while (in >> word) {
    EXPECT_TRUE(trie.insert(word)) << word;
    ++count;
  }
  EXPECT_EQ(count, 129u);
  EXPECT_EQ(trie.word_count(), 129u);

  std::istringstream again(answers);
  while (again >> word) EXPECT_TRUE(trie.contains(word)) << word;
  EXPECT_FALSE(trie.contains("theoriser"));
}
