import {
  LONGEST_FILTER,
  matchesLength,
  parseFrequencyList,
  pickHint,
  type FrequencyRanks,
  type LengthFilter,
  type Solution,
} from "../hint";
import type { GameState, Message } from "../messages";

interface BogzillaModule {
  solve(tiles: string[], words: string[], minLength: number): Solution[] | null;
}

const $ = <T extends Element>(id: string) => document.getElementById(id) as unknown as T;
const el = {
  board: $<HTMLDivElement>("board"),
  boardWrap: $<HTMLDivElement>("board-wrap"),
  pipe: $<SVGSVGElement>("pipe"),
  progress: $<HTMLParagraphElement>("progress"),
  status: $<HTMLParagraphElement>("status"),
  controls: $<HTMLDivElement>("controls"),
  lengths: $<HTMLDivElement>("lengths"),
  skip: $<HTMLButtonElement>("skip"),
  foundSection: $<HTMLElement>("found-section"),
  foundCount: $<HTMLSpanElement>("found-count"),
  found: $<HTMLUListElement>("found"),
};

const LENGTH_FILTERS: LengthFilter[] = [null, 3, 4, 5, 6, LONGEST_FILTER];

let wasm: BogzillaModule | null = null;
let ranks: FrequencyRanks = new Map();
let game: GameState | null = null;
let solutions: Solution[] = [];
let solvedKey = "";
let current: string | null = null;
const skipped = new Set<string>();
let lengthFilter: LengthFilter = null;

async function loadSolver(): Promise<BogzillaModule> {
  // Loaded at runtime rather than bundled so Emscripten can find bogzilla.wasm next to it.
  const { default: create } = await import(chrome.runtime.getURL("bogzilla.mjs"));
  return create();
}

async function loadRanks(): Promise<FrequencyRanks> {
  const response = await fetch(chrome.runtime.getURL("frequency.txt"));
  return parseFrequencyList(await response.text());
}

// Fallback for sites that don't expose their answers. Loaded only when first needed, since
// sites like Wordshake never use it.
let dictionary: string[] | null = null;
let dictionaryLoading: Promise<void> | null = null;

function ensureDictionary(): void {
  dictionaryLoading ??= fetch(chrome.runtime.getURL("enable1.txt"))
    .then((response) => response.text())
    .then((text) => {
      dictionary = text.split("\n").filter(Boolean);
      update(game);
    })
    .catch(console.error);
}

function update(state: GameState | null): void {
  game = state;
  if (!game || !wasm) return render();

  const words = game.answers ?? dictionary;
  if (!words) {
    ensureDictionary();
    return render();
  }

  // Only re-solve when the board or word list changes, not on every found word.
  const key = JSON.stringify([game.tiles, game.answers]);
  if (key !== solvedKey) {
    solvedKey = key;
    solutions = wasm.solve(game.tiles, words, 3) ?? [];
    current = null;
    skipped.clear();
  }
  render();
}

function render(): void {
  const ready = game !== null && solutions.length > 0;
  el.boardWrap.hidden = !game;
  el.controls.hidden = el.foundSection.hidden = !ready;

  if (!game) {
    el.progress.textContent = "";
    el.status.textContent = "Open a Wordshake Boggle game to get hints.";
    return;
  }

  const found = new Set(game.found);
  const hint = pickHint(solutions, found, current, skipped, ranks, lengthFilter);
  current = hint?.word ?? null;

  const foundWords = solutions.filter((s) => found.has(s.word)).map((s) => s.word);
  el.progress.textContent = solutions.length ? `${foundWords.length} / ${solutions.length}` : "";
  el.status.textContent =
    game.answers === null && dictionary === null
      ? "Loading dictionary…"
      : solutions.length > 0 && foundWords.length === solutions.length
        ? "You found every word!"
        : !hint && ready
          ? "None left at this length."
          : "";

  drawBoard(game.tiles, hint?.path ?? []);
  drawLengths(found);
  el.skip.disabled = !hint;
  drawFound(foundWords);
}

