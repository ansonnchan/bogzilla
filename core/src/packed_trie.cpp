#include "bogzilla/packed_trie.hpp"

#include <algorithm>
#include <bit>
#include <cctype>

namespace bogzilla {
namespace {

bool normalize(std::string& word) {
  if (word.empty()) return false;
  for (char& c : word) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c < 'a' || c > 'z') return false;
  }
  return true;
}

std::size_t letter_index(char c) { return static_cast<std::size_t>(c - 'a'); }

}  // namespace

PackedTrie PackedTrie::from_words(std::vector<std::string> words) {
  std::erase_if(words, [](std::string& w) { return !normalize(w); });
  std::sort(words.begin(), words.end());
  words.erase(std::unique(words.begin(), words.end()), words.end());

  PackedTrie trie;
  trie.word_count_ = words.size();
  trie.nodes_.emplace_back();  // root
  trie.build(kRoot, words, 0, words.size(), 0);
  trie.nodes_.shrink_to_fit();
  return trie;
}

// words[lo, hi) all share the prefix spelled by `node` (length `depth`) and are sorted, so the
// words under each child letter form one contiguous run. Reserve one block for all of this
// node's children first (keeping them adjacent), then fill each child's subtree.
void PackedTrie::build(NodeId node, const std::vector<std::string>& words, std::size_t lo,
                       std::size_t hi, std::size_t depth) {
  if (lo < hi && words[lo].size() == depth) {
    nodes_[node].bits |= kWordBit;
    ++lo;  // sorted, so the word equal to the prefix comes first
  }

  // Two passes (mask, then recurse) instead of collecting runs, to avoid an allocation per
  // node during the build.
  std::uint32_t mask = 0;
  for (std::size_t i = lo; i < hi; ++i) mask |= 1u << letter_index(words[i][depth]);
  if (mask == 0) return;

  const auto first = static_cast<NodeId>(nodes_.size());
  nodes_[node].bits |= mask;
  nodes_[node].first_child = first;
  nodes_.resize(nodes_.size() + static_cast<std::size_t>(std::popcount(mask)));

  NodeId next = first;
  for (std::size_t i = lo; i < hi;) {
    const char c = words[i][depth];
    std::size_t j = i + 1;
    while (j < hi && words[j][depth] == c) ++j;
    build(next++, words, i, j, depth + 1);
    i = j;
  }
}

PackedTrie::NodeId PackedTrie::child(NodeId node, char letter) const noexcept {
  if (letter < 'a' || letter > 'z') return kNone;
  const std::uint32_t bit = 1u << letter_index(letter);
  const Node& n = nodes_[node];
  if ((n.bits & bit) == 0) return kNone;
  return n.first_child + static_cast<NodeId>(std::popcount(n.bits & kLetterMask & (bit - 1)));
}

PackedTrie::NodeId PackedTrie::walk(NodeId node, std::string_view letters) const noexcept {
  for (char c : letters) {
    node = child(node, c);
    if (node == kNone) return kNone;
  }
  return node;
}

PackedTrie::NodeId PackedTrie::find(std::string_view text) const noexcept {
  NodeId node = kRoot;
  for (char c : text) {
    node = child(node, static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    if (node == kNone) return kNone;
  }
  return node;
}

bool PackedTrie::contains(std::string_view word) const noexcept {
  if (word.empty()) return false;
  const NodeId node = find(word);
  return node != kNone && is_word(node);
}

bool PackedTrie::has_prefix(std::string_view prefix) const noexcept {
  if (prefix.empty()) return word_count_ > 0;
  return find(prefix) != kNone;
}

}  // namespace bogzilla
