import { generateForPressed } from './generator.js';
import { mapToChordsDb } from './nameMapping.js';

function generated(midiNotes, source) {
  const result = generateForPressed(midiNotes);
  return {
    source,
    status: result.status,
    positions: result.positions,
    missingPcs: result.missingPcs,
    reasons: result.reasons,
    shifted: result.shifted,
    bassHonored: result.bassHonored !== false,
    dbLabel: null
  };
}

export function resolveDiagram({ detected, midiNotes, mode, dbIndex }) {
  if (mode === 'translation' && detected && detected.ok) {
    const ref = mapToChordsDb(detected, dbIndex);
    if (ref) {
      return {
        source: 'db',
        status: 'ok',
        positions: ref.chord.positions,
        missingPcs: [],
        reasons: [],
        shifted: false,
        bassHonored: true,
        dbLabel: ref.key + ' ' + ref.suffix
      };
    }
    return generated(midiNotes, 'generated');
  }
  return generated(midiNotes, 'literal');
}
