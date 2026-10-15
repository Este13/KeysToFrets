import { createMockBackend } from './mockBackend.js';

function createNativeBridge() {
  const backend = window.__JUCE__.backend;
  return {
    native: true,
    send(eventId, payload) {
      backend.emitEvent(eventId, payload === undefined ? {} : payload);
    },
    on(eventId, handler) {
      backend.addEventListener(eventId, handler);
    }
  };
}

export function createBridge() {
  if (window.__JUCE__ && window.__JUCE__.backend) return createNativeBridge();
  return createMockBackend();
}
