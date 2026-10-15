const STRING_COUNT = 6;
const STRING_GAP = 30;
const FRET_HEIGHT = 36;
const LEFT = 58;
const TOP = 46;
const DOT_RADIUS = 12;
const MIN_ROWS = 4;

function xOf(stringIndex) {
  return LEFT + stringIndex * STRING_GAP;
}

function rowCenter(row) {
  return TOP + (row - 0.5) * FRET_HEIGHT;
}

function gridLines(rows, baseFret) {
  const parts = [];
  const bottom = TOP + rows * FRET_HEIGHT;
  for (let s = 0; s < STRING_COUNT; s++) {
    parts.push(`<line class="dg-string" x1="${xOf(s)}" y1="${TOP}" x2="${xOf(s)}" y2="${bottom}"/>`);
  }
  for (let r = 0; r <= rows; r++) {
    const y = TOP + r * FRET_HEIGHT;
    parts.push(`<line class="dg-fret" x1="${xOf(0)}" y1="${y}" x2="${xOf(5)}" y2="${y}"/>`);
  }
  if (baseFret === 1) {
    parts.push(`<rect class="dg-nut" x="${xOf(0) - 1}" y="${TOP - 4}" width="${xOf(5) - xOf(0) + 2}" height="6" rx="1"/>`);
  } else {
    parts.push(`<text class="dg-label" x="${LEFT - 22}" y="${rowCenter(1) + 5}" text-anchor="end">${baseFret}fr</text>`);
  }
  return parts;
}

function stringMarkers(frets) {
  const parts = [];
  const y = TOP - 22;
  frets.forEach((fret, s) => {
    const x = xOf(s);
    if (fret < 0) {
      parts.push(`<path class="dg-x" d="M${x - 5} ${y - 5} L${x + 5} ${y + 5} M${x + 5} ${y - 5} L${x - 5} ${y + 5}"/>`);
    } else if (fret === 0) {
      parts.push(`<circle class="dg-o" cx="${x}" cy="${y}" r="6"/>`);
    }
  });
  return parts;
}

function barreShapes(position) {
  const parts = [];
  const covered = new Set();
  for (const barreFret of position.barres || []) {
    const strings = [];
    position.frets.forEach((f, s) => {
      if (f === barreFret) strings.push(s);
    });
    if (strings.length < 2) continue;
    const from = Math.min(...strings);
    const to = Math.max(...strings);
    const cy = rowCenter(barreFret);
    parts.push(
      `<rect class="dg-barre" x="${xOf(from) - DOT_RADIUS}" y="${cy - DOT_RADIUS}" width="${xOf(to) - xOf(from) + DOT_RADIUS * 2}" height="${DOT_RADIUS * 2}" rx="${DOT_RADIUS}"/>`
    );
    const finger = position.fingers[from] || 1;
    parts.push(`<text class="dg-num" x="${xOf(from)}" y="${cy + 4.5}" text-anchor="middle">${finger}</text>`);
    strings.forEach((s) => covered.add(s));
  }
  return { parts, covered };
}

function dotShapes(position, covered) {
  const parts = [];
  position.frets.forEach((fret, s) => {
    if (fret <= 0 || covered.has(s)) return;
    const cx = xOf(s);
    const cy = rowCenter(fret);
    parts.push(`<circle class="dg-dot" cx="${cx}" cy="${cy}" r="${DOT_RADIUS}"/>`);
    const finger = position.fingers && position.fingers[s];
    if (finger) parts.push(`<text class="dg-num" x="${cx}" y="${cy + 4.5}" text-anchor="middle">${finger}</text>`);
  });
  return parts;
}

export function renderDiagramSvg(position) {
  const maxFret = Math.max(0, ...position.frets);
  const rows = Math.max(MIN_ROWS, maxFret);
  const baseFret = position.baseFret || 1;
  const width = LEFT + 5 * STRING_GAP + 30;
  const height = TOP + rows * FRET_HEIGHT + 14;
  const barre = barreShapes(position);
  const body = [
    ...gridLines(rows, baseFret),
    ...barre.parts,
    ...dotShapes(position, barre.covered),
    ...stringMarkers(position.frets)
  ];
  return `<svg class="diagram" viewBox="0 0 ${width} ${height}" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="Guitar chord diagram">${body.join('')}</svg>`;
}
