import test from 'node:test';
import assert from 'node:assert/strict';
import { chordMidi, detector } from './helpers.mjs';

const CASES = [
  [['C', 'E', 'G'], 'C'],
  [['E', 'G', 'B'], 'Em'],
  [['A', 'C', 'E'], 'Am'],
  [['F#', 'A', 'D'], 'D/F#'],
  [['E', 'G', 'C'], 'C/E'],
  [['C', 'E', 'G', 'B'], 'Cmaj7'],
  [['B', 'D', 'F', 'A'], 'Bm7b5'],
  [['C', 'E', 'G', 'Bb'], 'C7'],
  [['D', 'G', 'A'], 'Dsus4'],
  [['D', 'E', 'A'], 'Dsus2'],
  [['C', 'E', 'G', 'D'], 'Cadd9'],
  [['C', 'E', 'G', 'Bb', 'D'], 'C9'],
  [['C', 'E', 'G', 'Bb', 'D', 'A'], 'C13'],
  [['G#', 'B', 'D#'], 'G#m'],
  [['Bb', 'D', 'F'], 'Bb'],
  [['F#', 'A#', 'C#'], 'F#'],
  [['C', 'Eb', 'Gb'], 'Cdim'],
  [['C', 'G'], 'C5']
];

for (const [notes, expected] of CASES) {
  test(`detects ${notes.join(' ')} as ${expected}`, () => {
    const result = detector.detect(chordMidi(notes));
    assert.equal(result.ok, true);
    assert.equal(result.name, expected);
  });
}

test('lowest note becomes the bass of a slash chord', () => {
  const result = detector.detect([42, 57, 62]);
  assert.equal(result.name, 'D/F#');
  assert.equal(result.bass, 'F#');
});

test('octave doubling does not change the name', () => {
  assert.equal(detector.detect([48, 55, 60, 64, 67, 72]).name, 'C');
});

test('unrecognised sets fall back to literal note names', () => {
  const result = detector.detect(chordMidi(['C', 'D', 'E']));
  assert.equal(result.ok, false);
  assert.equal(result.name, 'C D E');
});

test('a single note is shown as its name', () => {
  const result = detector.detect([61]);
  assert.equal(result.ok, false);
  assert.equal(result.name, 'C#');
});
