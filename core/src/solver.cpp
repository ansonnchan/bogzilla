#include "bogzilla/solver.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

namespace bogzilla {
namespace {

class Search {
public:
  Search(const Board& board, const Trie& trie, SolveOptions options)
      : board_(board), trie_(trie), options_(options), seen_(trie.node_count(), false) {}

  std::vector<FoundWord> run() {
    for (int tile = 0; tile < board_.tile_count(); ++tile) {
      visit(tile, Trie::kRoot, 0);
    }
    std::sort(found_.begin(), found_.end(),
              [](const FoundWord& a, const FoundWord& b) { return a.word < b.word; });
    return std::move(found_);
  }

private:
  // Steps onto `tile` from trie node `node`, then tries every unvisited neighbour.
  // Backtracks by undoing its changes to word_ and path_ on the way out. `visited` is a
  // bitmask over tile indices (boards have at most 25 tiles, so it fits in 32 bits).
  void visit(int tile, Trie::NodeId node, std::uint32_t visited) {
    const std::string& letters = board_.tile(tile);
    node = trie_.walk(node, letters);
    if (node == Trie::kNone) return;  // no word starts this way: prune

    visited |= std::uint32_t{1} << tile;
    word_ += letters;
    path_.push_back(tile);

    if (trie_.is_word(node) && !seen_[node] && word_.size() >= options_.min_length) {
      seen_[node] = true;
      found_.push_back({word_, path_});
    }

    for (const std::uint8_t next : board_.neighbors(tile)) {
      if ((visited & (std::uint32_t{1} << next)) == 0) visit(next, node, visited);
    }

    path_.pop_back();
    word_.resize(word_.size() - letters.size());
  }

  const Board& board_;
  const Trie& trie_;
  SolveOptions options_;
  // Indexed by trie node: each word ends at exactly one node, so this dedupes words.
  std::vector<bool> seen_;
  std::string word_;
  std::vector<int> path_;
  std::vector<FoundWord> found_;
};

}  // namespace

std::vector<FoundWord> solve(const Board& board, const Trie& trie, SolveOptions options) {
  return Search(board, trie, options).run();
}

}  // namespace bogzilla
