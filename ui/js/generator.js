import { HIGHEST_GUITAR_MIDI, LOWEST_GUITAR_MIDI, MAX_FRET, STRING_MIDI, pcOf, plainNoteName, uniquePcsFromBass } from './music.js';
import { analyseFingering } from './fingering.js';

const MAX_SPAN = 4;
const DEFAULT_LIMIT = 20;
const DROP_WEIGHT = { 7: 1, 2: 2, 5: 3, 9: 3, 8: 3, 10: 4, 11: 4, 6: 4, 1: 4, 3: 5, 4: 5 };

function stringOptions(stringIndex, lo, hi, required) {
  const options = [-1];
  const openPc = pcOf(STRING_MIDI[stringIndex]);
  if (required.has(openPc)) options.push(0);
  for (let fret = lo; fret <= hi; fret++) {
    if (required.has(pcOf(STRING_MIDI[stringIndex] + fret))) options.push(fret);
  }
  return options;
}

function scoreVoicing(frets, fingering, bassOk) {
  const sounded = frets.map((f, i) => (f >= 0 ? i : -1)).filter((i) => i >= 0);
  const first = sounded[0];
  const last = sounded[sounded.length - 1];
  let innerMutes = 0;
  for (let i = first; i <= last; i++) if (frets[i] < 0) innerMutes++;
  const opens = frets.filter((f) => f === 0).length;
  const fretted = frets.filter((f) => f > 0);
  const minFret = fretted.length ? Math.min(...fretted) : 0;
  const maxFret = fretted.length ? Math.max(...fretted) : 0;
  const span = fretted.length ? maxFret - minFret : 0;
  return (
    span * 6 +
    fingering.count * 4 +
    (fingering.barre ? 4 : 0) +
    innerMutes * 10 +
    (6 - sounded.length) * 5 -
    opens * 1.5 +
    minFret * 1.2 +
    (opens > 0 && minFret >= 5 ? 18 : 0) +
    (bassOk ? 0 : 20)
  );
}

function enumerate(required, bassPc, strictBass) {
  const found = new Map();
  const total = required.size;
  const frets = new Array(6).fill(-1);

  function leaf() {
    const sounded = [];
    const covered = new Set();
    for (let s = 0; s < 6; s++) {
      if (frets[s] >= 0) {
        sounded.push(s);
        covered.add(pcOf(STRING_MIDI[s] + frets[s]));
      }
    }
    if (covered.size !== total || sounded.length < 2) return;
    const bassOk = bassPc === null || pcOf(STRING_MIDI[sounded[0]] + frets[sounded[0]]) === bassPc;
    if (strictBass && !bassOk) return;
    const fretted = frets.filter((f) => f > 0);
    if (fretted.length && Math.max(...fretted) - Math.min(...fretted) > MAX_SPAN) return;
    const fingering = analyseFingering(frets);
    if (!fingering.valid) return;
    const key = frets.join(',');
    const score = scoreVoicing(frets, fingering, bassOk);
    const existing = found.get(key);
    if (!existing || existing.score > score) {
      found.set(key, { frets: [...frets], fingering, score, bassOk });
    }
  }

  for (let lo = 1; lo <= MAX_FRET; lo++) {
    const hi = Math.min(MAX_FRET, lo + MAX_SPAN);
    const options = [0, 1, 2, 3, 4, 5].map((s) => stringOptions(s, lo, hi, required));
    const walk = (s) => {
      if (s === 6) return leaf();
      for (const option of options[s]) {
        frets[s] = option;
        walk(s + 1);
      }
      frets[s] = -1;
    };
    walk(0);
  }
  return [...found.values()];
}

