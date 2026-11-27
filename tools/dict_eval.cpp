// Scores word lists as a fallback dictionary: solve every saved Wordshake board with the list
// and compare against the answers the game accepted.
//
// Usage: dict_eval <fixtures.txt> <wordlist.txt>...
//   recall: share of the game's answers the list finds (want 100%)
//   extra:  words the list finds that the game doesn't accept, i.e. bad hints (want few)

#include <chrono>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../tests/fixtures.hpp"
#include "bogzilla/board.hpp"
#include "bogzilla/solver.hpp"
#include "bogzilla/trie.hpp"

int main(int argc, char** argv) {
  if (argc < 3) {
    std::fprintf(stderr, "usage: %s <fixtures.txt> <wordlist.txt>...\n", argv[0]);
    return 1;
  }
  const auto fixtures = bogzilla::testing::load_board_fixtures(argv[1]);

  std::printf("%-28s %8s %8s %8s %10s %9s\n", "list", "words", "recall", "extra", "extra/brd",
              "solve ms");
  for (int arg = 2; arg < argc; ++arg) {
    bogzilla::Trie trie;
    std::ifstream in(argv[arg]);
    for (std::string word; in >> word;) trie.insert(word);

    std::size_t answers = 0, hits = 0, extra = 0;
    std::map<std::string, int> missing;
    double solve_ms = 0;
    for (const auto& fixture : fixtures) {
      const auto board = bogzilla::Board::from_tiles(fixture.tiles);
      const std::set<std::string> accepted(fixture.answers.begin(), fixture.answers.end());

      const auto start = std::chrono::steady_clock::now();
      const auto found = bogzilla::solve(*board, trie);
      solve_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

      std::set<std::string> found_words;
      for (const auto& f : found) found_words.insert(f.word);
      answers += accepted.size();
      for (const auto& w : accepted) {
        if (found_words.count(w)) ++hits;
        else ++missing[w];
      }
      for (const auto& w : found_words) extra += accepted.count(w) ? 0 : 1;
    }

    std::string name = argv[arg];
    name = name.substr(name.find_last_of('/') + 1);
    std::printf("%-28s %8zu %7.1f%% %8zu %10.1f %9.2f\n", name.c_str(), trie.word_count(),
                100.0 * static_cast<double>(hits) / static_cast<double>(answers), extra,
                static_cast<double>(extra) / static_cast<double>(fixtures.size()), solve_ms);
    std::string sample;
    int shown = 0;
    for (const auto& [w, n] : missing) {
      if (shown++ == 12) break;
      sample += w + " ";
    }
    if (!missing.empty()) std::printf("    missing e.g.: %s\n", sample.c_str());
  }
  return 0;
}
