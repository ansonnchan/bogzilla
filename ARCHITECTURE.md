# Architecture

bogzilla is an unofficial hint assistant for Boggle-style word games, delivered as a
**Chrome extension**. It reads the live game board, works out which words you haven't found
yet, and shows the path of the next one on a copy of the board, one hint at a time.

## Decisions

- **Chrome extension only** (Manifest V3). No terminal UI, no macOS app, no manual board entry.
- **First supported site:** https://wordshake.com/boggle. Other sites get their own adapters later.
- **Hints:** never "a 6-letter word starting with R" style clues. A hint is the word's path shaded
  on the extension's own copy of the board (in Chrome's side panel), with a line through the
  tiles to show the order. One word at a time, never the whole list.
- **Refresh:** event-driven, not polled. Recompute the hint whenever a correct word is accepted
  (a `MutationObserver` on the found-words list). No timers.
- **Hint ordering:** prefer common words over obscure ones.
- **Read only:** never auto-enter words into the game.
- **Keep the C++ solver** even though Wordshake exposes its own answer list. It's the learning
  goal (algorithms and performance), and it supports sites that don't expose answers. On
  Wordshake, the page's answer list decides *which* words count; our solver finds the paths.
  Cross-checking the two doubles as a correctness test.

## Components

| Directory | What | Tech |
|---|---|---|
| `core/` | Solver library: trie, DFS + backtracking, bitmask visited set, precomputed adjacency, `Qu` tiles, path coordinates. No I/O. | C++20, CMake |
| `wasm/` | Bindings that compile `core/` to WebAssembly so the extension can call it. | Emscripten |
| `extension/` | Site adapters (content scripts) read the page; side panel renders the board and hint; runs the WASM solver. | TypeScript, Chrome MV3 |
| `tests/`, `bench/` | Unit tests and benchmarks for `core/`. | GoogleTest, Google Benchmark |

## Data flow

1. A site adapter reads the board and found words from the page.
2. On each new found word, it sends the state to the extension.
3. The solver finds every word and its path; the adapter's answer list (if the site has one)
   filters it.
4. The hint engine picks the next unfound word (most common first).
5. The side panel draws the board and the word's path.

## Site adapters

### Wordshake (verified 2026-11-27)

| Data | Selector | Notes |
|---|---|---|
| Board tiles | `#letter-board .letter span[data-square]` | Squares 0–15, row by row. Lowercase letters. |
| Score | `#score span` | Updates immediately on a correct word. Not classic Boggle scoring (3 letters = 4 pts). |
| Found words | `.word-columns .found-word .word-text` | Appended on each correct word. Observe this for refresh. |
| Per-length counts | `.word-ticker .w-c` (found) / `.total` (all) | |
| All valid answers | `.all-the-words .game-word` | Hidden in the DOM. Words only, no paths. British spellings. |

New Game rebuilds all of the above in place. Words are entered by swiping; keyboard typing
does not register.
