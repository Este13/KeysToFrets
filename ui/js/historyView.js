const MAX_HISTORY = 6;
const GAP = 28;
const SCALES = [1, 0.46, 0.4, 0.35, 0.31, 0.28, 0.25];
const OPACITIES = [1, 0.62, 0.5, 0.42, 0.34, 0.27, 0.2];
const REMOVE_DELAY = 450;

export function createHistoryView(track) {
  const items = [];
  let layoutQueued = false;

  function slotTransform(x, scale) {
    return `translate(-50%, -50%) translateX(${x}px) scale(${scale})`;
  }

  function layout() {
    layoutQueued = false;
    if (items.length === 0) return;
    const currentWidth = items[0].el.offsetWidth;
    let cursor = -(currentWidth / 2 + GAP);
    items.forEach((item, i) => {
      const scale = SCALES[Math.min(i, SCALES.length - 1)];
      const opacity = OPACITIES[Math.min(i, OPACITIES.length - 1)];
      let x = 0;
      if (i > 0) {
        const width = item.el.offsetWidth * scale;
        x = cursor - width / 2;
        cursor -= width + GAP;
      }
      item.el.style.transform = slotTransform(x, scale);
      item.el.style.opacity = String(opacity);
      item.el.style.zIndex = String(100 - i);
      item.el.classList.toggle('is-current', i === 0);
    });
  }

  function queueLayout() {
    if (layoutQueued) return;
    layoutQueued = true;
    requestAnimationFrame(layout);
  }

  function fontSizeFor(name, ok) {
    if (ok) return '';
    if (name.length > 24) return '28px';
    if (name.length > 12) return '40px';
    return '';
  }

  function makeChip(name, ok) {
    const el = document.createElement('div');
    el.className = 'chip' + (ok ? '' : ' chip-literal');
    el.textContent = name;
    el.style.fontSize = fontSizeFor(name, ok);
    el.style.transform = slotTransform(0, 0.7);
    el.style.opacity = '0';
    track.appendChild(el);
    return el;
  }

  function push(name, ok) {
    const item = { el: makeChip(name, ok), name, ok };
    items.unshift(item);
    while (items.length > MAX_HISTORY + 1) {
      const old = items.pop();
      old.el.style.opacity = '0';
      setTimeout(() => old.el.remove(), REMOVE_DELAY);
    }
    queueLayout();
  }

  function preview(name, ok) {
    if (items.length === 0) push(name, ok);
    const el = items[0].el;
    el.textContent = name;
    el.style.fontSize = fontSizeFor(name, ok);
    el.classList.toggle('chip-literal', !ok);
    el.classList.add('is-preview');
    queueLayout();
  }

  function clearPreview() {
    if (items.length === 0) return;
    const el = items[0].el;
    el.textContent = items[0].name;
    el.style.fontSize = fontSizeFor(items[0].name, items[0].ok);
    el.classList.toggle('chip-literal', !items[0].ok);
    el.classList.remove('is-preview');
    queueLayout();
  }

  window.addEventListener('resize', queueLayout);
  return { push, preview, clearPreview };
}
