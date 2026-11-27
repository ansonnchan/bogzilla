#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bogzilla {

// Read-only trie at 8 bytes per node (vs ~108 for Trie): ENABLE drops from ~42 MB to ~3 MB.
// Each node stores a 26-bit mask of which letters have a child, plus the index of its first
// child; a node's children sit next to each other in letter order, so the child for a letter
// is first_child + (number of set mask bits below that letter).
//
// Built in one pass from a sorted word list, so the big pointer-style trie never exists.
// Same lookup API as Trie, so the solver works with either.
class PackedTrie {
public:
  using NodeId = std::uint32_t;

  // As in Trie, the root is never a child, so 0 doubles as "no child".
  static constexpr NodeId kRoot = 0;
  static constexpr NodeId kNone = 0;
  static constexpr bool kUniqueWordNodes = true;

  // Case-insensitive; words that aren't purely a-z are skipped, duplicates collapse.
  static PackedTrie from_words(std::vector<std::string> words);

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

  PackedTrie() = default;
  NodeId find(std::string_view text) const noexcept;
  void build(NodeId node, const std::vector<std::string>& words, std::size_t lo, std::size_t hi,
             std::size_t depth);

  std::vector<Node> nodes_;
  std::size_t word_count_ = 0;
};

}  // namespace bogzilla
