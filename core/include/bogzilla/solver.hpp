#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "bogzilla/board.hpp"
#include "bogzilla/dawg.hpp"
#include "bogzilla/packed_trie.hpp"
#include "bogzilla/trie.hpp"

namespace bogzilla {

struct FoundWord {
  std::string word;
  std::vector<int> path;  // tile indices in order
};

struct SolveOptions {
  // Counted in letters, so a "qu" tile counts as 2 (classic Boggle rule).
  std::size_t min_length = 3;
};

// Every trie word that can be traced on the board, each once, sorted alphabetically.
// When a word has several paths, the first one found is kept.
std::vector<FoundWord> solve(const Board& board, const Trie& trie, SolveOptions options = {});
std::vector<FoundWord> solve(const Board& board, const PackedTrie& trie, SolveOptions options = {});
std::vector<FoundWord> solve(const Board& board, const Dawg& trie, SolveOptions options = {});

}  // namespace bogzilla
