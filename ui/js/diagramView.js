import { renderDiagramSvg } from './renderer.js';
import { describeMissing } from './generator.js';

export function createDiagramView(elements) {
  const { host, pagerText, prev, next, badge, note, emptyText } = elements;
  let positions = [];
  let index = 0;
  let meta = null;

  function metaLine() {
    if (!meta) return { badgeText: '', badgeClass: '', noteText: '' };
    if (meta.status === 'none' && positions.length === 0) {
      return { badgeText: 'unplayable', badgeClass: 'bad', noteText: 'No guitar voicing could be built from these notes.' };
    }
    if (meta.status === 'unplayable') {
      const why = meta.reasons.length ? meta.reasons.join(', ') : 'not playable as pressed';
      const missing = meta.missingPcs.length ? ' Missing: ' + describeMissing(meta.missingPcs) + '.' : '';
      return { badgeText: 'unplayable', badgeClass: 'bad', noteText: 'Closest playable voicing (' + why + ').' + missing };
    }
    const flags = [];
    if (meta.shifted) flags.push('octave-shifted to fit the guitar range');
    if (meta.bassHonored === false) flags.push('bass note not lowest');
    const noteText = flags.join(', ');
    if (meta.source === 'db') return { badgeText: 'chords-db ' + meta.dbLabel, badgeClass: 'db', noteText };
    return { badgeText: 'generated', badgeClass: 'gen', noteText };
  }

  function render() {
    const info = metaLine();
    badge.textContent = info.badgeText;
    badge.className = 'badge ' + info.badgeClass;
    note.textContent = info.noteText;
    if (positions.length === 0) {
      host.innerHTML = '';
      emptyText.hidden = false;
      pagerText.textContent = '0 of 0';
      prev.disabled = next.disabled = true;
      return;
    }
    emptyText.hidden = true;
    host.innerHTML = renderDiagramSvg(positions[index]);
    pagerText.textContent = `${index + 1} of ${positions.length}`;
    prev.disabled = next.disabled = positions.length < 2;
  }

  function show(resolved) {
    meta = resolved;
    positions = resolved ? resolved.positions : [];
    index = 0;
    render();
  }

  function step(delta) {
    if (positions.length < 2) return;
    index = (index + delta + positions.length) % positions.length;
    render();
  }

  prev.addEventListener('click', () => step(-1));
  next.addEventListener('click', () => step(1));
  host.addEventListener('wheel', (event) => {
    event.preventDefault();
    step(event.deltaY > 0 ? 1 : -1);
  }, { passive: false });

  return { show, step };
}
