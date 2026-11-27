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
- **Hint ordering:** common words first, ranked by a subtitle-based frequency list
  (`extension/data/frequency.txt`, CC BY-SA 4.0, see `data/NOTICE.md`). Words not in the list
  are the obscure ones and go last, shortest first.
- **Hint stability:** a hint stays until it's found or skipped. Finding a different word
  doesn't swap it out.
- **Read only:** never auto-enter words into the game.
- **Keep the C++ solver** even though Wordshake exposes its own answer list. It's the learning
  goal (algorithms and performance), and it supports sites that don't expose answers. On
  Wordshake, the page's answer list decides *which* words count; our solver finds the paths.
  Cross-checking the two doubles as a correctness test.
- **Fallback dictionary: ENABLE** (public domain, `extension/data/enable1.txt`), for sites
  without an answer list. Loaded only when needed. Chosen with `tools/dict_eval` against 20
  saved Wordshake boards (`tests/fixtures/wordshake_boards.txt`):

  | List | Words | Finds game's answers | Extra words / board |
  |---|---|---|---|
  | **ENABLE** | 173k | **97.3%** | **26.6** |
  | dwyl words_alpha | 370k | 97.2% | 123.0 |
  | SCOWL British size 70 | 112k | 88.8% | 10.9 |
  | SCOWL British size 95 | 425k | 94.6% | 119.7 |

  Intersecting ENABLE with the others cut extras by under 10% but lost recall. The Oxford
  definitions dump was ruled out (no plurals or verb forms, copyrighted).

## Components

| Directory | What | Tech |
|---|---|---|
| `core/` | Solver library: trie, DFS + backtracking, bitmask visited set, precomputed adjacency, `Qu` tiles, path coordinates. No I/O. | C++20, CMake |
| `wasm/` | Bindings that compile `core/` to WebAssembly: `solve(tiles, words, minLength)` returns plain JS objects. Built with `-sDYNAMIC_EXECUTION=0` because MV3 forbids `eval`; the manifest allows `wasm-unsafe-eval`. | Emscripten, embind |
| `extension/` | Site adapters (content scripts) read the page and push state on DOM changes; the side panel runs the WASM solver, picks the hint and draws the board. | TypeScript, esbuild, Vitest, Chrome MV3 |
| `tests/`, `bench/` | Unit tests and benchmarks for `core/`; `tests/fixtures/` holds real Wordshake boards. | GoogleTest, Google Benchmark |
| `tools/` | `dict_eval`: scores word lists against the saved boards. | C++ |

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

## Performance

Measured with `build/bench/bogzilla_bench` (Release, Apple Silicon), ENABLE dictionary,
20 saved Wordshake boards. Each optimization from the README should be benchmarked against
this baseline on its own.

| Version | Trie build | Trie nodes | Trie memory | Solve per board |
|---|---|---|---|---|
| Baseline (flat nodes, 26 child slots each, bitmask DFS) | 12.4 ms | 388k | ~42 MB | 45 µs |

Solving is already fast; trie memory is the bottleneck (it all lives in the side panel's WASM
heap when the fallback dictionary is used).
