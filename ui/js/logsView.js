function formatTime(ms) {
  const total = Math.floor(ms / 1000);
  const m = Math.floor(total / 60);
  const s = String(total % 60).padStart(2, '0');
  return `${m}:${s}`;
}

function row(html, onClick) {
  const li = document.createElement('li');
  li.innerHTML = html;
  li.addEventListener('click', onClick);
  return li;
}

function escapeHtml(text) {
  return String(text).replace(/[&<>"]/g, (c) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
}

export function createLogsView(bridge, els, onSelectChord) {
  let chords = [];
  let selected = -1;

  function renderChords() {
    els.chordList.innerHTML = '';
    chords.forEach((chord, i) => {
      const li = row(
        `<span class="nm">${escapeHtml(chord.name || '?')}</span><span class="sub">${formatTime(chord.t)} &middot; ${chord.notes.length} notes</span>`,
        () => select(i)
      );
      els.chordList.appendChild(li);
    });
  }

  function select(index) {
    if (chords.length === 0) return;
    selected = Math.max(0, Math.min(chords.length - 1, index));
    [...els.chordList.children].forEach((li, i) => li.classList.toggle('selected', i === selected));
    const li = els.chordList.children[selected];
    if (li) li.scrollIntoView({ block: 'nearest' });
    onSelectChord(chords[selected]);
  }

  function move(delta) {
    select(selected < 0 ? 0 : selected + delta);
  }

  function renderList(logs) {
    els.logList.innerHTML = '';
    for (const log of logs) {
      els.logList.appendChild(
        row(`<span>${escapeHtml(log.date)}</span><span class="sub">${log.count} chords</span>`, () => bridge.send('openLog', { path: log.path }))
      );
    }
    if (logs.length === 0) els.logList.innerHTML = '<li class="muted">No sessions yet</li>';
  }

  function opened(log) {
    chords = log.chords || [];
    selected = -1;
    els.title.textContent = `${log.name} — ${chords.length} chords`;
    renderChords();
    els.chordList.focus({ preventScroll: true });
  }

  els.browse.addEventListener('click', () => bridge.send('browseLog'));
  els.refresh.addEventListener('click', () => bridge.send('listLogs'));

  return { renderList, opened, move, hasChords: () => chords.length > 0 };
}
