import { Board } from './ble.js';
import { bindUI } from './ui.js';

if (!navigator.bluetooth) {
  document.body.insertAdjacentHTML('afterbegin',
    `<div class="bg-rose-700 text-white p-3 text-sm text-center">
       Web Bluetooth not available. Use Chrome on desktop/Android, or Bluefy on iOS.
     </div>`);
}

const board = new Board();
bindUI(board);
