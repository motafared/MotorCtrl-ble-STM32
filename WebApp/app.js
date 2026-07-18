import { Board, VERSION as BLE_VERSION } from './ble.js';
import { bindUI, VERSION as UI_VERSION } from './ui.js';

// Build version — bump the SAME value in all 4 files on every deploy:
// here, ble.js, ui.js, and data-version on <html> in index.html.
// The check below proves the browser loaded one coherent build: if any file
// came from a stale cache, the header badge turns red instead of showing v#.
const VERSION = '1.1.0';

{
  const el = document.getElementById('app-version');
  const parts = [VERSION, BLE_VERSION, UI_VERSION, document.documentElement.dataset.version];
  if (parts.every((v) => v === VERSION)) {
    el.textContent = `v${VERSION}`;
  } else {
    el.textContent = `⚠ mixed cache (${parts.join(' / ')}) — hard-refresh needed`;
    el.className = 'ml-1 text-[10px] font-semibold text-rose-400 align-middle';
  }
}

if (!navigator.bluetooth) {
  document.body.insertAdjacentHTML('afterbegin',
    `<div class="bg-rose-700 text-white p-3 text-sm text-center">
       Web Bluetooth not available. Use Chrome on desktop/Android, or Bluefy on iOS.
     </div>`);
}

const board = new Board();
bindUI(board);
