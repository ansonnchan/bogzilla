# bogzilla

Unofficial Boggle hint assistant as a Chrome extension. C++ solver compiled to WASM.

Read [ARCHITECTURE.md](ARCHITECTURE.md) before making design changes. It holds the decisions,
components, and the per-site page details (selectors). Update it when any of those change.

## Rules

- Hints are paths shaded on the side-panel board, one word at a time. No text clues.
- The extension is read only: never auto-enter words into a game.
- Keep the C++ solver as the core, even on sites that expose their own answer lists.
- Keep comments minimal: only why something is needed, decisions made, and what complex code
  does. No comments restating obvious code.

## Git workflow

- Commit directly to `main`. **No pull requests.**
- **One small piece per commit.** A commit can span several files if they make up one piece
  (e.g. trie header + source + tests), but never mixes two pieces (e.g. trie and solver).
- **Commit subjects are short, informal and descriptive**: a verb plus what changed, not a full
  sentence. e.g. `implement trie`, `add board tests`, `add wordshake adapter`. Not just a noun
  like `trie`.
- **No `Co-Authored-By` trailers** (or any other attribution lines) in commit messages.
- **Before every commit, show the user the changed files (and the diff if useful) and wait for
  their go-ahead.** They may want to edit files or add comments first. Never commit without that OK.
