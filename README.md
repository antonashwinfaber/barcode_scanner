# Barcode Scanner 📱⚡💻

> Turn any smartphone into a wireless industrial barcode scanner that types directly into your active PC cursor (Excel, ERP, Notepad, inventory forms). **Zero mobile app installation required.**

---

## Quick Start

1. **Launch**: Run `MobileBarcodeScanner.exe` (or `run_live_wifi.bat`).
2. **Pair**: Scan the on-screen QR code with your phone camera.
3. **Scan**: Click into any text field on your PC and aim your phone at a barcode. It will beep, vibrate, and type the code instantly!

---

## Connection Modes

| Mode | Best For | Setup |
| :--- | :--- | :--- |
| **Wi-Fi HTTPS** *(Default)* | Same Wi-Fi / Local Office | Scan the Wi-Fi QR code. Accepts self-signed local cert. |
| **Cloudflare Quick Tunnel** | Anywhere / Mobile Data | Zero configuration. Global SSL URL (`*.trycloudflare.com`). |
| **USB Cable (ADB)** | Air-gapped / Maximum speed | Connect USB cable with USB Debugging enabled. |
| **Plain HTTP LAN** | Internal enterprise intranets | Direct HTTP access without certificates. |

---

## Core Features

- **Zero-Install Web App**: Runs directly in Safari, Chrome, and Firefox via WebRTC.
- **Freeze & Confirm**: Instantly freezes the detected frame with convenient thumb-zone **Continue** and **Cancel** buttons.
- **Industrial Debounce Filter**: Prevents accidental repeat typing while holding the camera on an item (adjustable 0s to 5s cooldown).
- **ZXing-C++ Engine**: Decodes 1D barcodes (Code 128, EAN, UPC, Code 39, ITF) and 2D codes (QR, Data Matrix, Aztec, PDF417).
- **Hardware-Grade Wedge**: Types characters using native Windows `SendInput` Unicode events — immune to keyboard language mismatches.
- **Camera Power Management**: Includes a dedicated **Camera Off / Standby** toggle to sleep the sensor, save battery, and prevent device heating.
- **Single-Device Session Lock**: Dynamic 4-digit PIN ensures exclusive cursor control without accidental multi-user collisions.

---

## Terminal Hotkeys & Mouse Controls

The terminal interface supports both keyboard hotkeys and direct mouse clicks:

- **`[1-4]`**: Switch Windows (1: Dashboard, 2: Activity Logs, 3: Settings, 4: Diagnostics)
- **`[H/T/U]`**: Cycle Connection Mode (Wi-Fi, Tunnel, USB, LAN)
- **`[D]`**: Cycle Debounce Cooldown Filter (0s, 1.5s, 3.0s, 5.0s)
- **`[N]`**: Rename PC Station display name
- **`[K]`**: Kick / Disconnect active phone session
- **`[C]`**: Clear debounce cache & reset history
- **`[Q]`**: Quit application

---

## Building from Source

Requirements: Windows 10/11, CMake 3.20+, MinGW or MSVC (C++20).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
