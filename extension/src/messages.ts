export interface GameState {
  site: string;
  // Row by row, lowercase. Multi-letter tiles like "qu" stay as one entry.
  tiles: string[];
  found: string[];
  // The site's own answer list, or null if the site doesn't expose one.
  answers: string[] | null;
}

export type Message = { type: "state"; state: GameState } | { type: "getState" };
