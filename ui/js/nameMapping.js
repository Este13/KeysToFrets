import { dbKeyName } from './music.js';

export function buildChordsDbIndex(guitarDb) {
  const index = new Map();
  for (const key of Object.keys(guitarDb.chords)) {
    for (const chord of guitarDb.chords[key]) {
      index.set(chord.key + '|' + chord.suffix, chord);
    }
  }
  return { index, keys: guitarDb.keys, suffixes: new Set(guitarDb.suffixes) };
}

export function dbSuffixFor(typeDbSuffix, bassName) {
  if (!bassName) return typeDbSuffix;
  if (typeDbSuffix === 'major') return '/' + bassName;
  if (typeDbSuffix === 'minor') return 'm/' + bassName;
  return typeDbSuffix + '/' + bassName;
}

export function mapToChordsDb(detected, dbIndex) {
  if (!detected || !detected.ok || !detected.type || !detected.type.db) return null;
  const key = dbKeyName(detected.rootPc);
  const bassName = detected.bassPc === null ? null : dbKeyName(detected.bassPc);
  const suffix = dbSuffixFor(detected.type.db, bassName);
  const chord = dbIndex.index.get(key + '|' + suffix);
  if (!chord) return null;
  return { key, suffix, chord };
}
