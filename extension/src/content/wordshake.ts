// Reads https://wordshake.com/boggle. Selectors are documented in ARCHITECTURE.md.
import type { GameState, Message } from "../messages";

function texts(selector: string): string[] {
  return [...document.querySelectorAll<HTMLElement>(selector)].map((el) =>
    (el.textContent ?? "").trim().toLowerCase(),
  );
}

function readState(): GameState | null {
  const tileEls = [...document.querySelectorAll<HTMLElement>("#letter-board .letter span[data-square]")];
  if (tileEls.length === 0) return null;
  tileEls.sort((a, b) => Number(a.dataset.square) - Number(b.dataset.square));

  return {
    site: "wordshake",
    tiles: tileEls.map((el) => (el.textContent ?? "").trim().toLowerCase()),
    found: texts(".word-columns .found-word .word-text"),
    answers: texts(".all-the-words .game-word"),
  };
}

let lastSent = "";

function sendIfChanged(): void {
  const state = readState();
  if (!state) return;
  const key = JSON.stringify(state);
  if (key === lastSent) return;
  lastSent = key;
  const message: Message = { type: "state", state };
  // Rejects when the side panel isn't open; that's expected.
  chrome.runtime.sendMessage(message).catch(() => {});
}

// Event-driven refresh: a correct word or New Game changes the DOM. Mutations come in bursts
// (animations), so read once things settle.
let pending: number | undefined;
const observer = new MutationObserver(() => {
  clearTimeout(pending);
  pending = window.setTimeout(sendIfChanged, 150);
});
observer.observe(document.querySelector("#game-wrap") ?? document.body, {
  childList: true,
  subtree: true,
  characterData: true,
});

chrome.runtime.onMessage.addListener((message: Message, _sender, sendResponse) => {
  if (message.type === "getState") sendResponse(readState());
});

sendIfChanged();
