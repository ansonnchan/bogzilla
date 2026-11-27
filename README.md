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

- Trie-based prefix searching.
- DFS with backtracking.
- Bitmasking to track visited cells.
- Precomputed adjacency lists.
- Board-level caching to avoid solving identical boards.
- Explore arena-allocated tries for better memory locality.
- Benchmark optimizations individually to see whether they actually improve performance.

## Chrome extension

- Site adapters read the board, found words, and answer list from each supported page.
- Event-driven: the hint updates the moment a correct word is accepted.
- The solver runs locally through WebAssembly.
- Side panel shows the board and the current hint's path.
- Skip a hint, or hide hints while you're on a roll.

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

Everything runs locally, without a backend server. See [ARCHITECTURE.md](ARCHITECTURE.md) for
the design.
