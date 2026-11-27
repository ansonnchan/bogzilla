#include <cstddef>
#include <string>
#include <vector>

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "bogzilla/board.hpp"
#include "bogzilla/solver.hpp"
#include "bogzilla/trie.hpp"

namespace {

using emscripten::val;

// JS: solve(tiles: string[], words: string[], minLength: number)
//       -> { word: string, path: number[] }[] | null (null if the tiles aren't a valid board)
// Takes and returns plain JS arrays/objects so the extension needs no embind wrapper types.
// Builds a fresh trie per call: fine for a site's ~150-word answer list. A large fallback
// dictionary will want a trie that persists between calls.
val solve(val tiles_js, val words_js, int min_length) {
  const auto tiles = emscripten::vecFromJSArray<std::string>(tiles_js);
  const auto board = bogzilla::Board::from_tiles(tiles);
  if (!board) return val::null();

  bogzilla::Trie trie;
  for (const auto& word : emscripten::vecFromJSArray<std::string>(words_js)) trie.insert(word);

  const bogzilla::SolveOptions options{.min_length = static_cast<std::size_t>(min_length)};
  val out = val::array();
  for (const auto& found : bogzilla::solve(*board, trie, options)) {
    val item = val::object();
    item.set("word", found.word);
    item.set("path", val::array(found.path));
    out.call<void>("push", item);
  }
  return out;
}

}  // namespace

EMSCRIPTEN_BINDINGS(bogzilla) {
  emscripten::function("solve", &solve);
}
