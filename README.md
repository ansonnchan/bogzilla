# bogzilla

A monster at finding words.

An unofficial hint assistant for Boggle-style word games, built as a Chrome extension with a
fast C++ solver compiled to WebAssembly. Mainly an excuse to explore algorithms, data
structures, and performance optimization.

Not affiliated with Hasbro. Boggle is a trademark of Hasbro.

## How it works

- Open a supported game (starting with [Wordshake Boggle](https://wordshake.com/boggle)).
- The extension reads the board and the words you've already found, straight from the page.
- Chrome's side panel shows a copy of the board with the path of **one** word you haven't found
  yet, shaded and traced in order.
- Find that word and the hint moves to the next one automatically. No typing, no screenshots,
  no timers.
- Common words come first, obscure ones last.
- It only reads the game. It never enters words for you.

## Core solver

- Support both 3×3 and 4×4 boards (potentially 5×5 later).
- Construct a trie for efficient prefix searching.
- Represent the board as an implicit graph.
- Use DFS and backtracking to explore all eight neighboring cells without reusing tiles.
- Prune paths as soon as they no longer form valid prefixes.
- Handle special tiles such as `Qu`.
- Deduplicate discovered words.
- Return the path (tile coordinates) of each discovered word.
- Cross-check results against the site's own answer list where one is available.

## Algorithms & optimizations

- [x] Trie-based prefix searching.
- [x] DFS with backtracking.
- [x] Bitmasking to track visited cells.
- [x] Precomputed adjacency lists.
- [x] Board-level caching to avoid solving identical boards (the side panel re-solves only
  when the board or word list changes).
- [x] Compact trie for better memory: `PackedTrie`, 8-byte nodes in one array, 13.5x less
  memory than the baseline at the same speed.
- [x] Benchmark optimizations individually (`bench/`, results in ARCHITECTURE.md).
- [x] Share common word endings too: `Dawg`, another 3.4x smaller (0.9 MB for ENABLE), at the
  cost of a slower build, so the extension keeps `PackedTrie`.

## Chrome extension

- Site adapters read the board, found words, and answer list from each supported page.
- Event-driven: the hint updates the moment a correct word is accepted.
- The solver runs locally through WebAssembly.
- Side panel shows the board and the current hint's path.
- Filter hints by word length (3, 4, 5, 6, 7+), or ask for another word.
- See the words you've found so far.

## Other ideas

- Support more Boggle-style sites.
- Find the highest-scoring possible board using hill climbing or simulated annealing.
- Visualize DFS traversal and trie pruning in real time.
- Track frequently missed words over time.

## Tech stack

- C++20 + CMake: core solver
- Emscripten / WebAssembly: running the solver in the browser
- TypeScript: extension
- Chrome Manifest V3: browser extension (side panel)
- GoogleTest: unit tests
- Google Benchmark: benchmarking

## Running it locally

On a Mac with [Homebrew](https://brew.sh):

```bash
git clone https://github.com/ansonnchan/bogzilla.git && cd bogzilla && ./setup.sh
```

`setup.sh` installs anything missing (CMake, Emscripten, Node), builds the WASM solver and the
extension, then prints where to load it from. Add `--test` to also run every test.

Then in Chrome: open `chrome://extensions`, turn on **Developer mode**, click **Load unpacked**
and choose `extension/dist`. Open a game on wordshake.com/boggle and click the bogzilla icon.
An unpacked extension stays installed across restarts; no Chrome Web Store needed.

After pulling changes, rerun `./setup.sh` and click the reload icon on bogzilla.

### Building pieces by hand

```bash
# Native build + C++ tests + benchmarks
cmake -S . -B build && cmake --build build && ctest --test-dir build
./build/bench/bogzilla_bench

# WASM module (+ smoke test)
emcmake cmake -S . -B build-wasm && cmake --build build-wasm
node wasm/smoke_test.mjs build-wasm/wasm/bogzilla.mjs

# Extension
cd extension && npm install && npm test && npm run build
```

Everything runs locally, without a backend server. See [ARCHITECTURE.md](ARCHITECTURE.md) for
the design.
