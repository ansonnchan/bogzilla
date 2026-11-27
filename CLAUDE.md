# bogzilla

Unofficial Boggle hint assistant as a Chrome extension. C++ solver compiled to WASM.

Read [ARCHITECTURE.md](ARCHITECTURE.md) before making design changes. It holds the decisions,
components, and the per-site page details (selectors). Update it when any of those change.

## Rules

- Hints are paths shaded on the side-panel board, one word at a time. No text clues.
- The extension is read only: never auto-enter words into a game.
- Keep the C++ solver as the core, even on sites that expose their own answer lists.

## Git workflow

- Commit directly to `main`. **No pull requests.**
- **One small piece per commit.** A commit can span several files if they make up one piece
  (e.g. trie header + source + tests), but never mixes two pieces (e.g. trie and solver).
- **Commit subjects are short and informal**: a phrase, not a sentence. e.g. `add dictionary`,
  `trie insert`, `wordshake adapter`.
- **Before every commit, show the user the changed files (and the diff if useful) and wait for
  their go-ahead.** They may want to edit files or add comments first. Never commit without that OK.
