export const STRING_MIDI = [40, 45, 50, 55, 59, 64];
export const MAX_FRET = 15;
export const LOWEST_GUITAR_MIDI = 40;
export const HIGHEST_GUITAR_MIDI = STRING_MIDI[5] + MAX_FRET;

const DB_KEY_BY_PC = ['C', 'C#', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B'];
const MAJOR_NAME_BY_PC = ['C', 'Db', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'Ab', 'A', 'Bb', 'B'];
const MINOR_NAME_BY_PC = ['C', 'C#', 'D', 'Eb', 'E', 'F', 'F#', 'G', 'G#', 'A', 'Bb', 'B'];
const SHARP_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B'];
const FLAT_NAMES = ['C', 'Db', 'D', 'Eb', 'E', 'F', 'Gb', 'G', 'Ab', 'A', 'Bb', 'B'];

export function pcOf(midi) {
  return ((midi % 12) + 12) % 12;
}

export function dbKeyName(pc) {
  return DB_KEY_BY_PC[pcOf(pc)];
}

export function chordNoteName(pc, minorish) {
  return (minorish ? MINOR_NAME_BY_PC : MAJOR_NAME_BY_PC)[pcOf(pc)];
}

export function plainNoteName(midi, withOctave) {
  const name = SHARP_NAMES[pcOf(midi)];
  return withOctave ? name + (Math.floor(midi / 12) - 1) : name;
}

export function spellPcs(pcs, scheme) {
  if (scheme === 'flats') return pcs.map((pc) => FLAT_NAMES[pcOf(pc)]);
  return pcs.map((pc) => SHARP_NAMES[pcOf(pc)]);
}

export function altSpelling(pc) {
  return [SHARP_NAMES[pcOf(pc)], FLAT_NAMES[pcOf(pc)]];
}

export function uniquePcsFromBass(midiNotes) {
  const sorted = [...midiNotes].sort((a, b) => a - b);
  const seen = new Set();
  const pcs = [];
  for (const m of sorted) {
    const pc = pcOf(m);
    if (!seen.has(pc)) {
      seen.add(pc);
      pcs.push(pc);
    }
  }
  return pcs;
}
