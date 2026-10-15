const KEY_ROW = 'awsedftgyhujkolp';
const BASE_NOTE = 48;

export function createMockBackend() {
  const handlers = new Map();
  const held = new Map();
  let nextId = 1;
  let groupTimer = null;
  const startedAt = performance.now();

  function emit(eventId, payload) {
    const list = handlers.get(eventId) || [];
    setTimeout(() => list.forEach((fn) => fn(payload)), 0);
  }

  function commit() {
    groupTimer = null;
    const notes = [...held.entries()].map(([midi, vel]) => ({ midi, vel }));
    if (notes.length < 2) return;
    emit('chord', { id: nextId++, t: Math.round(performance.now() - startedAt), notes });
  }

  function onKey(event, down) {
    if (event.repeat || event.ctrlKey || event.metaKey || event.altKey) return;
    const index = KEY_ROW.indexOf(event.key.toLowerCase());
    if (index < 0) return;
    const midi = BASE_NOTE + index + (event.shiftKey ? 12 : 0);
    if (down) {
      held.set(midi, 96);
      if (!groupTimer) groupTimer = setTimeout(commit, 60);
    } else {
      held.delete(midi);
    }
    emit('held', { notes: [...held.keys()] });
  }

  window.addEventListener('keydown', (e) => onKey(e, true));
  window.addEventListener('keyup', (e) => onKey(e, false));

  const responders = {
    ready() {
      emit('init', {
        midi: { connected: true, name: 'Browser preview (keys A-P)', devices: [] },
        vst: { loaded: false, name: '', path: '', notice: '' },
        audio: { types: [], devices: [], sampleRates: [], bufferSizes: [], latencyMs: 0, bufferMs: 0 },
        mode: 'translation',
        groupWindowMs: 60,
        replayLengthMs: 600
      });
    },
    chordResult() {}
  };

  return {
    native: false,
    send(eventId, payload) {
      if (responders[eventId]) responders[eventId](payload);
    },
    on(eventId, handler) {
      if (!handlers.has(eventId)) handlers.set(eventId, []);
      handlers.get(eventId).push(handler);
    }
  };
}
