#include "bogzilla/solver.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <utility>

namespace bogzilla {
namespace {

// Templated so the same search runs on Trie (baseline), PackedTrie and Dawg, which share an API.
template <typename TrieT>
class Search {
public:
  Search(const Board& board, const TrieT& trie, SolveOptions options)
      : board_(board), trie_(trie), options_(options) {
    if constexpr (TrieT::kUniqueWordNodes) seen_nodes_.assign(trie.node_count(), false);
  }

  std::vector<FoundWord> run() {
    for (int tile = 0; tile < board_.tile_count(); ++tile) {
      visit(tile, TrieT::kRoot, 0);
    }
    std::sort(found_.begin(), found_.end(),
              [](const FoundWord& a, const FoundWord& b) { return a.word < b.word; });
    return std::move(found_);
  }

private:
  // Steps onto `tile` from trie node `node`, then tries every unvisited neighbour.
  // Backtracks by undoing its changes to word_ and path_ on the way out. `visited` is a
  // bitmask over tile indices (boards have at most 25 tiles, so it fits in 32 bits).
  // Dedupes words. A per-node flag is cheapest, but only valid when each word has its own
  // end node; a Dawg shares end nodes between words, so it falls back to a set of words.
  bool first_time(typename TrieT::NodeId node) {
    if constexpr (TrieT::kUniqueWordNodes) {
      if (seen_nodes_[node]) return false;
      seen_nodes_[node] = true;
      return true;
    } else {
      return seen_words_.insert(word_).second;
    }
  }

  void visit(int tile, typename TrieT::NodeId node, std::uint32_t visited) {
    const std::string& letters = board_.tile(tile);
    node = trie_.walk(node, letters);
    if (node == TrieT::kNone) return;  // no word starts this way: prune

    visited |= std::uint32_t{1} << tile;
    word_ += letters;
    path_.push_back(tile);

    if (trie_.is_word(node) && word_.size() >= options_.min_length && first_time(node)) {
      found_.push_back({word_, path_});
    }

    for (const std::uint8_t next : board_.neighbors(tile)) {
      if ((visited & (std::uint32_t{1} << next)) == 0) visit(next, node, visited);
    }

    path_.pop_back();
    word_.resize(word_.size() - letters.size());
  }

  const Board& board_;
  const TrieT& trie_;
  SolveOptions options_;
  std::vector<bool> seen_nodes_;
  std::unordered_set<std::string> seen_words_;
  std::string word_;
  std::vector<int> path_;
  std::vector<FoundWord> found_;
};

}  // namespace

std::vector<FoundWord> solve(const Board& board, const Trie& trie, SolveOptions options) {
  return Search<Trie>(board, trie, options).run();
}

std::vector<FoundWord> solve(const Board& board, const PackedTrie& trie, SolveOptions options) {
  return Search<PackedTrie>(board, trie, options).run();
}

std::vector<FoundWord> solve(const Board& board, const Dawg& trie, SolveOptions options) {
  return Search<Dawg>(board, trie, options).run();
}

}  // namespace bogzilla
