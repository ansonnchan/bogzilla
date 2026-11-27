#include "bogzilla/trie.hpp"

#include <cctype>
#include <optional>

namespace bogzilla {
namespace {

std::optional<std::size_t> slot(char c) {
  const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  if (lower < 'a' || lower > 'z') return std::nullopt;
  return static_cast<std::size_t>(lower - 'a');
}

}  // namespace

Trie::Trie() : nodes_(1) {}

bool Trie::insert(std::string_view word) {
  if (word.empty()) return false;
  // Validate first so a rejected word leaves no partial path behind.
  for (char c : word) {
    if (!slot(c)) return false;
  }

  NodeId node = kRoot;
  for (char c : word) {
    const auto s = *slot(c);
    NodeId next = nodes_[node].children[s];
    if (next == kNone) {
      next = static_cast<NodeId>(nodes_.size());
      nodes_.emplace_back();  // may reallocate, so don't hold references across this
      nodes_[node].children[s] = next;
    }
    node = next;
  }

  if (nodes_[node].is_word) return false;
  nodes_[node].is_word = true;
  ++word_count_;
  return true;
}

Trie::NodeId Trie::find(std::string_view text) const noexcept {
  NodeId node = kRoot;
  for (char c : text) {
    const auto s = slot(c);
    if (!s) return kNone;
    node = nodes_[node].children[*s];
    if (node == kNone) return kNone;
  }
  return node;
}

bool Trie::contains(std::string_view word) const noexcept {
  if (word.empty()) return false;
  const NodeId node = find(word);
  return node != kNone && nodes_[node].is_word;
}

bool Trie::has_prefix(std::string_view prefix) const noexcept {
  if (prefix.empty()) return word_count_ > 0;
  return find(prefix) != kNone;
}

Trie::NodeId Trie::child(NodeId node, char letter) const noexcept {
  if (letter < 'a' || letter > 'z') return kNone;
  return nodes_[node].children[static_cast<std::size_t>(letter - 'a')];
}

Trie::NodeId Trie::walk(NodeId node, std::string_view letters) const noexcept {
  for (char c : letters) {
    node = child(node, c);
    if (node == kNone) return kNone;
  }
  return node;
}

bool Trie::is_word(NodeId node) const noexcept { return nodes_[node].is_word; }

}  // namespace bogzilla
