// Regenerates data/frequency.txt: one word per line, most common first.
// Source: hermitdave/FrequencyWords (OpenSubtitles 2018), CC-BY-SA-4.0. See data/NOTICE.md.
// Kept to a-z words of 3+ letters, the only ones a hint can ever be.
import { writeFileSync } from "node:fs";

const url =
  "https://raw.githubusercontent.com/hermitdave/FrequencyWords/master/content/2018/en/en_50k.txt";

const response = await fetch(url);
if (!response.ok) throw new Error(`${url}: ${response.status}`);

const seen = new Set();
const words = [];
for (const line of (await response.text()).split("\n")) {
  const word = line.split(" ")[0]?.trim().toLowerCase();
  if (!word || !/^[a-z]{3,}$/.test(word) || seen.has(word)) continue;
  seen.add(word);
  words.push(word);
}

writeFileSync(new URL("../data/frequency.txt", import.meta.url), words.join("\n") + "\n");
console.log(`wrote ${words.length} words`);
