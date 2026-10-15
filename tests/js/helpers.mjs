import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import * as Tonal from 'tonal';
import { createDetector } from '../../ui/js/detect.js';
import { buildChordsDbIndex } from '../../ui/js/nameMapping.js';

const here = path.dirname(fileURLToPath(import.meta.url));
export const guitarDb = JSON.parse(fs.readFileSync(path.join(here, '..', '..', 'third_party', 'chords-db', 'guitar.json'), 'utf8'));
export const dbIndex = buildChordsDbIndex(guitarDb);
export const detector = createDetector(Tonal);
export { Tonal };

const NOTE_OFFSETS = { C: 0, 'C#': 1, Db: 1, D: 2, 'D#': 3, Eb: 3, E: 4, F: 5, 'F#': 6, Gb: 6, G: 7, 'G#': 8, Ab: 8, A: 9, 'A#': 10, Bb: 10, B: 11 };

export function midiOf(name, octave) {
  return 12 * (octave + 1) + NOTE_OFFSETS[name];
}

export function chordMidi(names, startOctave = 3) {
  const out = [];
  let previous = -1;
  for (const name of names) {
    let midi = midiOf(name, startOctave);
    while (midi <= previous) midi += 12;
    out.push(midi);
    previous = midi;
  }
  return out;
}
