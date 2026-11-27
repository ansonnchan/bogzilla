export interface Solution {
  word: string;
  path: number[];
}

// Word -> rank, 0 = most common.
export type FrequencyRanks = ReadonlyMap<string, number>;

export function parseFrequencyList(text: string): FrequencyRanks {
  const ranks = new Map<string, number>();
  for (const word of text.split("\n")) {
    if (word && !ranks.has(word)) ranks.set(word, ranks.size);
  }
  return ranks;
}

// Common words first. Words missing from the frequency list are the obscure ones, so they
// go last, shortest first.
export function orderCandidates(
  solutions: Solution[],
  found: ReadonlySet<string>,
  ranks: FrequencyRanks = new Map(),
): Solution[] {
  const rank = (word: string) => ranks.get(word) ?? Infinity;
  return solutions
    .filter((s) => !found.has(s.word))
    .sort(
      (a, b) =>
        rank(a.word) - rank(b.word) ||
        a.word.length - b.word.length ||
        a.word.localeCompare(b.word),
    );
}

// Keeps the current hint until it's found or skipped, so finding some other word doesn't
// swap the hint out from under the user. When everything left was skipped, starts over.
export function pickHint(
  solutions: Solution[],
  found: ReadonlySet<string>,
  current: string | null,
  skipped: ReadonlySet<string>,
  ranks?: FrequencyRanks,
): Solution | null {
  const candidates = orderCandidates(solutions, found, ranks);
  const kept = candidates.find((s) => s.word === current && !skipped.has(s.word));
  if (kept) return kept;
  return candidates.find((s) => !skipped.has(s.word)) ?? candidates[0] ?? null;
}
