import { describe, expect, it } from "vitest";
import { orderCandidates, pickHint, type Solution } from "./hint";

const sol = (word: string): Solution => ({ word, path: [] });
const solutions = ["theories", "tie", "hissed", "sit", "ties"].map(sol);
const none = new Set<string>();

describe("orderCandidates", () => {
  it("drops found words and puts shorter words first", () => {
    const ordered = orderCandidates(solutions, new Set(["sit"]));
    expect(ordered.map((s) => s.word)).toEqual(["tie", "ties", "hissed", "theories"]);
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

  it("returns null when every word is found", () => {
    const all = new Set(solutions.map((s) => s.word));
    expect(pickHint(solutions, all, null, none)).toBeNull();
  });
});
