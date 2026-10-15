import { createBridge } from './bridge.js';
import { createDetector } from './detect.js';
import { buildChordsDbIndex } from './nameMapping.js';
import { resolveDiagram } from './diagramResolver.js';
import { createHistoryView } from './historyView.js';
import { createDiagramView } from './diagramView.js';
import { createSettingsView } from './settingsView.js';
import { createLogsView } from './logsView.js';
import { plainNoteName } from './music.js';

const $ = (id) => document.getElementById(id);
const bridge = createBridge();
const detector = createDetector(globalThis.Tonal);
const state = { mode: 'translation', shown: null, dbIndex: { index: new Map() }, replayActive: false };

const history = createHistoryView($('track'));
const diagram = createDiagramView({
  host: $('diagramHost'),
  pagerText: $('pagerText'),
  prev: $('btnPrev'),
  next: $('btnNext'),
  badge: $('badge'),
  note: $('diagramNote'),
  emptyText: $('emptyDiagram')
});

const settings = createSettingsView(bridge, {
  driver: $('selDriver'),
  device: $('selDevice'),
  rate: $('selRate'),
  buffer: $('selBuffer'),
  midi: $('selMidi'),
  group: $('inGroup'),
  replay: $('inReplay'),
  badge: $('latencyBadge'),
  readout: $('latencyReadout'),
  error: $('audioError'),
  pill: $('midiPill'),
  midiText: $('midiText')
});

function showDiagramFor(detected, midi) {
  state.shown = { detected, midi };
  diagram.show(resolveDiagram({ detected, midiNotes: midi, mode: state.mode, dbIndex: state.dbIndex }));
}

function handleChord(message) {
  const midi = message.notes.map((n) => n.midi);
  const detected = detector.detect(midi);
  $('idleText').classList.add('hidden');
  history.clearPreview();
  history.push(detected.name, detected.ok);
  showDiagramFor(detected, midi);
  bridge.send('chordResult', { id: message.id, name: detected.name, mode: state.mode });
}

function showReplayChord(entry) {
  const midi = entry.notes.map((n) => n.midi);
  const detected = detector.detect(midi);
  $('idleText').classList.add('hidden');
  history.preview(detected.name, detected.ok);
  showDiagramFor(detected, midi);
  bridge.send('replay', { notes: entry.notes });
}

function setMode(mode) {
  state.mode = mode;
  document.querySelectorAll('#modeSwitch button').forEach((b) => b.classList.toggle('active', b.dataset.mode === mode));
  if (state.shown) showDiagramFor(state.shown.detected, state.shown.midi);
}

const logs = createLogsView(
  bridge,
  { logList: $('logList'), chordList: $('chordList'), title: $('logTitle'), browse: $('btnBrowseLog'), refresh: $('btnRefreshLogs') },
  showReplayChord
);

function syncPanelLayout() {
  const anyOpen = !$('settingsPanel').hidden || !$('logsPanel').hidden;
  document.body.classList.toggle('panel-open', anyOpen);
  if ($('logsPanel').hidden) history.clearPreview();
  setTimeout(() => window.dispatchEvent(new Event('resize')), 260);
}

function togglePanel(id) {
  const panel = $(id);
  const open = panel.hidden;
  $('settingsPanel').hidden = true;
  $('logsPanel').hidden = true;
  panel.hidden = !open;
  if (id === 'logsPanel' && open) bridge.send('listLogs');
  syncPanelLayout();
}

document.querySelectorAll('[data-close]').forEach((b) => b.addEventListener('click', () => {
  $(b.dataset.close).hidden = true;
  syncPanelLayout();
}));
$('btnSettings').addEventListener('click', () => togglePanel('settingsPanel'));
$('btnLogs').addEventListener('click', () => togglePanel('logsPanel'));
$('btnSelectVst').addEventListener('click', () => bridge.send('selectVst'));
$('btnEditor').addEventListener('click', () => bridge.send('openVstEditor'));
document.querySelectorAll('#modeSwitch button').forEach((b) => b.addEventListener('click', () => {
  setMode(b.dataset.mode);
  bridge.send('setMode', { mode: b.dataset.mode });
}));

window.addEventListener('keydown', (event) => {
  const tag = event.target && event.target.tagName;
  if (tag === 'SELECT' || tag === 'INPUT') return;
  const left = event.key === 'ArrowLeft';
  const right = event.key === 'ArrowRight';
  if (event.shiftKey && (left || right)) {
    event.preventDefault();
    diagram.step(left ? -1 : 1);
    return;
  }
  if (event.shiftKey || event.ctrlKey || event.altKey || event.metaKey) return;
  const replayOpen = !$('logsPanel').hidden && logs.hasChords();
  if (!replayOpen) return;
  if (event.key === 'ArrowUp' || left) {
    event.preventDefault();
    logs.move(-1);
  } else if (event.key === 'ArrowDown' || right) {
    event.preventDefault();
    logs.move(1);
  }
});

let toastTimer = null;
function toast(text, kind) {
  const el = $('toast');
  el.textContent = text;
  el.className = 'toast ' + (kind || '');
  el.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { el.hidden = true; }, 4500);
}

function applyVst(vst) {
  const name = $('vstName');
  name.textContent = vst.loaded ? vst.name : 'No plugin loaded';
  name.title = vst.path || '';
  name.classList.toggle('loaded', Boolean(vst.loaded));
  name.classList.toggle('muted', !vst.loaded);
  $('btnEditor').disabled = !vst.loaded;
  const notice = $('vstNotice');
  notice.textContent = vst.notice || '';
  notice.hidden = !vst.notice;
}

bridge.on('init', (s) => {
  applyVst(s.vst);
  settings.applyMidi(s.midi);
  settings.applyAudio(s.audio);
  settings.applyBehaviour(s.groupWindowMs, s.replayLengthMs);
  setMode(s.mode);
});
bridge.on('midiStatus', settings.applyMidi);
bridge.on('audioStatus', settings.applyAudio);
bridge.on('vstStatus', applyVst);
bridge.on('chord', handleChord);
bridge.on('held', (s) => {
  $('held').textContent = s.notes.length ? [...s.notes].sort((a, b) => a - b).map((m) => plainNoteName(m, true)).join('  ') : '';
});
bridge.on('logList', (s) => logs.renderList(s.logs));
bridge.on('logOpened', (s) => logs.opened(s));
bridge.on('toast', (s) => toast(s.text, s.kind));

async function loadDb() {
  try {
    const response = await fetch('data/guitar.json');
    state.dbIndex = buildChordsDbIndex(await response.json());
  } catch (error) {
    toast('Chord database could not be loaded; using generated voicings only.', 'bad');
  }
}

loadDb().then(() => bridge.send('ready'));
