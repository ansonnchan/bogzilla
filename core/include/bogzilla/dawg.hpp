#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace bogzilla {

// PackedTrie's 8-byte node layout, but identical subtrees are stored once (a DAWG), so shared
// endings like "-ing" / "-ed" / "-s" aren't repeated for every word.
//
// Built bottom-up: a node's subtree is identical to another's exactly when their records
// (word bit, child mask, first-child index) are equal, because children are merged first.
// Each distinct block of siblings is also stored once.
//
// Many words can end at the same node here, so node ids can't be used to dedupe words.
class Dawg {
public:
  using NodeId = std::uint32_t;

  static constexpr NodeId kRoot = 0;
  static constexpr NodeId kNone = 0;
  static constexpr bool kUniqueWordNodes = false;

  // Case-insensitive; words that aren't purely a-z are skipped, duplicates collapse.
  static Dawg from_words(std::vector<std::string> words);

  bool contains(std::string_view word) const noexcept;
  bool has_prefix(std::string_view prefix) const noexcept;

  // Expects lowercase a-z.
  NodeId child(NodeId node, char letter) const noexcept;
  NodeId walk(NodeId node, std::string_view letters) const noexcept;
  bool is_word(NodeId node) const noexcept { return (nodes_[node].bits & kWordBit) != 0; }

  std::size_t word_count() const noexcept { return word_count_; }
  std::size_t node_count() const noexcept { return nodes_.size(); }

private:
  static constexpr std::uint32_t kWordBit = 1u << 31;
  static constexpr std::uint32_t kLetterMask = (1u << 26) - 1;

  struct Node {
    std::uint32_t bits = 0;  // bit i: has a child for 'a' + i; bit 31: is a word
    NodeId first_child = 0;
  };
  static_assert(sizeof(Node) == 8);

  Dawg() = default;
  NodeId find(std::string_view text) const noexcept;
  Node build(const std::vector<std::string>& words, std::size_t lo, std::size_t hi,
             std::size_t depth);
  NodeId intern_block(const std::vector<Node>& block);

  std::vector<Node> nodes_;
  // Sibling block (raw bytes) -> index of its first node, so repeated blocks are stored once.
  std::unordered_map<std::string, NodeId> blocks_;
  std::size_t word_count_ = 0;
};

}  // namespace bogzilla
