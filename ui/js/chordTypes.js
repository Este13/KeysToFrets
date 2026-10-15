export const CHORD_TYPES = [
  { intervals: '1P 3M 5P', display: '', db: 'major' },
  { intervals: '1P 3m 5P', display: 'm', db: 'minor' },
  { intervals: '1P 3m 5d', display: 'dim', db: 'dim' },
  { intervals: '1P 3m 5d 7d', display: 'dim7', db: 'dim7' },
  { intervals: '1P 3M 5A', display: 'aug', db: 'aug' },
  { intervals: '1P 4P 5P', display: 'sus4', db: 'sus4' },
  { intervals: '1P 2M 5P', display: 'sus2', db: 'sus2' },
  { intervals: '1P 2M 4P 5P', display: 'sus2sus4', db: 'sus2sus4' },
  { intervals: '1P 4P 5P 7m', display: '7sus4', db: '7sus4' },
  { intervals: '1P 5P', display: '5', db: '5' },
  { intervals: '1P 3M 5P 6M', display: '6', db: '6' },
  { intervals: '1P 3M 5P 6M 9M', display: '6/9', db: '69' },
  { intervals: '1P 3m 5P 6M', display: 'm6', db: 'm6' },
  { intervals: '1P 3m 5P 6M 9M', display: 'm6/9', db: 'm69' },
  { intervals: '1P 3M 5P 7m', display: '7', db: '7' },
  { intervals: '1P 3M 7m', display: '7(no5)', db: '7' },
  { intervals: '1P 3M 5d 7m', display: '7b5', db: '7b5' },
  { intervals: '1P 3M 5A 7m', display: '7#5', db: 'aug7' },
  { intervals: '1P 3M 5P 7m 9M', display: '9', db: '9' },
  { intervals: '1P 3M 7m 9M', display: '9(no5)', db: '9' },
  { intervals: '1P 3M 5d 7m 9M', display: '9b5', db: '9b5' },
  { intervals: '1P 3M 5A 7m 9M', display: '9#5', db: 'aug9' },
  { intervals: '1P 3M 5P 7m 9m', display: '7b9', db: '7b9' },
  { intervals: '1P 3M 5P 7m 9A', display: '7#9', db: '7#9' },
  { intervals: '1P 5P 7m 9M 11P', display: '11', db: '11' },
  { intervals: '1P 3M 5P 7m 9M 11A', display: '9#11', db: '9#11' },
  { intervals: '1P 3M 5P 7m 9M 13M', display: '13', db: '13' },
  { intervals: '1P 3M 7m 9M 13M', display: '13(no5)', db: '13' },
  { intervals: '1P 3M 5P 7M', display: 'maj7', db: 'maj7' },
  { intervals: '1P 3M 5d 7M', display: 'maj7b5', db: 'maj7b5' },
  { intervals: '1P 3M 5A 7M', display: 'maj7#5', db: 'maj7#5' },
  { intervals: '1P 3M 5P 7M 9M', display: 'maj9', db: 'maj9' },
  { intervals: '1P 3M 5P 7M 9M 13M', display: 'maj13', db: 'maj13' },
  { intervals: '1P 3M 5P 7M 9M 11A', display: 'maj9#11', db: null },
  { intervals: '1P 3m 5P 7m', display: 'm7', db: 'm7' },
  { intervals: '1P 3m 5d 7m', display: 'm7b5', db: 'm7b5' },
  { intervals: '1P 3m 5P 7m 9M', display: 'm9', db: 'm9' },
  { intervals: '1P 3m 5P 7m 9M 11P', display: 'm11', db: 'm11' },
  { intervals: '1P 3m 5P 7m 9M 13M', display: 'm13', db: null },
  { intervals: '1P 3m 5P 7M', display: 'mMaj7', db: 'mmaj7' },
  { intervals: '1P 3m 5P 7M 9M', display: 'mMaj9', db: 'mmaj9' },
  { intervals: '1P 3M 5P 9M', display: 'add9', db: 'add9' },
  { intervals: '1P 3m 5P 9M', display: 'madd9', db: 'madd9' },
  { intervals: '1P 3M 5P 7m 9m 13M', display: '13b9', db: null },
  { intervals: '1P 3M 5P 7m 11A', display: '7#11', db: null },
  { intervals: '1P 3M 7m 9m', display: '7alt', db: null }
];

const BY_INTERVALS = new Map(CHORD_TYPES.map((t) => [t.intervals, t]));

export function findChordType(intervalsString) {
  return BY_INTERVALS.get(intervalsString) || null;
}

export function isMinorish(intervalsString) {
  return intervalsString.split(' ').includes('3m');
}
