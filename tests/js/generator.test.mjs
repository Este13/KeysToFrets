import test from 'node:test';
import assert from 'node:assert/strict';
import { generateForPcs, generateForPressed } from '../../ui/js/generator.js';
import { STRING_MIDI, pcOf } from '../../ui/js/music.js';
import { chordMidi } from './helpers.mjs';

function absoluteFrets(position) {
  return position.frets.map((f) => (f > 0 ? f + position.baseFret - 1 : f));
}

function soundedPcs(position) {
  const abs = absoluteFrets(position);
  const out = [];
  abs.forEach((f, s) => {
    if (f >= 0) out.push(pcOf(STRING_MIDI[s] + f));
  });
  return out;
}

function assertValid(result, pcs, bassPc) {
  assert.notEqual(result.status, 'none');
  assert.ok(result.positions.length > 0);
  assert.ok(result.positions.length <= 20);
  const required = new Set(pcs);
  for (const position of result.positions) {
    const played = soundedPcs(position);
    assert.deepEqual(new Set(played), required, `wrong pitch classes in ${position.frets}`);
    if (result.bassHonored) assert.equal(played[0], bassPc, `bass not lowest in ${position.frets}`);
    assert.ok(new Set(position.fingers.filter(Boolean)).size <= 4);
    const fretted = absoluteFrets(position).filter((f) => f > 0);
    if (fretted.length) assert.ok(Math.max(...fretted) - Math.min(...fretted) <= 4);
  }
}

function topContains(result, frets, depth = 20) {
  return result.positions.slice(0, depth).some((p) => absoluteFrets(p).join() === frets.join());
}

const KNOWN = [
  ['C', [0, 4, 7], 0, [-1, 3, 2, 0, 1, 0]],
  ['G', [7, 11, 2], 7, [3, 2, 0, 0, 0, 3]],
  ['Am', [9, 0, 4], 9, [-1, 0, 2, 2, 1, 0]],
  ['D/F#', [6, 9, 2], 6, [2, -1, 0, 2, 3, 2]],
  ['Cmaj7', [0, 4, 7, 11], 0, [-1, 3, 2, 0, 0, 0]],
  ['Bm7b5', [11, 2, 5, 9], 11, [-1, 2, 3, 2, 3, -1]],
  ['Dsus4', [2, 7, 9], 2, [-1, -1, 0, 2, 3, 3]],
  ['Cadd9', [0, 4, 7, 2], 0, [-1, 3, 2, 0, 3, 0]],
  ['C9', [0, 4, 7, 10, 2], 0, [-1, 3, 2, 3, 3, -1]],
  ['C13', [0, 4, 10, 2, 9], 0, [-1, 3, 2, 3, 5, 5]]
];

for (const [name, pcs, bass, expectedShape] of KNOWN) {
  test(`generator builds valid voicings for ${name}`, () => {
    const result = generateForPcs(pcs, bass);
    assert.equal(result.status, 'ok');
    assertValid(result, pcs, bass);
    assert.equal(result.positions.length, 20);
    assert.ok(result.positions[0].score <= result.positions[19].score);
    if (name === 'C' || name === 'G' || name === 'Am') assert.ok(topContains(result, expectedShape, 5), `${name} shape missing`);
  });
}

test('D/F# keeps F# as the lowest sounding string', () => {
  const result = generateForPcs([6, 9, 2], 6);
  assert.equal(result.bassHonored, true);
  for (const position of result.positions) assert.equal(soundedPcs(position)[0], 6);
});

test('barre chords are detected as a single finger', () => {
  const result = generateForPcs([5, 9, 0], 5);
  const barre = result.positions.find((p) => p.barres.length > 0);
  assert.ok(barre, 'no barre voicing found for F');
  assert.ok(barre.capo);
});

test('pressed notes use their lowest key as bass', () => {
  const result = generateForPressed([42, 57, 62]);
  assert.equal(result.status, 'ok');
  assertValid(result, [6, 9, 2], 6);
});

test('seven distinct notes are reported unplayable with a closest voicing and the missing notes', () => {
  const result = generateForPressed(chordMidi(['C', 'E', 'G', 'Bb', 'D', 'F', 'A'], 3));
  assert.equal(result.status, 'unplayable');
  assert.ok(result.reasons.includes('more than 6 distinct notes'));
  assert.ok(result.positions.length > 0);
  assert.ok(result.missingPcs.length >= 1);
  for (const position of result.positions) {
    const played = new Set(soundedPcs(position));
    for (const pc of played) assert.ok([0, 4, 7, 10, 2, 5, 9].includes(pc), 'invented note');
  }
});

test('notes below the guitar range are octave shifted, never silently altered', () => {
  const result = generateForPressed([36, 52, 55]);
  assert.equal(result.status, 'ok');
  assert.equal(result.shifted, true);
  assertValid(result, [0, 4, 7], 0);
});

test('empty input does not crash', () => {
  assert.equal(generateForPressed([]).status, 'none');
});

test('generation is fast enough to run on every chord', () => {
  const start = performance.now();
  for (let i = 0; i < 10; i++) generateForPcs([0, 4, 7, 10, 2, 9], 0);
  assert.ok((performance.now() - start) / 10 < 80);
});
