// Bundles the extension into dist/, ready for chrome://extensions "Load unpacked".
// Needs the WASM build first: emcmake cmake -S .. -B ../build-wasm && cmake --build ../build-wasm
import { build } from "esbuild";
import { cpSync, existsSync, mkdirSync, rmSync } from "node:fs";

const wasmDir = "../build-wasm/wasm";
if (!existsSync(`${wasmDir}/bogzilla.wasm`)) {
  console.error(`missing ${wasmDir}/bogzilla.wasm: build the WASM module first`);
  process.exit(1);
}

rmSync("dist", { recursive: true, force: true });
mkdirSync("dist");

await build({
  entryPoints: {
    background: "src/background.ts",
    wordshake: "src/content/wordshake.ts",
    sidepanel: "src/sidepanel/sidepanel.ts",
  },
  outdir: "dist",
  bundle: true,
  // Content scripts can't be ES modules, so everything ships as an IIFE except the side
  // panel, which imports the Emscripten module at runtime.
  format: "iife",
  target: "chrome120",
  logLevel: "info",
});

for (const file of [
  "manifest.json",
  "src/sidepanel/sidepanel.html",
  "src/sidepanel/sidepanel.css",
  "data/frequency.txt",
  "data/NOTICE.md",
]) {
  cpSync(file, `dist/${file.split("/").pop()}`);
}
cpSync(`${wasmDir}/bogzilla.mjs`, "dist/bogzilla.mjs");
cpSync(`${wasmDir}/bogzilla.wasm`, "dist/bogzilla.wasm");
