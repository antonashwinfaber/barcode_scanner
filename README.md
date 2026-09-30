# Mobile Barcode Scanner to PC Keystroke Wedge

Turn your mobile phone into an instant, wireless 1D/2D barcode and QR scanner for your PC. Scanned codes are automatically typed directly into whatever window your cursor is focused on (Excel, Notepad, ERP forms, web search, etc.).

Powered by **ZXing-C++**, **Windows SendInput**, and a zero-install **HTML5 Mobile Client**.

---

## Features

- **Zero Phone Installation**: Works in standard mobile browsers (Safari, Chrome, Firefox). Just connect phone to Wi-Fi and scan the QR code.
- **ZXing-C++ Decoding Engine**: Fast, robust decoding for all common formats:
  - 1D: EAN-13, UPC-A, UPC-E, Code 128, Code 39, Code 93, Codabar, ITF
  - 2D: QR Code, Data Matrix, Aztec, PDF417, MaxiCode
- **Single-Device Exclusive Lock**:
  - Dynamically generated 4-digit PIN prevents unauthorized access.
  - Only **one** device can connect and scan at a time.
  - If a second phone attempts to connect while one is active, it receives an immediate `Session In Use` rejection.
  - Heartbeat tracking automatically releases the lock if the phone disconnects or tab is closed.
- **Hardware-Grade Keyboard Wedge**:
  - Injects keystrokes using Windows `SendInput` (`KEYEVENTF_UNICODE`) to avoid keyboard layout mismatches.
  - Configured with automatic `ENTER` suffix.
  - **Debounce / Cooldown**: Prevents duplicate rapid-fire typing when holding a barcode in view.
- **Mobile Viewfinder**:
  - Rear camera targeting with live scanning laser animation.
  - Flashlight / Torch toggle button.
  - Electronic audio "beep" and haptic vibration feedback on successful scans.

---

## Quick Start (Just Double-Click the .exe!)

1. **Launch the Application**:
   - Double-click **`MobileBarcodeScanner.exe`** directly in the folder.
   - It automatically starts all services (HTTP server, live video HTTPS proxy, and USB port forwarding) in a single application window — no batch files needed!

2. **Connect from Your Phone**:
   - Point your phone camera at the **QR Code** displayed in the terminal window (or navigate to `https://192.168.1.59:8443` on Wi-Fi, or `http://localhost:8080` on USB).
   - On Wi-Fi HTTPS, tap **Advanced** -> **Proceed to 192.168.1.59 (unsafe)** to allow the secure camera context.

3. **Start Live Scanning**:
   - The phone's **live rear video camera** will turn on with the laser targeting reticle.
   - Click into **Notepad**, **Excel**, or any input box on your PC.
   - Hover your phone over any barcode — it will beep and type directly into your active cursor in real time!

---

## PC Terminal Controls

- `[U]` : **Setup / Refresh USB Mode** (runs `adb reverse` and displays USB QR code)
- `[W]` : **Display Wi-Fi QR Code**
- `[K]` : **Kick / Disconnect** the currently connected phone to free up the session slot
- `[C]` : **Clear debounce cache** (allows re-scanning the same barcode immediately)
- `[Q]` : **Quit** the application

---

## Project Structure

```
Barcode/
├── CMakeLists.txt              # CMake build configuration with FetchContent for ZXing-C++
├── run.bat                     # Quick launcher batch file
├── build/
│   └── MobileBarcodeScanner.exe # Self-contained compiled binary (5.2 MB)
└── src/
    ├── main.cpp                # App entry point, IP detection, ASCII QR code, console hotkeys
    ├── SessionManager.h/.cpp   # Single-device PIN auth, session mutex, heartbeat timeout
    ├── BarcodeDecoder.h/.cpp   # ZXing-C++ wrapper with stb_image in-memory decoding
    ├── KeyboardWedge.h/.cpp    # Windows SendInput keystroke injection with debounce
    ├── WebServer.h/.cpp        # Embedded HTTP server handling endpoints (/api/login, /api/scan)
    ├── MobileUI.h              # Single-page mobile web app (HTML5 / WebRTC / AudioContext)
    └── third_party/            # cpp-httplib, stb_image, and Nayuki's qrcodegen
```
