#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bogzilla {

// Nodes sit in one flat vector and point to children by index, not pointer: better memory
// locality, and the solver can track its position as a plain integer.
class Trie {
public:
  using NodeId = std::uint32_t;

  // The root is never a child, so 0 doubles as "no child" and new nodes start zeroed.
  // Since kNone == kRoot, check each step for kNone before stepping again.
  static constexpr NodeId kRoot = 0;
  static constexpr NodeId kNone = 0;
  // Each word ends at its own node, so the solver can dedupe words by node id.
  static constexpr bool kUniqueWordNodes = true;

  Trie();

  // Case-insensitive. False if already present or not purely a-z.
  bool insert(std::string_view word);

  bool contains(std::string_view word) const noexcept;
  bool has_prefix(std::string_view prefix) const noexcept;

  // Expects lowercase a-z.
  NodeId child(NodeId node, char letter) const noexcept;
  // For multi-letter tiles like "qu".
  NodeId walk(NodeId node, std::string_view letters) const noexcept;
  bool is_word(NodeId node) const noexcept;

  std::size_t word_count() const noexcept { return word_count_; }
  std::size_t node_count() const noexcept { return nodes_.size(); }

private:
  struct Node {
    std::array<NodeId, 26> children{};
    bool is_word = false;
  };

  NodeId find(std::string_view text) const noexcept;

  std::vector<Node> nodes_;
  std::size_t word_count_ = 0;
};

}  // namespace bogzilla
