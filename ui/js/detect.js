import { findChordType, isMinorish } from './chordTypes.js';
import { altSpelling, chordNoteName, pcOf, plainNoteName, spellPcs, uniquePcsFromBass } from './music.js';

function spellingCandidates(pcs) {
  const schemes = [spellPcs(pcs, 'flats'), spellPcs(pcs, 'sharps')];
  const accidentals = pcs.filter((pc) => [1, 3, 6, 8, 10].includes(pcOf(pc)));
  if (accidentals.length > 1 && accidentals.length <= 5) {
    const total = 1 << accidentals.length;
    for (let mask = 0; mask < total; mask++) {
      const choice = new Map();
      accidentals.forEach((pc, i) => choice.set(pc, altSpelling(pc)[(mask >> i) & 1]));
      schemes.push(pcs.map((pc) => choice.get(pc) || spellPcs([pc], 'flats')[0]));
    }
  }
  return schemes;
}

export function createDetector(Tonal) {
  function tryScheme(spelled) {
    const symbols = Tonal.Chord.detect(spelled);
    for (const symbol of symbols) {
      const full = Tonal.Chord.get(symbol);
      if (full.empty) continue;
      const rootPosition = full.bass ? Tonal.Chord.get(symbol.slice(0, symbol.lastIndexOf('/'))) : full;
      if (rootPosition.empty) continue;
      const chord = { tonic: full.tonic, bass: full.bass };
      const intervals = rootPosition.intervals.join(' ');
      const type = findChordType(intervals);
      if (!type) continue;
      const rootPc = Tonal.Note.chroma(chord.tonic);
      const bassNote = chord.bass || '';
      const bassPc = bassNote ? Tonal.Note.chroma(bassNote) : null;
      return { type, intervals, rootPc, bassPc };
    }
    return null;
  }

  function literal(pcs, midiNotes) {
    const sorted = [...midiNotes].sort((a, b) => a - b);
    const names = [];
    for (const m of sorted) {
      const n = plainNoteName(m, false);
      if (!names.includes(n)) names.push(n);
    }
    return { ok: false, name: names.join(' '), notes: names, pcs, rootPc: null, bassPc: pcs[0] ?? null };
  }

  function detect(midiNotes) {
    const pcs = uniquePcsFromBass(midiNotes);
    if (pcs.length < 2) return literal(pcs, midiNotes);

    for (const spelled of spellingCandidates(pcs)) {
      const hit = tryScheme(spelled);
      if (!hit) continue;
      const minorish = isMinorish(hit.intervals);
      const root = chordNoteName(hit.rootPc, minorish);
      const bass = hit.bassPc === null ? null : chordNoteName(hit.bassPc, minorish);
      const name = root + hit.type.display + (bass ? '/' + bass : '');
      return {
        ok: true,
        name,
        root,
        bass,
        rootPc: hit.rootPc,
        bassPc: hit.bassPc,
        pcs,
        type: hit.type,
        intervals: hit.intervals
      };
    }
    return literal(pcs, midiNotes);
  }

  return { detect };
}
