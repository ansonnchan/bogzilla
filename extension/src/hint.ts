export interface Solution {
  word: string;
  path: number[];
}

// "Common words first" needs a word-frequency list we don't have yet. Until then, shorter
// words stand in for common ones.
export function orderCandidates(solutions: Solution[], found: ReadonlySet<string>): Solution[] {
  return solutions
    .filter((s) => !found.has(s.word))
    .sort((a, b) => a.word.length - b.word.length || a.word.localeCompare(b.word));
}

// Keeps the current hint until it's found or skipped, so finding some other word doesn't
// swap the hint out from under the user. When everything left was skipped, starts over.
export function pickHint(
  solutions: Solution[],
  found: ReadonlySet<string>,
  current: string | null,
  skipped: ReadonlySet<string>,
): Solution | null {
  const candidates = orderCandidates(solutions, found);
  const kept = candidates.find((s) => s.word === current && !skipped.has(s.word));
  if (kept) return kept;
  return candidates.find((s) => !skipped.has(s.word)) ?? candidates[0] ?? null;
}
