import test from 'node:test';
import assert from 'node:assert/strict';
import { renderDiagramSvg } from '../../ui/js/renderer.js';
import { guitarDb } from './helpers.mjs';

const cMajor = guitarDb.chords.C.find((c) => c.suffix === 'major').positions[0];
const barreChord = guitarDb.chords.F.find((c) => c.suffix === 'major').positions[0];
const highChord = guitarDb.chords.C.find((c) => c.suffix === 'major').positions[1];

test('open C shows a muted string, open strings, nut and three numbered dots', () => {
  const svg = renderDiagramSvg(cMajor);
  assert.equal((svg.match(/class="dg-x"/g) || []).length, 1);
  assert.equal((svg.match(/class="dg-o"/g) || []).length, 2);
  assert.equal((svg.match(/class="dg-dot"/g) || []).length, 3);
  assert.ok(svg.includes('class="dg-nut"'));
  assert.ok(!svg.includes('fr<'));
});

test('barre chords draw a rounded bar and a finger number', () => {
  const svg = renderDiagramSvg(barreChord);
  assert.ok(svg.includes('class="dg-barre"'));
  assert.match(svg, /rx="12"/);
});

test('positions above the nut show the start fret label', () => {
  const svg = renderDiagramSvg(highChord);
  assert.ok(svg.includes(`${highChord.baseFret}fr`));
  assert.ok(!svg.includes('class="dg-nut"'));
});

test('every position in the database renders without throwing', () => {
  let count = 0;
  for (const key of Object.keys(guitarDb.chords)) {
    for (const chord of guitarDb.chords[key]) {
      for (const position of chord.positions) {
        assert.ok(renderDiagramSvg(position).startsWith('<svg'));
        count++;
      }
    }
  }
  assert.equal(count, 3283);
});
