# MotorCtrl WebApp

Web Bluetooth dashboard for the **MotorCtrl** STM32WB55 BLDC controller (Phase 2 firmware).

- Connect to the board over BLE 5 directly from the browser — no install.
- Live actual / target RPM for both motors, rotation counts, elapsed time.
- Start (timed), Stop, Start (continuous) commands.
- Live speed chart (60-second rolling window).
- Settings are locked while the motors are running (stop first to change).

## Browser support

- **Desktop / Android:** Google Chrome, Edge, Brave (any Chromium with Web Bluetooth).
- **iOS / iPadOS:** [Bluefy](https://apps.apple.com/app/bluefy-web-ble-browser/id1492822055) (free, App Store).

Web Bluetooth requires a secure context. Use `https://` (e.g. GitHub Pages) or `http://localhost` for development. `file://` is **not** supported.

## Run locally

```bash
python -m http.server 8000
# then open http://localhost:8000
```

## Firmware

See the firmware repo: [MotorCtrl-ble-STM32](https://github.com/motafared/MotorCtrl-ble-STM32).
