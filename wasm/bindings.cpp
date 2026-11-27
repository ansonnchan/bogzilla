#include <cstddef>
#include <string>
#include <vector>

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "bogzilla/board.hpp"
#include "bogzilla/packed_trie.hpp"
#include "bogzilla/solver.hpp"

namespace {

using emscripten::val;

// JS: solve(tiles: string[], words: string[], minLength: number)
//       -> { word: string, path: number[] }[] | null (null if the tiles aren't a valid board)
// Takes and returns plain JS arrays/objects so the extension needs no embind wrapper types.
// Builds a fresh trie per call. PackedTrie keeps even the ENABLE fallback at ~3 MB of WASM
// heap; the side panel only calls this when the board or word list changes.
val solve(val tiles_js, val words_js, int min_length) {
  const auto tiles = emscripten::vecFromJSArray<std::string>(tiles_js);
  const auto board = bogzilla::Board::from_tiles(tiles);
  if (!board) return val::null();

  const auto trie =
      bogzilla::PackedTrie::from_words(emscripten::vecFromJSArray<std::string>(words_js));

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