function toPosition(entry) {
  const { frets, fingering } = entry;
  const fretted = frets.filter((f) => f > 0);
  const maxFret = fretted.length ? Math.max(...fretted) : 0;
  const minFret = fretted.length ? Math.min(...fretted) : 1;
  const baseFret = maxFret <= 5 ? 1 : minFret;
  const rel = frets.map((f) => (f > 0 ? f - baseFret + 1 : f));
  const midi = [];
  frets.forEach((f, s) => {
    if (f >= 0) midi.push(STRING_MIDI[s] + f);
  });
  return {
    frets: rel,
    fingers: fingering.fingers,
    baseFret,
    barres: fingering.barre ? [fingering.barre.fret - baseFret + 1] : [],
    capo: Boolean(fingering.barre),
    midi,
    score: entry.score,
    bassOk: entry.bassOk
  };
}

function runEnumeration(requiredPcs, bassPc, limit) {
  const required = new Set(requiredPcs);
  const keepBass = bassPc !== null && required.has(bassPc) ? bassPc : null;
  let entries = enumerate(required, keepBass, true);
  if (entries.length === 0) entries = enumerate(required, keepBass, false);
  entries.sort((a, b) => a.score - b.score || a.frets.join().localeCompare(b.frets.join()));
  return entries.slice(0, limit).map(toPosition);
}

function dropCombinations(pcs, bassPc, count) {
  const droppable = pcs.filter((pc) => pc !== bassPc);
  const combos = [];
  const pick = (start, chosen) => {
    if (chosen.length === count) return combos.push([...chosen]);
    for (let i = start; i < droppable.length; i++) {
      chosen.push(droppable[i]);
      pick(i + 1, chosen);
      chosen.pop();
    }
  };
  pick(0, []);
  const weight = (combo) => combo.reduce((sum, pc) => sum + (DROP_WEIGHT[(pc - bassPc + 12) % 12] ?? 5), 0);
  return combos.sort((a, b) => weight(a) - weight(b));
}

function closestPlayable(pcs, bassPc, limit) {
  const maxDrop = Math.min(pcs.length - 2, 6);
  for (let count = 1; count <= maxDrop; count++) {
    for (const dropped of dropCombinations(pcs, bassPc, count)) {
      const kept = pcs.filter((pc) => !dropped.includes(pc));
      const positions = runEnumeration(kept, bassPc, limit);
      if (positions.length) return { positions, missingPcs: dropped };
    }
  }
  return { positions: [], missingPcs: pcs.slice() };
}

export function generateForPcs(pcs, bassPc, limit = DEFAULT_LIMIT) {
  const unique = [...new Set(pcs.map(pcOf))];
  const bass = bassPc === null || bassPc === undefined ? null : pcOf(bassPc);
  if (unique.length === 0) return { status: 'none', positions: [], missingPcs: [], reasons: [] };

  if (unique.length <= 6) {
    const positions = runEnumeration(unique, bass, limit);
    if (positions.length) {
      return { status: 'ok', positions, missingPcs: [], reasons: [], bassHonored: positions[0].bassOk };
    }
  }
  const reasons = [unique.length > 6 ? 'more than 6 distinct notes' : 'unreachable span'];
  const fallback = closestPlayable(unique, bass, limit);
  return {
    status: fallback.positions.length ? 'unplayable' : 'none',
    positions: fallback.positions,
    missingPcs: fallback.missingPcs,
    reasons,
    bassHonored: fallback.positions.length ? fallback.positions[0].bassOk : false
  };
}

export function generateForPressed(midiNotes, limit = DEFAULT_LIMIT) {
  const pcs = uniquePcsFromBass(midiNotes);
  const sorted = [...midiNotes].sort((a, b) => a - b);
  const result = generateForPcs(pcs, pcs[0] ?? null, limit);
  const outOfRange = sorted.length > 0 && (sorted[0] < LOWEST_GUITAR_MIDI || sorted[sorted.length - 1] > HIGHEST_GUITAR_MIDI);
  return { ...result, shifted: outOfRange && result.status === 'ok', outOfRange };
}

export function describeMissing(missingPcs) {
  return missingPcs.map((pc) => plainNoteName(pc, false)).join(' ');
}
