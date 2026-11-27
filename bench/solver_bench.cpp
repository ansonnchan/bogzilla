// Baselines for the optimizations listed in the README. Uses the ENABLE fallback dictionary,
// the biggest trie the extension builds, and the saved Wordshake boards.
// Run a Release build: ./build/bench/bogzilla_bench

#include <fstream>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#include "../tests/fixtures.hpp"
#include "bogzilla/board.hpp"
#include "bogzilla/solver.hpp"
#include "bogzilla/trie.hpp"

namespace {

const std::vector<std::string>& dictionary() {
  static const auto words = [] {
    std::vector<std::string> out;
    std::ifstream in(BOGZILLA_DICTIONARY);
    for (std::string word; in >> word;) out.push_back(word);
    return out;
  }();
  return words;
}

const bogzilla::Trie& dictionary_trie() {
  static const auto trie = [] {
    bogzilla::Trie t;
    for (const auto& word : dictionary()) t.insert(word);
    return t;
  }();
  return trie;
}

const std::vector<bogzilla::Board>& boards() {
  static const auto out = [] {
    std::vector<bogzilla::Board> b;
    for (const auto& f : bogzilla::testing::load_board_fixtures(BOGZILLA_FIXTURES)) {
      b.push_back(*bogzilla::Board::from_tiles(f.tiles));
    }
    return b;
  }();
  return out;
}

void BM_TrieBuild(benchmark::State& state) {
  const auto& words = dictionary();
  std::size_t nodes = 0;
  for (auto _ : state) {
    bogzilla::Trie trie;
    for (const auto& word : words) trie.insert(word);
    nodes = trie.node_count();
    benchmark::DoNotOptimize(trie);
  }
  state.counters["words"] = static_cast<double>(words.size());
  state.counters["nodes"] = static_cast<double>(nodes);
  // Approximate: the node array dominates.
  state.counters["MB"] = static_cast<double>(nodes) * (26 * sizeof(bogzilla::Trie::NodeId) + 4) / 1e6;
}
BENCHMARK(BM_TrieBuild)->Unit(benchmark::kMillisecond);

// One iteration solves all saved boards.
void BM_SolveBoards(benchmark::State& state) {
  const auto& trie = dictionary_trie();
  const auto& all = boards();
  std::size_t words = 0;
  for (auto _ : state) {
    words = 0;
    for (const auto& board : all) words += bogzilla::solve(board, trie).size();
    benchmark::DoNotOptimize(words);
  }
  state.counters["boards"] = static_cast<double>(all.size());
  state.counters["words"] = static_cast<double>(words);
  state.counters["us/board"] = benchmark::Counter(
      static_cast<double>(all.size()), benchmark::Counter::kIsIterationInvariantRate |
                                           benchmark::Counter::kInvert);
}
BENCHMARK(BM_SolveBoards)->Unit(benchmark::kMicrosecond);

}  // namespace
