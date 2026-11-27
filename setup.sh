#!/usr/bin/env bash
# One command to get bogzilla running locally: installs the build tools, builds the WASM solver
# and the Chrome extension, then says how to load it.
#
#   ./setup.sh          install + build
#   ./setup.sh --test   also run the C++ and extension tests
set -euo pipefail

cd "$(dirname "$0")"
run_tests=false
[[ "${1:-}" == "--test" ]] && run_tests=true

step() { printf '\n\033[1;32m==>\033[0m %s\n' "$1"; }
fail() { printf '\n\033[1;31merror:\033[0m %s\n' "$1" >&2; exit 1; }

# --- Tools -----------------------------------------------------------------------------------
# cmake: builds the C++ solver. emscripten: compiles it to WebAssembly. node: builds the extension.
step "Checking build tools"
missing=()
command -v cmake >/dev/null || missing+=(cmake)
command -v emcmake >/dev/null || missing+=(emscripten)
command -v npm >/dev/null || missing+=(node)

if ((${#missing[@]})); then
  if command -v brew >/dev/null; then
    echo "Installing with Homebrew: ${missing[*]} (Emscripten is large, ~1 GB; this can take a while)"
    brew install "${missing[@]}"
  else
    fail "missing ${missing[*]}. Install them (on macOS: https://brew.sh, then rerun this script)."
  fi
else
  echo "cmake, emscripten and node are already installed."
fi

# --- Build -----------------------------------------------------------------------------------
step "Building the WASM solver (first run compiles Emscripten's libraries, so it's slow)"
emcmake cmake -S . -B build-wasm >/dev/null
cmake --build build-wasm

step "Building the extension"
(cd extension && npm ci --no-audit --no-fund && npm run build)

if $run_tests; then
  step "Running C++ tests"
  cmake -S . -B build >/dev/null
  cmake --build build
  ctest --test-dir build --output-on-failure
  node wasm/smoke_test.mjs build-wasm/wasm/bogzilla.mjs

  step "Running extension tests"
  (cd extension && npm test)
fi

# --- Done ------------------------------------------------------------------------------------
step "Done. Load it in Chrome:"
cat <<EOF
  1. Open chrome://extensions
  2. Turn on Developer mode (top right)
  3. Click "Load unpacked" and choose:
       $(pwd)/extension/dist
  4. Open https://wordshake.com/boggle and click the bogzilla icon in the toolbar.

After pulling new changes, rerun ./setup.sh and click the reload icon on bogzilla in
chrome://extensions.
EOF
