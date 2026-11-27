import { pickHint, type Solution } from "../hint";
import type { GameState, Message } from "../messages";

interface BogzillaModule {
  solve(tiles: string[], words: string[], minLength: number): Solution[] | null;
}

const el = {
  board: document.getElementById("board") as HTMLDivElement,
  boardWrap: document.getElementById("board-wrap") as HTMLDivElement,
  trace: document.getElementById("trace") as unknown as SVGSVGElement,
  progress: document.getElementById("progress") as HTMLParagraphElement,
  status: document.getElementById("status") as HTMLParagraphElement,
  skip: document.getElementById("skip") as HTMLButtonElement,
  hide: document.getElementById("hide") as HTMLButtonElement,
};

let wasm: BogzillaModule | null = null;
let game: GameState | null = null;
let solutions: Solution[] = [];
let solvedKey = "";
let current: string | null = null;
const skipped = new Set<string>();
let hidden = false;

async function loadSolver(): Promise<BogzillaModule> {
  // Loaded at runtime rather than bundled so Emscripten can find bogzilla.wasm next to it.
  const { default: create } = await import(chrome.runtime.getURL("bogzilla.mjs"));
  return create();
}

function update(state: GameState | null): void {
  game = state;
  if (!game || !wasm) return render();

  // Only re-solve when the board or word list changes, not on every found word.
  const key = JSON.stringify([game.tiles, game.answers]);
  if (key !== solvedKey) {
    solvedKey = key;
    solutions = wasm.solve(game.tiles, game.answers ?? [], 3) ?? [];
    current = null;
    skipped.clear();
  }
  render();
}

function render(): void {
  if (!game) {
    el.boardWrap.hidden = true;
    el.progress.textContent = "";
    el.status.textContent = "Open a Wordshake Boggle game to get hints.";
    el.skip.disabled = el.hide.disabled = true;
    return;
  }

  const found = new Set(game.found);
  const hint = pickHint(solutions, found, current, skipped);
  current = hint?.word ?? null;

  const foundCount = solutions.filter((s) => found.has(s.word)).length;
  el.progress.textContent = solutions.length ? `${foundCount} / ${solutions.length} found` : "";
  el.status.textContent =
    game.answers === null
      ? "No word list for this site yet."
      : !hint
        ? "You found every word!"
        : hidden
          ? "Hint hidden."
          : "";
  el.skip.disabled = !hint || hidden;
  el.hide.disabled = !hint;
  el.hide.textContent = hidden ? "Show hint" : "Hide hint";
  el.hide.setAttribute("aria-pressed", String(hidden));

  drawBoard(game.tiles, hidden ? [] : (hint?.path ?? []));
}

function drawBoard(tiles: string[], path: number[]): void {
  const size = Math.round(Math.sqrt(tiles.length));
  el.boardWrap.hidden = false;
  el.board.style.setProperty("--size", String(size));
  el.board.replaceChildren(
    ...tiles.map((letters, i) => {
      const tile = document.createElement("div");
      tile.className = "tile";
      tile.textContent = letters === "qu" ? "Qu" : letters;
      if (path.includes(i)) tile.classList.add(i === path[0] ? "start" : "hint");
      return tile;
    }),
  );
  drawTrace(path);
}

// Draws the line through the hint's tiles so the reading order is clear.
function drawTrace(path: number[]): void {
  el.trace.replaceChildren();
  if (path.length === 0) return;

  const origin = el.boardWrap.getBoundingClientRect();
  const tiles = el.board.children;
  const points = path.map((i) => {
    const r = (tiles[i] as HTMLElement).getBoundingClientRect();
    return [r.left - origin.left + r.width / 2, r.top - origin.top + r.height / 2] as const;
  });

  const ns = "http://www.w3.org/2000/svg";
  const line = document.createElementNS(ns, "polyline");
  line.setAttribute("points", points.map(([x, y]) => `${x},${y}`).join(" "));
  el.trace.append(line);
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

el.hide.addEventListener("click", () => {
  hidden = !hidden;
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

loadSolver()
  .then((module) => {
    wasm = module;
    return requestState();
  })
  .catch((error: unknown) => {
    console.error(error);
    el.status.textContent = "Couldn't load the solver.";
  });
