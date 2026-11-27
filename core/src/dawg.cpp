#include "bogzilla/dawg.hpp"

#include <algorithm>
#include <bit>
#include <cctype>
#include <cstring>

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

Dawg Dawg::from_words(std::vector<std::string> words) {
  std::erase_if(words, [](std::string& w) { return !normalize(w); });
  std::sort(words.begin(), words.end());
  words.erase(std::unique(words.begin(), words.end()), words.end());

  Dawg dawg;
  dawg.word_count_ = words.size();
  dawg.nodes_.emplace_back();  // root slot, so no block starts at 0 (0 means "no child")
  dawg.nodes_[kRoot] = dawg.build(words, 0, words.size(), 0);
  dawg.blocks_ = {};  // only needed while building
  dawg.nodes_.shrink_to_fit();
  return dawg;
}

// Returns the record for the subtree of words[lo, hi) below a prefix of length `depth`.
// Children are built (and merged) first, so equal records mean equal subtrees.
Dawg::Node Dawg::build(const std::vector<std::string>& words, std::size_t lo, std::size_t hi,
                       std::size_t depth) {
  Node node;
  if (lo < hi && words[lo].size() == depth) {
    node.bits |= kWordBit;
    ++lo;
  }

  std::vector<Node> block;
  for (std::size_t i = lo; i < hi;) {
    const char c = words[i][depth];
    std::size_t j = i + 1;
    while (j < hi && words[j][depth] == c) ++j;
    node.bits |= 1u << letter_index(c);
    block.push_back(build(words, i, j, depth + 1));
    i = j;
  }
  if (!block.empty()) node.first_child = intern_block(block);
  return node;
}

Dawg::NodeId Dawg::intern_block(const std::vector<Node>& block) {
  std::string key(block.size() * sizeof(Node), '\0');
  std::memcpy(key.data(), block.data(), key.size());
  const auto [it, inserted] = blocks_.try_emplace(std::move(key), 0);
  if (inserted) {
    it->second = static_cast<NodeId>(nodes_.size());
    nodes_.insert(nodes_.end(), block.begin(), block.end());
  }
  return it->second;
}

Dawg::NodeId Dawg::child(NodeId node, char letter) const noexcept {
  if (letter < 'a' || letter > 'z') return kNone;
  const std::uint32_t bit = 1u << letter_index(letter);
  const Node& n = nodes_[node];
  if ((n.bits & bit) == 0) return kNone;
  return n.first_child + static_cast<NodeId>(std::popcount(n.bits & kLetterMask & (bit - 1)));
}

Dawg::NodeId Dawg::walk(NodeId node, std::string_view letters) const noexcept {
  for (char c : letters) {
    node = child(node, c);
    if (node == kNone) return kNone;
  }
  return node;
}

Dawg::NodeId Dawg::find(std::string_view text) const noexcept {
  NodeId node = kRoot;
  for (char c : text) {
    node = child(node, static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    if (node == kNone) return kNone;
  }
  return node;
}

bool Dawg::contains(std::string_view word) const noexcept {
  if (word.empty()) return false;
  const NodeId node = find(word);
  return node != kNone && is_word(node);
}

bool Dawg::has_prefix(std::string_view prefix) const noexcept {
  if (prefix.empty()) return word_count_ > 0;
  return find(prefix) != kNone;
}

}  // namespace bogzilla
