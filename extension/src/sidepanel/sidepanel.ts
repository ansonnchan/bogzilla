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
  arrows: $<SVGSVGElement>("arrows"),
  progress: $<HTMLParagraphElement>("progress"),
  status: $<HTMLParagraphElement>("status"),
  controls: $<HTMLDivElement>("controls"),
  lengths: $<HTMLDivElement>("lengths"),
  skip: $<HTMLButtonElement>("skip"),
  foundSection: $<HTMLElement>("found-section"),
  foundCount: $<HTMLSpanElement>("found-count"),
  found: $<HTMLDivElement>("found"),
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
  drawFound(found);
}

// Shade for step `i` of a path: darkest at the first tile, lightest at the last, so the
// direction reads even without the arrows.
function stepColor(i: number, length: number): string {
  const t = length > 1 ? i / (length - 1) : 0;
  return `color-mix(in srgb, var(--accent-light) ${Math.round(t * 100)}%, var(--accent-strong))`;
}

function drawBoard(tiles: string[], path: number[]): void {
  const size = Math.round(Math.sqrt(tiles.length));
  el.board.style.setProperty("--size", String(size));
  el.board.replaceChildren(
    ...tiles.map((letters, i) => {
      const tile = document.createElement("div");
      tile.className = "tile";
      tile.textContent = letters === "qu" ? "Qu" : letters;
      const step = path.indexOf(i);
      if (step >= 0) {
        tile.classList.add("on");
        tile.style.background = stepColor(step, path.length);
      }
      return tile;
    }),
  );
  drawPath(path);
}

// A thick rounded pipe runs under the tiles, joining the hint into one shape; small arrows
// on top of each joint point the way to go.
function drawPath(path: number[]): void {
  el.pipe.replaceChildren();
  el.arrows.replaceChildren();
  if (path.length < 2) return;

  const origin = el.boardWrap.getBoundingClientRect();
  const tiles = el.board.children;
  const width = (tiles[0] as HTMLElement).getBoundingClientRect().width;
  const centers = path.map((i) => {
    const r = (tiles[i] as HTMLElement).getBoundingClientRect();
    return { x: r.left - origin.left + r.width / 2, y: r.top - origin.top + r.height / 2 };
  });

  const ns = "http://www.w3.org/2000/svg";
  const arm = width * 0.11;
  const placed: { x: number; y: number }[] = [];
  for (let k = 1; k < centers.length; k++) {
    const a = centers[k - 1]!;
    const b = centers[k]!;

    const segment = document.createElementNS(ns, "line");
    segment.setAttribute("x1", String(a.x));
    segment.setAttribute("y1", String(a.y));
    segment.setAttribute("x2", String(b.x));
    segment.setAttribute("y2", String(b.y));
    segment.setAttribute("stroke-width", String(width * 0.42));
    segment.style.stroke = stepColor(k - 0.5, path.length);
    el.pipe.append(segment);

    // Arrowhead pointing from a to b, at the joint's midpoint. When the path crosses itself
    // (an X between two diagonals) two midpoints coincide, so slide the later arrow along
    // its own segment until it's clear.
    const spot =
      [0.5, 0.3, 0.7]
        .map((t) => ({ x: a.x + (b.x - a.x) * t, y: a.y + (b.y - a.y) * t }))
        .find((p) => placed.every((q) => Math.hypot(p.x - q.x, p.y - q.y) > arm * 2.2)) ??
      { x: (a.x + b.x) / 2, y: (a.y + b.y) / 2 };
    placed.push(spot);
    const angle = (Math.atan2(b.y - a.y, b.x - a.x) * 180) / Math.PI;
    const arrow = document.createElementNS(ns, "path");
    arrow.setAttribute("d", `M ${arm} 0 L ${-arm * 0.7} ${-arm * 0.85} L ${-arm * 0.7} ${arm * 0.85} Z`);
    arrow.setAttribute("stroke-width", String(Math.max(1.5, width * 0.035)));
    arrow.setAttribute(
      "transform",
      `translate(${spot.x} ${spot.y}) rotate(${angle})`,
    );
    el.arrows.append(arrow);
  }
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

// One column per word length (3, 4, 5, 6, 7+), like Wordshake: found / total in the header,
// found words running down underneath.
function drawFound(found: ReadonlySet<string>): void {
  const lengths = LENGTH_FILTERS.filter((f): f is number => f !== null);
  const columns = lengths
    .map((length) => {
      const words = solutions.filter((s) => matchesLength(s.word, length)).map((s) => s.word);
      return { length, total: words.length, found: words.filter((w) => found.has(w)).sort() };
    })
    .filter((c) => c.total > 0);

  const foundTotal = columns.reduce((n, c) => n + c.found.length, 0);
  el.foundCount.textContent = foundTotal ? `· ${foundTotal}` : "";
  el.found.style.setProperty("--columns", String(columns.length));
  el.found.replaceChildren(
    ...columns.map((c) => {
      const column = document.createElement("div");
      column.className = "column";
      const heading = document.createElement("h3");
      heading.textContent = c.length === LONGEST_FILTER ? `${c.length}+` : String(c.length);
      const count = document.createElement("span");
      count.textContent = `${c.found.length}/${c.total}`;
      heading.append(count);
      const list = document.createElement("ul");
      list.append(
        ...c.found.map((word) => {
          const li = document.createElement("li");
          li.textContent = word;
          return li;
        }),
      );
      column.append(heading, list);
      return column;
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