function drawBoard(tiles: string[], path: number[]): void {
  const size = Math.round(Math.sqrt(tiles.length));
  el.board.style.setProperty("--size", String(size));
  el.board.replaceChildren(
    ...tiles.map((letters, i) => {
      const tile = document.createElement("div");
      tile.className = "tile";
      tile.textContent = letters === "qu" ? "Qu" : letters;
      if (path.includes(i)) tile.classList.add("on", ...(i === path[0] ? ["start"] : []));
      return tile;
    }),
  );
  drawPipe(path);
}

// A thick rounded line in the highlight colour runs under the tiles, joining the hint's tiles
// into one shape so the order reads at a glance.
function drawPipe(path: number[]): void {
  el.pipe.replaceChildren();
  if (path.length < 2) return;

  const origin = el.boardWrap.getBoundingClientRect();
  const tiles = el.board.children;
  const first = (tiles[0] as HTMLElement).getBoundingClientRect();
  const points = path.map((i) => {
    const r = (tiles[i] as HTMLElement).getBoundingClientRect();
    return `${r.left - origin.left + r.width / 2},${r.top - origin.top + r.height / 2}`;
  });

  const line = document.createElementNS("http://www.w3.org/2000/svg", "polyline");
  line.setAttribute("points", points.join(" "));
  line.setAttribute("stroke-width", String(first.width * 0.42));
  el.pipe.append(line);
}

function drawLengths(found: ReadonlySet<string>): void {
  el.lengths.replaceChildren(
    ...LENGTH_FILTERS.map((filter) => {
      const left = solutions.filter(
        (s) => !found.has(s.word) && matchesLength(s.word, filter),
      ).length;
      const chip = document.createElement("button");
      chip.type = "button";
      chip.className = "chip";
      chip.textContent =
        filter === null ? "Any" : filter === LONGEST_FILTER ? `${filter}+` : String(filter);
      chip.title = `${left} left`;
      chip.setAttribute("aria-pressed", String(filter === lengthFilter));
      chip.disabled = left === 0 && filter !== lengthFilter;
      chip.addEventListener("click", () => {
        lengthFilter = filter;
        render();
      });
      return chip;
    }),
  );
}

// Longest first: those are the ones worth showing off.
function drawFound(words: string[]): void {
  el.foundCount.textContent = words.length ? `· ${words.length}` : "";
  const sorted = [...words].sort((a, b) => b.length - a.length || a.localeCompare(b));
  if (sorted.length === 0) {
    const empty = document.createElement("li");
    empty.className = "empty";
    empty.textContent = "Nothing yet.";
    el.found.replaceChildren(empty);
    return;
  }
  el.found.replaceChildren(
    ...sorted.map((word) => {
      const li = document.createElement("li");
      li.textContent = word;
      return li;
    }),
  );
}

async function requestState(): Promise<void> {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });
  if (tab?.id === undefined) return update(null);
  try {
    const message: Message = { type: "getState" };
    update((await chrome.tabs.sendMessage(tab.id, message)) as GameState | null);
  } catch {
    update(null); // no content script on this tab
  }
}

el.skip.addEventListener("click", () => {
  if (current) skipped.add(current);
  current = null;
  render();
});

chrome.runtime.onMessage.addListener((message: Message, sender) => {
  if (message.type !== "state" || !sender.tab?.active) return;
  update(message.state);
});
chrome.tabs.onActivated.addListener(() => void requestState());
chrome.tabs.onUpdated.addListener((_id, info, tab) => {
  if (tab.active && info.status === "complete") void requestState();
});
window.addEventListener("resize", () => render());

// Hints still work without the frequency list (shortest first), so its failure isn't fatal.
loadRanks()
  .then((loaded) => {
    ranks = loaded;
    render();
  })
  .catch(console.error);

loadSolver()
  .then((module) => {
    wasm = module;
    return requestState();
  })
  .catch((error: unknown) => {
    console.error(error);
    el.status.textContent = "Couldn't load the solver.";
  });
