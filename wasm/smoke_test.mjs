// Checks the built WASM module end to end from JS.
// Usage: node wasm/smoke_test.mjs build-wasm/wasm/bogzilla.mjs
import assert from "node:assert/strict";
import path from "node:path";
import { pathToFileURL } from "node:url";

const modulePath = process.argv[2];
assert.ok(modulePath, "pass the path to bogzilla.mjs");
const { default: createBogzilla } = await import(pathToFileURL(path.resolve(modulePath)).href);
const bogzilla = await createBogzilla();

// Wordshake board "otie shis rres logd" with a few of its listed answers.
const tiles = [..."otieshisrreslogd"];
const found = bogzilla.solve(tiles, ["theories", "theorise", "hissed", "tie", "zebra"], 3);

assert.deepEqual(
  found.map((f) => f.word),
  ["hissed", "theories", "theorise", "tie"],
);
for (const { word, path: tilePath } of found) {
  assert.equal(tilePath.map((i) => tiles[i]).join(""), word);
}

assert.equal(bogzilla.solve(["a", "b"], ["ab"], 3), null);

console.log(`ok: ${found.length} words`);
