const STRING_COUNT = 6;

function assignRest(rest, firstFinger, fingers) {
  rest.sort((a, b) => a.fret - b.fret || a.string - b.string);
  let next = firstFinger;
  for (const note of rest) {
    if (next > 4) return false;
    fingers[note.string] = next++;
  }
  return true;
}

function tryBarre(frets, fretted, minFret) {
  const atMin = fretted.filter((n) => n.fret === minFret).map((n) => n.string);
  if (atMin.length < 2) return null;
  const from = Math.min(...atMin);
  const to = Math.max(...atMin);
  for (let s = from; s <= to; s++) {
    if (frets[s] < minFret) return null;
  }
  const fingers = new Array(STRING_COUNT).fill(0);
  for (const s of atMin) fingers[s] = 1;
  const rest = fretted.filter((n) => n.fret > minFret);
  if (!assignRest(rest, 2, fingers)) return null;
  return { fingers, barre: { fret: minFret, from, to }, count: 1 + rest.length };
}

export function analyseFingering(frets) {
  const fretted = [];
  frets.forEach((fret, string) => {
    if (fret > 0) fretted.push({ string, fret });
  });
  if (fretted.length === 0) {
    return { valid: true, fingers: new Array(STRING_COUNT).fill(0), barre: null, count: 0 };
  }
  const minFret = Math.min(...fretted.map((n) => n.fret));

  const plainFingers = new Array(STRING_COUNT).fill(0);
  if (assignRest(fretted.map((n) => ({ ...n })), 1, plainFingers)) {
    return { valid: true, fingers: plainFingers, barre: null, count: fretted.length };
  }

  const barred = tryBarre(frets, fretted, minFret);
  if (barred) return { valid: true, ...barred };
  return { valid: false, fingers: plainFingers, barre: null, count: 99 };
}
