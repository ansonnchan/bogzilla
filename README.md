# bogzilla

A monster at finding words.

A fast Boggle solver written in Rust, with a web interface and potentially a Chrome extension. Mainly an excuse to explore algorithms, data structures, and performance optimization.

## Core solver

- Support both 3×3 and 4×4 Boggle boards (potentially 5×5 later).
- Load words from a `dictionary.txt` or an open-source English dictionary.
- Construct a trie for efficient prefix searching.
- Represent the board as an implicit graph.
- Use DFS and backtracking to explore all eight neighboring cells without reusing tiles.
- Prune paths as soon as they no longer form valid prefixes.
- Handle special tiles such as `Qu`.
- Deduplicate discovered words.
- Calculate word scores and sort by length or score.
- Return the coordinates of each discovered word.

## Algorithms & optimizations

- Trie-based prefix searching.
- DFS with backtracking.
- Bitmasking to track visited cells.
- Precomputed adjacency lists.
- Dynamic programming / memoization for repeated DFS states.
- Board-level caching to avoid solving identical boards.
- Explore arena-allocated tries for better memory locality.
- Experiment with parallel traversal.
- Benchmark optimizations individually to see whether they actually improve performance.

## Web app

- Minimalist, clean interface.
- Manually enter or randomly generate a board.
- Switch between 3×3 and 4×4 boards.
- Display all possible words and their scores.
- Click on a word to visualize its path across the board.
- Play timed games and review missed words.
- Display solver statistics (execution time, explored states, pruned branches, cache hits, etc.).

## Chrome extension (potential)

- Detect and extract boards from supported Boggle websites.
- Run the solver locally through WebAssembly.
- Display results in a sidebar or overlay.
- Highlight word paths directly on the board.
- Support website-specific adapters.

## Other ideas

- Find the highest-scoring possible Boggle board using hill climbing or simulated annealing.
- Visualize DFS traversal and trie pruning in real time.
- Compare different solver implementations and their performance.
- Import physical boards from photos using OCR.
- Track frequently missed words and personal performance.

## Tech stack

- Rust — core solver
- WebAssembly — browser integration
- React / TypeScript — web interface
- Tailwind CSS — styling
- Chrome Manifest V3 — browser extension
- Criterion — benchmarking

Everything should run locally, without requiring a backend server.
