import test from 'node:test';
import assert from 'node:assert/strict';
import { CHORD_TYPES } from '../../ui/js/chordTypes.js';
import { dbSuffixFor, mapToChordsDb } from '../../ui/js/nameMapping.js';
import { dbKeyName } from '../../ui/js/music.js';
import { chordMidi, dbIndex, detector, guitarDb, Tonal } from './helpers.mjs';

test('every mapped chords-db suffix exists in the database', () => {
  for (const type of CHORD_TYPES) {
    if (type.db) assert.ok(dbIndex.suffixes.has(type.db), `missing suffix ${type.db}`);
  }
});

test('every interval signature is a real Tonal chord type', () => {
  const known = new Set(Tonal.ChordType.all().map((t) => t.intervals.join(' ')));
  for (const type of CHORD_TYPES) assert.ok(known.has(type.intervals), `unknown intervals ${type.intervals}`);
});

test('interval signatures are unique', () => {
  const seen = new Set(CHORD_TYPES.map((t) => t.intervals));
  assert.equal(seen.size, CHORD_TYPES.length);
});

test('slash suffixes follow the chords-db conventions', () => {
  assert.equal(dbSuffixFor('major', 'F#'), '/F#');
  assert.equal(dbSuffixFor('minor', 'B'), 'm/B');
  assert.equal(dbSuffixFor('m9', 'Bb'), 'm9/Bb');
  assert.equal(dbSuffixFor('7', 'G'), '7/G');
  assert.equal(dbSuffixFor('maj7', null), 'maj7');
});

const LOOKUPS = [
  [['C', 'E', 'G'], 'C', 'major'],
  [['E', 'G', 'B'], 'E', 'minor'],
  [['F#', 'A', 'D'], 'D', '/F#'],
  [['C', 'E', 'G', 'B'], 'C', 'maj7'],
  [['B', 'D', 'F', 'A'], 'B', 'm7b5'],
  [['Bb', 'D', 'F'], 'Bb', 'major'],
  [['C#', 'E', 'G#'], 'C#', 'minor'],
  [['A#', 'C#', 'F'], 'Bb', 'minor'],
  [['G', 'C', 'D'], 'G', 'sus4'],
  [['C', 'E', 'G', 'Bb', 'D'], 'C', '9'],
  [['C', 'E', 'G', 'Bb', 'D', 'A'], 'C', '13'],
  [['C', 'E', 'G', 'D'], 'C', 'add9']
];

for (const [notes, key, suffix] of LOOKUPS) {
  test(`${notes.join(' ')} maps to chords-db ${key} ${suffix}`, () => {
    const ref = mapToChordsDb(detector.detect(chordMidi(notes)), dbIndex);
    assert.ok(ref, 'no mapping found');
    assert.equal(ref.key, key);
    assert.equal(ref.suffix, suffix);
  });
}

test('chords absent from chords-db return null so the generator takes over', () => {
  const detected = detector.detect(chordMidi(['E', 'G#', 'B', 'D#'], 3));
  assert.equal(detected.name, 'Emaj7');
  assert.equal(mapToChordsDb({ ...detected, bassPc: 8 }, dbIndex), null);
});

test('database voicings round-trip to the same chord for the vast majority of entries', () => {
  let tried = 0;
  let same = 0;
  const mismatches = [];
  for (const key of Object.keys(guitarDb.chords)) {
    for (const chord of guitarDb.chords[key]) {
      if (chord.suffix.includes('/') || !chord.positions[0]) continue;
      const detected = detector.detect(chord.positions[0].midi);
      tried++;
      const ref = mapToChordsDb(detected, dbIndex);
      if (ref && ref.key === chord.key && ref.suffix === chord.suffix) same++;
      else mismatches.push(`${chord.key} ${chord.suffix} -> ${detected.name}`);
    }
  }
  const ratio = same / tried;
  console.log(`round trip: ${same}/${tried} (${(ratio * 100).toFixed(1)}%)`);
  assert.ok(ratio > 0.55, `round trip too low: ${mismatches.slice(0, 20).join('; ')}`);
});

test('every table entry is detected and mapped back from its own intervals in all 12 keys', () => {
  const failures = [];
  for (const type of CHORD_TYPES) {
    for (let root = 0; root < 12; root++) {
      const midi = type.intervals.split(' ').map((iv) => 48 + root + Tonal.Interval.semitones(iv));
      const detected = detector.detect(midi);
      const ok = detected.ok && detected.intervals === type.intervals && detected.rootPc === root;
      if (!ok) failures.push(`${type.intervals} @${root} -> ${detected.name}`);
      if (ok && type.db) {
        const ref = mapToChordsDb(detected, dbIndex);
        const listed = dbIndex.index.has(dbKeyName(root) + '|' + type.db);
        if (listed && (!ref || ref.suffix !== type.db)) failures.push(`${detected.name} not mapped to ${type.db}`);
        if (!listed && ref) failures.push(`${detected.name} mapped although absent from the database`);
      }
    }
  }
  assert.deepEqual(failures.slice(0, 30), []);
});
