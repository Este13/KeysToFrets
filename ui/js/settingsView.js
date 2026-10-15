function fillSelect(select, items, current) {
  select.innerHTML = '';
  for (const item of items) {
    const option = document.createElement('option');
    option.value = String(item.value);
    option.textContent = item.label;
    select.appendChild(option);
  }
  if (current !== undefined && current !== null) select.value = String(current);
}

function formatMs(value) {
  return value > 0 ? value.toFixed(1) + ' ms' : '—';
}

export function createSettingsView(bridge, els) {
  let audioTypes = [];

  function postAudio(partial) {
    bridge.send('setAudio', {
      type: els.driver.value,
      device: els.device.value,
      sampleRate: Number(els.rate.value) || 0,
      bufferSize: Number(els.buffer.value) || 0,
      ...partial
    });
  }

  els.driver.addEventListener('change', () => postAudio({ device: '', sampleRate: 0, bufferSize: 0 }));
  els.device.addEventListener('change', () => postAudio({ sampleRate: 0, bufferSize: 0 }));
  els.rate.addEventListener('change', () => postAudio({}));
  els.buffer.addEventListener('change', () => postAudio({}));
  els.midi.addEventListener('change', () => bridge.send('setMidiDevice', { id: els.midi.value }));
  els.group.addEventListener('change', () => bridge.send('setSetting', { key: 'groupWindowMs', value: Number(els.group.value) }));
  els.replay.addEventListener('change', () => bridge.send('setSetting', { key: 'replayLengthMs', value: Number(els.replay.value) }));

  function applyAudio(status) {
    audioTypes = status.types || [];
    fillSelect(els.driver, audioTypes.map((t) => ({ value: t.id, label: t.label })), status.type);
    fillSelect(els.device, (status.devices || []).map((d) => ({ value: d, label: d })), status.device);
    fillSelect(els.rate, (status.sampleRates || []).map((r) => ({ value: r, label: r + ' Hz' })), status.sampleRate);
    fillSelect(els.buffer, (status.bufferSizes || []).map((b) => ({ value: b, label: b + ' samples' })), status.bufferSize);
    const total = formatMs(status.latencyMs);
    els.badge.textContent = status.latencyMs > 0 ? 'Latency ' + total : 'Latency —';
    els.readout.textContent = status.latencyMs > 0
      ? `Output latency ${total} (buffer ${formatMs(status.bufferMs)}), ${status.driverLabel || ''}`
      : 'No audio device open';
    els.error.hidden = !status.error;
    els.error.textContent = status.error || '';
  }

  function applyMidi(status) {
    els.pill.classList.toggle('on', Boolean(status.connected));
    els.midiText.textContent = status.connected ? 'MIDI: ' + status.name : 'MIDI: not connected';
    fillSelect(els.midi, (status.devices || []).map((d) => ({ value: d.id, label: d.name })), status.id);
  }

  function applyBehaviour(groupWindowMs, replayLengthMs) {
    els.group.value = groupWindowMs;
    els.replay.value = replayLengthMs;
  }

  return { applyAudio, applyMidi, applyBehaviour };
}
