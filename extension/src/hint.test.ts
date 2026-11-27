import { describe, expect, it } from "vitest";
import {
  matchesLength,
  orderCandidates,
  parseFrequencyList,
  pickHint,
  type Solution,
} from "./hint";

const sol = (word: string): Solution => ({ word, path: [] });
const solutions = ["theories", "tie", "hissed", "sit", "ties"].map(sol);
const none = new Set<string>();

describe("orderCandidates", () => {
  it("drops found words and puts shorter words first", () => {
    const ordered = orderCandidates(solutions, new Set(["sit"]));
    expect(ordered.map((s) => s.word)).toEqual(["tie", "ties", "hissed", "theories"]);
  });

  it("puts common words first and unranked words last", () => {
    const ranks = parseFrequencyList("sit\ntheories\ntie\n");
    const ordered = orderCandidates(solutions, none, ranks);
    expect(ordered.map((s) => s.word)).toEqual(["sit", "theories", "tie", "ties", "hissed"]);
  });
});

describe("parseFrequencyList", () => {
  it("ranks by line, ignoring blanks and repeats", () => {
    const ranks = parseFrequencyList("the\nthis\n\nthe\nshe\n");
    expect([...ranks]).toEqual([
      ["the", 0],
      ["this", 1],
      ["she", 2],
    ]);
  });
});

describe("pickHint", () => {
  it("starts with the first candidate", () => {
    expect(pickHint(solutions, none, null, none)?.word).toBe("sit");
  });

  it("keeps the current hint while it's still unfound", () => {
    expect(pickHint(solutions, new Set(["sit"]), "hissed", none)?.word).toBe("hissed");
  });

  it("moves on once the current hint is found", () => {
    expect(pickHint(solutions, new Set(["sit"]), "sit", none)?.word).toBe("tie");
  });

  it("skips skipped words", () => {
    expect(pickHint(solutions, none, "sit", new Set(["sit"]))?.word).toBe("tie");
  });

  it("starts over when everything left was skipped", () => {
    const all = new Set(solutions.map((s) => s.word));
    expect(pickHint(solutions, none, null, all)?.word).toBe("sit");
  });

  it("uses frequency ranks when given", () => {
    const ranks = parseFrequencyList("hissed\n");
    expect(pickHint(solutions, none, null, none, ranks)?.word).toBe("hissed");
  });

  it("only picks words of the chosen length", () => {
    expect(pickHint(solutions, none, null, none, undefined, 4)?.word).toBe("ties");
    expect(pickHint(solutions, none, null, none, undefined, 6)?.word).toBe("hissed");
    expect(pickHint(solutions, none, null, none, undefined, 5)).toBeNull();
  });

  it("drops the current hint when it doesn't match the filter", () => {
    expect(pickHint(solutions, none, "sit", none, undefined, 4)?.word).toBe("ties");
  });

  it("returns null when every word is found", () => {
    const all = new Set(solutions.map((s) => s.word));
    expect(pickHint(solutions, all, null, none)).toBeNull();
  });
});

describe("matchesLength", () => {
  it("treats the last filter as that length or longer", () => {
    expect(matchesLength("theories", 7)).toBe(true);
    expect(matchesLength("theorise", 7)).toBe(true);
    expect(matchesLength("hissed", 7)).toBe(false);
    expect(matchesLength("tie", null)).toBe(true);
    expect(matchesLength("tie", 3)).toBe(true);
    expect(matchesLength("ties", 3)).toBe(false);
  });
});
