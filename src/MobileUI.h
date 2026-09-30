#pragma once

#include <string>

inline std::string GetMobileHtml(const std::string& deviceName = "WORKSTATION-01") {
    std::string html = R"html(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
<title>Barcode Scanner</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@500;700&family=Outfit:wght@400;500;600;700&display=swap" rel="stylesheet">
<style>
  :root {
    --bg-base: #0a0d14;
    --bg-surface: #101520;
    --surface-border: rgba(255, 255, 255, 0.08);
    --surface-border-subtle: rgba(255, 255, 255, 0.04);
    --accent: #10b981;
    --accent-glow: rgba(16, 185, 129, 0.35);
    --warning: #f59e0b;
    --warning-glow: rgba(245, 158, 11, 0.35);
    --cyan: #06b6d4;
    --text-primary: #f8fafc;
    --text-secondary: #94a3b8;
    --text-muted: #64748b;
    --font-sans: 'Outfit', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    --font-mono: 'JetBrains Mono', monospace;
  }

  * {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
    -webkit-tap-highlight-color: transparent;
  }

  body {
    background-color: var(--bg-base);
    color: var(--text-primary);
    font-family: var(--font-sans);
    overflow: hidden;
    height: 100vh;
    height: 100dvh;
    width: 100vw;
    user-select: none;
    -webkit-user-select: none;
  }

  /* 1. AUTH SCREEN */
  #login-view {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    height: 100%;
    padding: 24px;
    background: radial-gradient(circle at 50% 20%, rgba(16, 185, 129, 0.08) 0%, transparent 60%),
                radial-gradient(circle at 80% 80%, rgba(6, 182, 212, 0.05) 0%, transparent 50%),
                var(--bg-base);
  }

  .brand-badge {
    display: inline-flex;
    align-items: center;
    gap: 8px;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid var(--surface-border);
    padding: 6px 14px;
    border-radius: 999px;
    font-size: 12px;
    font-weight: 600;
    color: var(--text-secondary);
    margin-bottom: 24px;
    letter-spacing: 0.5px;
    text-transform: uppercase;
  }

  .login-card {
    background: var(--bg-surface);
    border: 1px solid var(--surface-border);
    border-radius: 20px;
    padding: 32px 24px;
    width: 100%;
    max-width: 360px;
    box-shadow: 0 20px 40px -15px rgba(0, 0, 0, 0.7);
    text-align: center;
  }

  .login-title {
    font-size: 22px;
    font-weight: 700;
    letter-spacing: -0.5px;
    margin-bottom: 8px;
    color: #fff;
  }

  .login-desc {
    font-size: 13px;
    color: var(--text-secondary);
    margin-bottom: 28px;
    line-height: 1.5;
  }

  .pin-input-group {
    display: flex;
    justify-content: center;
    gap: 12px;
    margin-bottom: 24px;
  }

  .pin-digit {
    width: 54px;
    height: 64px;
    border-radius: 12px;
    border: 1.5px solid var(--surface-border);
    background: rgba(255, 255, 255, 0.02);
    font-family: var(--font-mono);
    font-size: 26px;
    font-weight: 700;
    color: #fff;
    text-align: center;
    transition: all 0.15s ease;
    outline: none;
  }

  .pin-digit:focus {
    border-color: var(--accent);
    background: rgba(16, 185, 129, 0.04);
    box-shadow: 0 0 16px var(--accent-glow);
  }

  .btn-primary {
    width: 100%;
    height: 48px;
    background: var(--accent);
    color: #042f1a;
    font-family: var(--font-sans);
    font-size: 15px;
    font-weight: 700;
    border: none;
    border-radius: 12px;
    cursor: pointer;
    transition: all 0.15s ease;
    box-shadow: 0 4px 14px var(--accent-glow);
  }

  .btn-primary:active {
    transform: scale(0.98);
    background: #0ea371;
  }

  .login-error {
    margin-top: 14px;
    font-size: 13px;
    color: #f87171;
    font-weight: 500;
    display: none;
  }

  /* 2. SCANNER VIEWPORT */
  #scanner-view {
    display: none;
    position: relative;
    flex-direction: column;
    height: 100vh;
    height: 100dvh;
    overflow: hidden;
  }

  /* Top Navigation Bar */
  .hud-topbar {
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    z-index: 100;
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: max(16px, env(safe-area-inset-top)) 18px 14px 18px;
    background: linear-gradient(180deg, rgba(10, 13, 20, 0.94) 0%, rgba(10, 13, 20, 0.4) 75%, transparent 100%);
    backdrop-filter: blur(10px);
    -webkit-backdrop-filter: blur(10px);
  }

  .hud-identity {
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .hud-badge-online {
    display: flex;
    align-items: center;
    gap: 6px;
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 700;
    color: var(--accent);
    background: rgba(16, 185, 129, 0.12);
    border: 1px solid rgba(16, 185, 129, 0.25);
    padding: 3px 9px;
    border-radius: 999px;
    letter-spacing: 0.8px;
  }
  .pulse-dot {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: var(--accent);
    box-shadow: 0 0 6px var(--accent);
  }
  .hud-station-tag {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    background: rgba(255, 255, 255, 0.08);
    border: 1px solid var(--surface-border);
    padding: 3px 9px;
    border-radius: 8px;
    font-size: 12px;
    font-weight: 700;
    color: #f1f5f9;
    letter-spacing: 0.5px;
    max-width: 140px;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  .hud-actions {
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .hud-icon-btn {
    width: 40px;
    height: 40px;
    border-radius: 10px;
    border: 1px solid var(--surface-border);
    background: rgba(16, 21, 32, 0.8);
    color: var(--text-secondary);
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    transition: all 0.15s ease;
  }
  .hud-icon-btn:active {
    transform: scale(0.94);
    background: rgba(255, 255, 255, 0.1);
  }
  .hud-icon-btn.active {
    color: var(--accent);
    border-color: rgba(16, 185, 129, 0.35);
    background: rgba(16, 185, 129, 0.12);
  }
  .hud-icon-btn.camera-off {
    color: var(--danger);
    border-color: rgba(239, 68, 68, 0.4);
    background: rgba(239, 68, 68, 0.15);
  }

  /* Camera Container */
  .camera-viewport {
    flex: 1;
    position: relative;
    background: #000;
    overflow: hidden;
  }
  #camera-feed {
    width: 100%;
    height: 100%;
    object-fit: cover;
  }

  /* Industrial Reticle Viewfinder */
  .hud-reticle-wrap {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    width: min(80vw, 300px);
    height: min(50vw, 190px);
    pointer-events: none;
    z-index: 50;
    transition: all 0.2s ease;
  }
  
  .reticle-bracket {
    position: absolute;
    width: 22px;
    height: 22px;
    border-color: var(--accent);
    border-style: solid;
    pointer-events: none;
    transition: all 0.2s ease;
  }
  .bracket-tl { top: 0; left: 0; border-width: 3px 0 0 3px; border-top-left-radius: 8px; }
  .bracket-tr { top: 0; right: 0; border-width: 3px 3px 0 0; border-top-right-radius: 8px; }
  .bracket-bl { bottom: 0; left: 0; border-width: 0 0 3px 3px; border-bottom-left-radius: 8px; }
  .bracket-br { bottom: 0; right: 0; border-width: 0 3px 3px 0; border-bottom-right-radius: 8px; }

  .hud-laser {
    position: absolute;
    left: 0;
    right: 0;
    height: 2px;
    background: linear-gradient(90deg, transparent, var(--accent) 20%, var(--accent) 80%, transparent);
    box-shadow: 0 0 10px var(--accent);
    animation: laserScan 2.4s ease-in-out infinite;
  }
  @keyframes laserScan {
    0% { top: 6%; opacity: 0.3; }
    50% { top: 94%; opacity: 1; }
    100% { top: 6%; opacity: 0.3; }
  }

  .reticle-center-cross {
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    width: 14px;
    height: 14px;
    opacity: 0.4;
  }
  .reticle-center-cross::before, .reticle-center-cross::after {
    content: '';
    position: absolute;
    background: var(--text-secondary);
  }
  .reticle-center-cross::before { top: 6px; left: 0; width: 14px; height: 2px; }
  .reticle-center-cross::after { top: 0; left: 6px; width: 2px; height: 14px; }

  /* Locked / Successful Scan State */
  .reticle-locked .reticle-bracket {
    border-color: #10b981 !important;
    box-shadow: 0 0 20px rgba(16, 185, 129, 0.6);
    transform: scale(1.04);
  }
  /* Duplicate Suppressed State */
  .reticle-duplicate .reticle-bracket {
    border-color: #f59e0b !important;
    box-shadow: 0 0 16px rgba(245, 158, 11, 0.5);
    transform: scale(0.98);
  }

  .reticle-badge {
    position: absolute;
    bottom: -32px;
    left: 50%;
    transform: translateX(-50%);
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 700;
    padding: 4px 12px;
    border-radius: 6px;
    letter-spacing: 0.5px;
    white-space: nowrap;
    text-transform: uppercase;
    pointer-events: none;
    transition: all 0.15s ease;
  }
  .badge-locked {
    background: rgba(16, 185, 129, 0.25);
    border: 1px solid rgba(16, 185, 129, 0.4);
    color: #10b981;
  }
  .badge-dup {
    background: rgba(245, 158, 11, 0.25);
    border: 1px solid rgba(245, 158, 11, 0.4);
    color: #f59e0b;
  }

  /* Scan Success Banner */
  .scan-success-card {
    position: absolute;
    top: max(80px, env(safe-area-inset-top) + 60px);
    left: 18px;
    right: 18px;
    background: rgba(13, 30, 22, 0.95);
    backdrop-filter: blur(20px);
    border: 1px solid var(--accent);
    border-radius: 14px;
    padding: 14px 18px;
    box-shadow: 0 16px 36px rgba(0, 0, 0, 0.6);
    display: none;
    z-index: 150;
    animation: slideDownToast 0.25s cubic-bezier(0.16, 1, 0.3, 1);
  }
  @keyframes slideDownToast {
    from { opacity: 0; transform: translateY(-12px); }
    to { opacity: 1; transform: translateY(0); }
  }
  .scan-tag {
    font-size: 11px;
    font-weight: 700;
    color: var(--accent);
    letter-spacing: 1px;
    display: flex;
    justify-content: space-between;
    margin-bottom: 4px;
  }
  .scan-value {
    font-family: var(--font-mono);
    font-size: 19px;
    font-weight: 700;
    color: #fff;
    word-break: break-all;
  }

  .camera-frozen #hud-laser {
    display: none !important;
    animation: none !important;
  }

  /* Bottom Dock */
  .dock-container {
    background: rgba(12, 16, 24, 0.94);
    backdrop-filter: blur(20px);
    -webkit-backdrop-filter: blur(20px);
    border-top: 1px solid var(--surface-border);
    padding: 14px 18px max(20px, env(safe-area-inset-bottom)) 18px;
    display: flex;
    flex-direction: column;
    gap: 10px;
    z-index: 120;
  }
  .dock-status-row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 12px;
    color: var(--text-secondary);
  }
  .dock-actions-row {
    display: flex;
    gap: 10px;
  }
  .dock-btn {
    flex: 1;
    height: 44px;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid var(--surface-border);
    border-radius: 12px;
    color: var(--text-primary);
    font-family: var(--font-sans);
    font-size: 13px;
    font-weight: 600;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    cursor: pointer;
    transition: all 0.15s ease;
  }
  .dock-btn:active {
    background: rgba(255, 255, 255, 0.1);
    transform: scale(0.98);
  }

  /* 2.5 Freeze & Confirm Thumb-Zone Sheet */
  .confirm-panel {
    position: absolute;
    bottom: 0;
    left: 0;
    right: 0;
    z-index: 160;
    background: rgba(12, 16, 24, 0.96);
    backdrop-filter: blur(24px);
    -webkit-backdrop-filter: blur(24px);
    border-top: 1.5px solid rgba(16, 185, 129, 0.5);
    border-radius: 20px 20px 0 0;
    padding: 16px 18px max(24px, env(safe-area-inset-bottom)) 18px;
    box-shadow: 0 -12px 36px rgba(0, 0, 0, 0.8), 0 0 20px rgba(16, 185, 129, 0.15);
    display: none;
    animation: confirmSlideUp 0.22s cubic-bezier(0.16, 1, 0.3, 1);
  }
  @keyframes confirmSlideUp {
    from { transform: translateY(100%); opacity: 0; }
    to { transform: translateY(0); opacity: 1; }
  }

  .confirm-header-row {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 8px;
  }
  .confirm-tag {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    font-family: var(--font-mono);
    font-size: 11px;
    font-weight: 700;
    color: var(--accent);
    background: rgba(16, 185, 129, 0.12);
    border: 1px solid rgba(16, 185, 129, 0.3);
    padding: 2px 8px;
    border-radius: 6px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
  }
  .confirm-hint {
    font-size: 11px;
    color: var(--text-muted);
    font-weight: 500;
  }
  .confirm-value-box {
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid rgba(255, 255, 255, 0.08);
    border-radius: 12px;
    padding: 12px 14px;
    margin-bottom: 14px;
    max-height: 80px;
    overflow-y: auto;
  }
  .confirm-value-text {
    font-family: var(--font-mono);
    font-size: 18px;
    font-weight: 700;
    color: #fff;
    word-break: break-all;
    user-select: text;
    -webkit-user-select: text;
  }

  .confirm-actions-row {
    display: flex;
    gap: 12px;
  }
  .btn-confirm-cancel {
    flex: 1;
    height: 52px;
    border-radius: 14px;
    background: rgba(255, 255, 255, 0.07);
    border: 1.5px solid var(--surface-border);
    color: var(--text-secondary);
    font-family: var(--font-sans);
    font-size: 15px;
    font-weight: 600;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    cursor: pointer;
    transition: all 0.1s ease;
  }
  .btn-confirm-cancel:active {
    background: rgba(255, 255, 255, 0.12);
    transform: scale(0.97);
  }
  .btn-confirm-continue {
    flex: 1.8;
    height: 52px;
    border-radius: 14px;
    background: linear-gradient(135deg, #10b981 0%, #059669 100%);
    border: none;
    color: #042f1a;
    font-family: var(--font-sans);
    font-size: 16px;
    font-weight: 700;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 8px;
    cursor: pointer;
    box-shadow: 0 4px 18px rgba(16, 185, 129, 0.45);
    transition: all 0.1s ease;
  }
  .btn-confirm-continue:active {
    transform: scale(0.97);
    background: #0ea371;
  }

  .camera-frozen::after {
    content: '';
    position: absolute;
    inset: 0;
    background: rgba(16, 185, 129, 0.05);
    border: 3px solid rgba(16, 185, 129, 0.6);
    pointer-events: none;
    z-index: 40;
    animation: freezeGlow 1.5s ease-in-out infinite alternate;
  }
  @keyframes freezeGlow {
    from { opacity: 0.6; }
    to { opacity: 1; }
  }

  /* 3. MODALS (SETTINGS & HISTORY) */
  .modal-overlay {
    display: none;
    position: fixed;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background: rgba(0, 0, 0, 0.7);
    backdrop-filter: blur(8px);
    -webkit-backdrop-filter: blur(8px);
    z-index: 200;
    align-items: flex-end;
  }
  .modal-sheet {
    background: var(--bg-surface);
    border-top: 1px solid var(--surface-border);
    border-radius: 24px 24px 0 0;
    width: 100%;
    max-height: 85vh;
    padding: 20px 20px max(24px, env(safe-area-inset-bottom)) 20px;
    box-shadow: 0 -20px 40px rgba(0, 0, 0, 0.6);
    overflow-y: auto;
    animation: sheetSlideUp 0.25s cubic-bezier(0.16, 1, 0.3, 1);
  }
  @keyframes sheetSlideUp {
    from { transform: translateY(100%); }
    to { transform: translateY(0); }
  }
  .sheet-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 18px;
  }
  .sheet-title {
    font-size: 16px;
    font-weight: 700;
    color: #fff;
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .sheet-section {
    margin-bottom: 20px;
  }
  .section-label {
    font-size: 11px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.8px;
    color: var(--text-muted);
    margin-bottom: 8px;
  }

  /* Manual Text Injection & Special Keys */
  .input-action-wrap {
    display: flex;
    gap: 8px;
    margin-bottom: 12px;
  }
  .manual-text-field {
    flex: 1;
    height: 44px;
    background: rgba(255, 255, 255, 0.03);
    border: 1px solid var(--surface-border);
    border-radius: 10px;
    padding: 0 14px;
    color: #fff;
    font-family: var(--font-mono);
    font-size: 14px;
    outline: none;
  }
  .manual-text-field:focus {
    border-color: var(--accent);
    background: rgba(16, 185, 129, 0.04);
  }
  .btn-inject {
    height: 44px;
    padding: 0 18px;
    background: var(--accent);
    border: none;
    border-radius: 10px;
    color: #042f1a;
    font-size: 13px;
    font-weight: 700;
    cursor: pointer;
  }
  .btn-inject:active {
    background: #0ea371;
  }

  .control-keys-grid {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 8px;
  }
  .key-pill {
    padding: 12px 6px;
    border-radius: 8px;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid var(--surface-border);
    color: var(--text-secondary);
    font-family: var(--font-mono);
    font-size: 12px;
    font-weight: 600;
    cursor: pointer;
    text-align: center;
    transition: all 0.1s ease;
  }
  .key-pill:active {
    background: rgba(255, 255, 255, 0.15);
    color: #fff;
    transform: translateY(1px);
  }

  .grid-options {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;
  }
  .opt-pill {
    padding: 11px 8px;
    border-radius: 10px;
    background: rgba(255, 255, 255, 0.04);
    border: 1px solid var(--surface-border);
    color: var(--text-secondary);
    font-size: 12px;
    font-weight: 600;
    cursor: pointer;
    text-align: center;
    transition: all 0.15s ease;
  }
  .opt-pill:active {
    transform: scale(0.97);
  }
  .opt-pill.active {
    background: rgba(16, 185, 129, 0.12);
    border-color: rgba(16, 185, 129, 0.35);
    color: var(--accent);
    font-weight: 700;
  }

  /* Diagnostics & Options */
  .settings-row-btn {
    width: 100%;
    padding: 13px 14px;
    border-radius: 10px;
    border: 1px solid var(--surface-border);
    background: rgba(255, 255, 255, 0.04);
    color: var(--text-primary);
    font-size: 13px;
    font-weight: 600;
    display: flex;
    justify-content: space-between;
    align-items: center;
    cursor: pointer;
  }
  .settings-row-btn:active {
    background: rgba(255, 255, 255, 0.08);
  }
  .btn-danger-outline {
    border-color: rgba(239, 68, 68, 0.3);
    color: #f87171;
    background: rgba(239, 68, 68, 0.05);
  }

  /* History Drawer */
  .history-item {
    padding: 12px 14px;
    border-radius: 10px;
    background: rgba(255, 255, 255, 0.03);
    border: 1px solid var(--surface-border-subtle);
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 8px;
  }
  .history-code {
    font-family: var(--font-mono);
    font-size: 14px;
    font-weight: 600;
    color: var(--text-primary);
  }
  .history-meta {
    font-size: 11px;
    color: var(--text-muted);
    margin-top: 2px;
  }
</style>
</head>
<body>

<!-- 1. LOGIN SCREEN -->
<div id="login-view">
  <div class="brand-badge">Target PC: <span class="workstation-name-display" style="color: #fff; margin-left: 4px;">{{DEVICE_NAME}}</span></div>
  <div class="login-card">
    <div class="login-title">Scanner Authentication</div>
    <div class="login-desc">Enter the 4-digit session security PIN displayed on <strong class="workstation-name-display" style="color: var(--accent);">{{DEVICE_NAME}}</strong> to unlock exclusive pairing.</div>
    
    <div class="pin-input-group">
      <input type="tel" maxlength="1" class="pin-digit" id="p1" autofocus inputmode="numeric" pattern="[0-9]*">
      <input type="tel" maxlength="1" class="pin-digit" id="p2" inputmode="numeric" pattern="[0-9]*">
      <input type="tel" maxlength="1" class="pin-digit" id="p3" inputmode="numeric" pattern="[0-9]*">
      <input type="tel" maxlength="1" class="pin-digit" id="p4" inputmode="numeric" pattern="[0-9]*">
    </div>

    <button id="login-btn" class="btn-primary">Connect Scanner</button>
    <div id="login-error" class="login-error"></div>
  </div>
</div>

<!-- 2. SCANNER VIEWPORT -->
<div id="scanner-view">
  <!-- Top Navigation Bar -->
  <div class="hud-topbar">
    <div class="hud-identity">
      <div class="hud-badge-online">
        <span class="pulse-dot"></span>
        <span>LIVE</span>
      </div>
      <div class="hud-station-tag" title="Connected PC Station">
        <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="3" width="20" height="14" rx="2" ry="2"></rect><line x1="8" y1="21" x2="16" y2="21"></line><line x1="12" y1="17" x2="12" y2="21"></line></svg>
        <span class="workstation-name-display">{{DEVICE_NAME}}</span>
      </div>
    </div>

    <div class="hud-actions">
      <!-- Camera Feed Toggle Button (SVG) -->
      <button id="camera-toggle-btn" class="hud-icon-btn active" title="Toggle Camera Power On/Off">
        <svg id="cam-icon-on" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M23 19a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h4l2-3h6l2 3h4a2 2 0 0 1 2 2z"></path>
          <circle cx="12" cy="13" r="4"></circle>
        </svg>
        <svg id="cam-icon-off" style="display: none;" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M1 1l22 22"></path>
          <path d="M21 21H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h3m3-3h6l2 3h4a2 2 0 0 1 2 2v9.34"></path>
          <path d="M14.12 14.12a3 3 0 1 1-4.24-4.24"></path>
        </svg>
      </button>

      <!-- Flashlight / Torch Button (SVG) -->
      <button id="torch-btn" class="hud-icon-btn" title="Toggle Torch">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"></path></svg>
      </button>
      
      <!-- Audio Toggle Button (SVG) -->
      <button id="audio-toggle-btn" class="hud-icon-btn active" title="Toggle Sound">
        <svg id="audio-icon-on" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon><path d="M19.07 4.93a10 10 0 0 1 0 14.14M15.54 8.46a5 5 0 0 1 0 7.07"></path></svg>
        <svg id="audio-icon-off" style="display: none;" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon><line x1="23" y1="9" x2="17" y2="15"></line><line x1="17" y1="9" x2="23" y2="15"></line></svg>
      </button>

      <!-- Settings / Wedge Controls Button (SVG) -->
      <button id="settings-btn" class="hud-icon-btn" title="Settings & Input Injection">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>
      </button>
    </div>
  </div>

  <!-- Real-time Scan Result Toast -->
  <div id="scan-toast" class="scan-success-card">
    <div class="scan-tag">
      <span id="toast-format">BARCODE</span>
      <span id="toast-status-badge">INJECTED</span>
    </div>
    <div id="toast-value" class="scan-value"></div>
  </div>

  <!-- Camera Viewport & Reticle -->
  <div class="camera-viewport">
    <video id="camera-feed" playsinline autoplay muted></video>
    <!-- Instant frame freeze display canvas -->
    <canvas id="frozen-frame-canvas" style="display: none; position: absolute; inset: 0; width: 100%; height: 100%; object-fit: cover; z-index: 30; pointer-events: none;"></canvas>

    <!-- Camera Standby / Off Overlay -->
    <div id="camera-off-overlay" style="display: none; position: absolute; inset: 0; background: rgba(5, 7, 10, 0.94); z-index: 28; flex-direction: column; align-items: center; justify-content: center; gap: 14px; backdrop-filter: blur(8px);">
      <div style="width: 58px; height: 58px; border-radius: 50%; background: rgba(239, 68, 68, 0.12); border: 1px solid rgba(239, 68, 68, 0.3); display: flex; align-items: center; justify-content: center; color: var(--danger);">
        <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M1 1l22 22"></path>
          <path d="M21 21H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h3m3-3h6l2 3h4a2 2 0 0 1 2 2v9.34"></path>
          <path d="M14.12 14.12a3 3 0 1 1-4.24-4.24"></path>
        </svg>
      </div>
      <div style="text-align: center;">
        <div style="font-size: 15px; font-weight: 700; color: #fff; margin-bottom: 4px;">Camera Inactive</div>
        <div style="font-size: 12px; color: var(--text-muted); max-width: 220px; line-height: 1.4;">Hardware sensor paused to save battery & standby.</div>
      </div>
      <button id="resume-cam-btn" style="background: linear-gradient(135deg, var(--accent), #059669); color: #042f1a; border: none; border-radius: 10px; padding: 10px 18px; font-size: 13px; font-weight: 700; display: flex; align-items: center; gap: 8px; cursor: pointer; box-shadow: 0 4px 14px rgba(16, 185, 129, 0.35);">
        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M23 19a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h4l2-3h6l2 3h4a2 2 0 0 1 2 2z"></path>
          <circle cx="12" cy="13" r="4"></circle>
        </svg>
        <span>Turn Camera On</span>
      </button>
    </div>
    
    <div class="hud-reticle-wrap" id="reticle-wrap">
      <div class="reticle-bracket bracket-tl"></div>
      <div class="reticle-bracket bracket-tr"></div>
      <div class="reticle-bracket bracket-bl"></div>
      <div class="reticle-bracket bracket-br"></div>
      <div class="reticle-center-cross"></div>
      <div class="hud-laser" id="hud-laser"></div>
      <div class="reticle-badge" id="reticle-badge" style="display: none;"></div>
    </div>
    
    <canvas id="capture-canvas" style="display: none;"></canvas>
  </div>

  <!-- Native file picker for high-res photo fallback -->
  <input type="file" id="camera-file" accept="image/*" capture="environment" style="display: none;">

  <!-- 2.5 Freeze & Confirm Bottom Thumb-Zone Sheet -->
  <div id="confirm-panel" class="confirm-panel">
    <div class="confirm-header-row">
      <div class="confirm-tag">
        <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"></polyline></svg>
        <span id="confirm-format">BARCODE</span>
      </div>
      <div class="confirm-hint">Station: <strong class="workstation-name-display" style="color: var(--accent);">{{DEVICE_NAME}}</strong></div>
    </div>
    
    <div class="confirm-value-box">
      <div id="confirm-value" class="confirm-value-text"></div>
    </div>

    <div class="confirm-actions-row">
      <button id="confirm-cancel-btn" class="btn-confirm-cancel">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
        <span>Cancel</span>
      </button>

      <button id="confirm-continue-btn" class="btn-confirm-continue">
        <span>Continue</span>
        <svg width="19" height="19" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="9 18 15 12 9 6"></polyline></svg>
      </button>
    </div>
  </div>

  <!-- Bottom Dock -->
  <div class="dock-container">
    <div class="dock-status-row">
      <span id="hud-fps-text">60 FPS Hardware Feed</span>
      <span id="hud-count-text">0 Scans Injected</span>
    </div>
    <div class="dock-actions-row">
      <!-- History Button (SVG) -->
      <button id="history-toggle-btn" class="dock-btn">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="8" y1="6" x2="21" y2="6"></line><line x1="8" y1="12" x2="21" y2="12"></line><line x1="8" y1="18" x2="21" y2="18"></line><line x1="3" y1="6" x2="3.01" y2="6"></line><line x1="3" y1="12" x2="3.01" y2="12"></line><line x1="3" y1="18" x2="3.01" y2="18"></line></svg>
        <span>History</span>
      </button>

      <!-- High-Res Photo Button (SVG) -->
      <button id="snap-btn" class="dock-btn">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M23 19a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h4l2-3h6l2 3h4a2 2 0 0 1 2 2z"></path><circle cx="12" cy="13" r="4"></circle></svg>
        <span>Photo Scan</span>
      </button>
    </div>
  </div>
</div>

<!-- 3. SETTINGS & DEDUPLICATION MODAL -->
<div id="settings-modal" class="modal-overlay">
  <div class="modal-sheet">
    <div class="sheet-header">
      <div class="sheet-title">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>
        <span>Scanner Settings</span>
      </div>
      <button id="close-settings-btn" class="hud-icon-btn" style="width: 32px; height: 32px;">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
      </button>
    </div>

    <!-- Station Target Badge -->
    <div style="background: rgba(16, 185, 129, 0.08); border: 1px solid rgba(16, 185, 129, 0.2); border-radius: 10px; padding: 10px 14px; margin-bottom: 16px; display: flex; align-items: center; justify-content: space-between;">
      <div style="display: flex; align-items: center; gap: 8px;">
        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="#10b981" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2" ry="2"></rect><line x1="8" y1="21" x2="16" y2="21"></line><line x1="12" y1="17" x2="12" y2="21"></line></svg>
        <span style="font-size: 11px; color: var(--text-muted); font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px;">Connected Target PC</span>
      </div>
      <span class="workstation-name-display" style="font-size: 13px; font-weight: 700; color: #10b981; font-family: var(--font-mono);">{{DEVICE_NAME}}</span>
    </div>

    <!-- Section A: Duplicate Scan Filter (Debounce Cooldown) -->
    <div class="sheet-section">
      <div class="section-label">Duplicate Scan Filter (Cooldown)</div>
      <div class="grid-options" id="cooldown-options" style="margin-bottom: 10px;">
        <button class="opt-pill" data-ms="0">Off (0s)</button>
        <button class="opt-pill" data-ms="1500">1.5s Fast</button>
        <button class="opt-pill active" data-ms="3000">3.0s Standard</button>
        <button class="opt-pill" data-ms="5000">5.0s Strict</button>
      </div>

      <!-- Custom Dedup Seconds Input -->
      <div style="display: flex; gap: 8px; align-items: center;">
        <div style="position: relative; flex: 1;">
          <input type="number" id="custom-cooldown-input" class="manual-text-field" placeholder="Custom seconds (e.g. 2.5)" min="0" max="600" step="0.5" style="width: 100%; padding-right: 48px;">
          <span style="position: absolute; right: 12px; top: 50%; transform: translateY(-50%); font-size: 12px; color: var(--text-muted); font-family: var(--font-mono); pointer-events: none;">sec</span>
        </div>
        <button id="btn-set-custom-cooldown" class="btn-inject" style="height: 44px; padding: 0 16px;">Set</button>
      </div>
      <div id="cooldown-status-note" style="font-size: 11px; color: var(--text-muted); margin-top: 6px; line-height: 1.4;">
        Current cooldown: <strong id="active-cd-text" style="color: var(--accent);">3.0s</strong>. Controls how long repeat barcodes are blocked.
      </div>
    </div>

    <!-- Section B: Scanner Mode -->
    <div class="sheet-section">
      <div class="section-label">Scanner Mode</div>
      <div class="grid-options" id="mode-options" style="grid-template-columns: 1fr;">
        <button class="opt-pill active" data-mode="freeze_confirm" style="display: flex; align-items: center; justify-content: space-between; padding: 12px 14px;">
          <span>Freeze & Confirm (Default)</span>
          <span style="font-size: 10px; color: var(--accent); background: rgba(16, 185, 129, 0.15); border: 1px solid rgba(16, 185, 129, 0.3); padding: 2px 6px; border-radius: 4px; font-family: var(--font-mono); font-weight: 700;">RECOMMENDED</span>
        </button>
        <button class="opt-pill" data-mode="continuous" style="padding: 12px 14px; margin-top: 4px;">Instant Auto-Type</button>
      </div>
      <div style="font-size: 11px; color: var(--text-muted); margin-top: 6px; line-height: 1.4;">
        Freeze & Confirm captures the detected frame and holds it on screen for quick thumb approval before typing into PC.
      </div>
    </div>

    <!-- Section C: Camera & Feedback Preferences -->
    <div class="sheet-section">
      <div class="section-label">Device Hardware Controls</div>
      <button id="toggle-camera-setting-btn" class="settings-row-btn" style="padding: 11px 14px; margin-bottom: 8px;">
        <span>Hardware Camera Sensor</span>
        <span id="camera-status-text" style="color: var(--accent); font-weight: 700;">ON (LIVE)</span>
      </button>
      <button id="toggle-haptic-btn" class="settings-row-btn" style="padding: 11px 14px;">
        <span>Haptic Vibration Feedback</span>
        <span id="haptic-status-text" style="color: var(--accent); font-weight: 700;">ENABLED</span>
      </button>
    </div>

    <!-- Section D: Manual Keystroke & Text Injection -->
    <div class="sheet-section">
      <div class="section-label">Manual Text Injection</div>
      <div class="input-action-wrap">
        <input type="text" id="manual-text-input" class="manual-text-field" placeholder="Type text to send to PC cursor...">
        <button id="btn-send-manual-text" class="btn-inject">Send</button>
      </div>
      <div class="control-keys-grid">
        <button class="key-pill" data-key="enter">Enter</button>
        <button class="key-pill" data-key="tab">Tab</button>
        <button class="key-pill" data-key="backspace">Backspace</button>
        <button class="key-pill" data-key="escape">Esc</button>
      </div>
    </div>

    <!-- Section E: Keystroke Diagnostic Test -->
    <div class="sheet-section">
      <div class="section-label">Diagnostic Tools</div>
      <button id="settings-test-pulse-btn" class="settings-row-btn">
        <span>Send Test Keystroke (TEST-BARCODE-12345)</span>
        <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="9 18 15 12 9 6"></polyline></svg>
      </button>
    </div>

    <!-- Section F: Session Disconnect -->
    <div class="sheet-section" style="margin-top: 10px;">
      <button id="settings-disconnect-btn" class="settings-row-btn btn-danger-outline">
        <span>Disconnect Scanner Session</span>
        <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M18.36 6.64a9 9 0 1 1-12.73 0"></path><line x1="12" y1="2" x2="12" y2="12"></line></svg>
      </button>
    </div>
  </div>
</div>

<!-- 4. HISTORY MODAL -->
<div id="history-modal" class="modal-overlay">
  <div class="modal-sheet">
    <div class="sheet-header">
      <div class="sheet-title">
        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="8" y1="6" x2="21" y2="6"></line><line x1="8" y1="12" x2="21" y2="12"></line><line x1="8" y1="18" x2="21" y2="18"></line></svg>
        <span>Session Scan Log</span>
      </div>
      <button id="close-history-btn" class="hud-icon-btn" style="width: 32px; height: 32px;">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
      </button>
    </div>
    <div id="history-list"></div>
  </div>
</div>

<script>
  let sessionToken = null;
  let videoTrack = null;
  let streamInterval = null;
  let isSending = false;
  let isTorchOn = false;
  let isSoundEnabled = true;
  let isHapticEnabled = true;
  let isCameraOn = true;
  let scanMode = "freeze_confirm"; // "freeze_confirm" (default) | "continuous"
  let isFrozen = false;
  let pendingBarcode = null;
  let cooldownDurationMs = 3000;
  
  // Industrial Aim & Lock State Machine
  let lockedBarcode = "";
  let lockExpiresAt = 0;
  let consecutiveEmptyFrames = 0;
  let lastUploadTime = 0;

  let scanCount = 0;
  const recentScans = [];

  // Parse PIN from query params if scanned via QR
  window.addEventListener("DOMContentLoaded", () => {
    const urlParams = new URLSearchParams(window.location.search);
    const pinParam = urlParams.get("pin");
    if (pinParam && pinParam.length === 4) {
      setPinBoxes(pinParam);
      attemptLogin();
    }
    refreshDeviceInfo();
    setInterval(refreshDeviceInfo, 4000);
  });

  // Audio Chime Generator (Web Audio API)
  function playScanChime() {
    if (!isSoundEnabled) return;
    try {
      const audioCtx = new (window.AudioContext || window.webkitAudioContext)();
      const now = audioCtx.currentTime;
      
      const osc = audioCtx.createOscillator();
      const gain = audioCtx.createGain();
      osc.type = "sine";
      osc.frequency.setValueAtTime(1400, now);
      gain.gain.setValueAtTime(0.3, now);
      gain.gain.exponentialRampToValueAtTime(0.01, now + 0.12);
      osc.connect(gain);
      gain.connect(audioCtx.destination);
      osc.start(now);
      osc.stop(now + 0.12);

      const osc2 = audioCtx.createOscillator();
      const gain2 = audioCtx.createGain();
      osc2.type = "sine";
      osc2.frequency.setValueAtTime(2100, now + 0.05);
      gain2.gain.setValueAtTime(0.25, now + 0.05);
      gain2.gain.exponentialRampToValueAtTime(0.01, now + 0.18);
      osc2.connect(gain2);
      gain2.connect(audioCtx.destination);
      osc2.start(now + 0.05);
      osc2.stop(now + 0.18);
    } catch(e) {}
  }

  function triggerHaptic(pattern) {
    if (typeof isHapticEnabled !== "undefined" && !isHapticEnabled) return;
    if (!navigator.vibrate) return;
    try { navigator.vibrate(pattern); } catch(e) {}
  }

  function showToast(text, format, isDuplicate) {
    const toast = document.getElementById("scan-toast");
    document.getElementById("toast-format").textContent = format || "BARCODE";
    const badge = document.getElementById("toast-status-badge");
    badge.textContent = isDuplicate ? "DUP BLOCKED" : "INJECTED";
    badge.style.color = isDuplicate ? "var(--warning)" : "var(--accent)";
    document.getElementById("toast-value").textContent = text;
    
    toast.style.display = "block";
    clearTimeout(toast.timer);
    toast.timer = setTimeout(() => { toast.style.display = "none"; }, 2500);

    if (!isDuplicate) {
      recentScans.unshift({ text, format, time: new Date().toLocaleTimeString() });
      if (recentScans.length > 30) recentScans.pop();
      updateHistoryUI();
    }
  }

  function updateHistoryUI() {
    const list = document.getElementById("history-list");
    if (!recentScans.length) {
      list.innerHTML = "<div style='color: var(--text-muted); font-size: 13px; text-align: center; padding: 24px;'>No barcodes scanned yet.</div>";
      return;
    }
    list.innerHTML = recentScans.map(item => `
      <div class="history-item">
        <div>
          <div class="history-code">${item.text}</div>
          <div class="history-meta">${item.format} • ${item.time}</div>
        </div>
        <div style="color: var(--accent); font-size: 12px; font-weight: 700;">Injected</div>
      </div>
    `).join("");
  }

  function showError(msg) {
    const errBox = document.getElementById("login-error");
    errBox.textContent = msg;
    errBox.style.display = "block";
  }

  // 4-Box PIN Input Logic
  const pinBoxes = [document.getElementById("p1"), document.getElementById("p2"), document.getElementById("p3"), document.getElementById("p4")];
  pinBoxes.forEach((box, idx) => {
    box.addEventListener("input", () => {
      if (box.value.length === 1 && idx < 3) {
        pinBoxes[idx + 1].focus();
      }
      if (idx === 3 && box.value.length === 1) {
        attemptLogin();
      }
    });
    box.addEventListener("keydown", (e) => {
      if (e.key === "Backspace" && !box.value && idx > 0) {
        pinBoxes[idx - 1].focus();
      }
    });
  });

  function getEnteredPin() {
    return pinBoxes.map(b => b.value.trim()).join("");
  }

  function setPinBoxes(pinStr) {
    if (!pinStr) return;
    for (let i = 0; i < 4; i++) {
      if (pinBoxes[i]) pinBoxes[i].value = pinStr[i] || "";
    }
  }

  async function attemptLogin() {
    const pin = getEnteredPin();
    if (pin.length !== 4) {
      showError("Please enter the complete 4-digit PIN.");
      return;
    }

    try {
      const resp = await fetch("/api/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ pin })
      });
      const data = await resp.json();

      if (resp.ok && data.status === "ok") {
        sessionToken = data.token;
        document.getElementById("login-view").style.display = "none";
        document.getElementById("scanner-view").style.display = "flex";
        startScanner();
        startHeartbeat();
      } else {
        showError(data.message || "Authentication failed. Try again.");
      }
    } catch (err) {
      showError("Connection failed. Check server connection.");
    }
  }

  document.getElementById("login-btn").addEventListener("click", attemptLogin);

  async function startScanner() {
    if (!isCameraOn) return;

    if (streamInterval) {
      clearInterval(streamInterval);
      streamInterval = null;
    }
    if (videoTrack) {
      try { videoTrack.stop(); } catch(e) {}
      videoTrack = null;
    }

    const video = document.getElementById("camera-feed");
    const canvas = document.getElementById("capture-canvas");
    const ctx = canvas.getContext("2d", { willReadFrequently: true });
    const reticleWrap = document.getElementById("reticle-wrap");
    const reticleBadge = document.getElementById("reticle-badge");
    const hudLaser = document.getElementById("hud-laser");

    try {
      const constraints = {
        video: {
          facingMode: { ideal: "environment" },
          width: { ideal: 1280 },
          height: { ideal: 720 }
        },
        audio: false
      };

      const stream = await navigator.mediaDevices.getUserMedia(constraints);
      video.srcObject = stream;
      videoTrack = stream.getVideoTracks()[0];
      await video.play();

      document.getElementById("hud-fps-text").textContent = "60 FPS Live Feed";

      const camBtn = document.getElementById("camera-toggle-btn");
      if (camBtn) {
        camBtn.classList.add("active");
        camBtn.classList.remove("camera-off");
        document.getElementById("cam-icon-on").style.display = "block";
        document.getElementById("cam-icon-off").style.display = "none";
      }
      const camSettingTxt = document.getElementById("camera-status-text");
      if (camSettingTxt) {
        camSettingTxt.textContent = "ON (LIVE)";
        camSettingTxt.style.color = "var(--accent)";
      }
      const overlay = document.getElementById("camera-off-overlay");
      if (overlay) overlay.style.display = "none";
      if (hudLaser && !isFrozen) hudLaser.style.display = "block";

      // Frame capture loop
      streamInterval = setInterval(() => {
        if (!isCameraOn || !video.videoWidth || !video.videoHeight || isSending || isFrozen) return;

        const now = Date.now();
        // If locked on a barcode, throttle checks to 350ms to conserve battery & bandwidth
        if (lockedBarcode !== "" && now < lockExpiresAt && (now - lastUploadTime < 350)) {
          return;
        }

        canvas.width = video.videoWidth;
        canvas.height = video.videoHeight;
        ctx.drawImage(video, 0, 0, canvas.width, canvas.height);

        canvas.toBlob(async (blob) => {
          if (!blob || isFrozen || !isCameraOn) return;
          isSending = true;
          lastUploadTime = Date.now();

          try {
            const autoInject = (scanMode === "continuous") ? 1 : 0;
            const resp = await fetch(`/api/scan?token=${encodeURIComponent(sessionToken)}&auto_inject=${autoInject}`, {
              method: "POST",
              headers: { 
                "Content-Type": "image/jpeg",
                "X-Session-Token": sessionToken || ""
              },
              body: blob
            });

            if (resp.status === 401) {
              handleKicked("Session expired or disconnected.");
              return;
            }

            const data = await resp.json();
            if (data.scanned) {
              consecutiveEmptyFrames = 0;

              if (data.needsConfirm) {
                // Freeze screen immediately with exact captured frame for operator review
                freezeScanner(data.text, data.format);
              } else if (data.injected) {
                // 1. FRESH SUCCESSFUL SCAN INJECTED INTO PC
                scanCount++;
                document.getElementById("hud-count-text").textContent = `${scanCount} Scans Injected`;
                lockedBarcode = data.text;
                lockExpiresAt = Date.now() + cooldownDurationMs;

                // Visual reticle lock
                reticleWrap.className = "hud-reticle-wrap reticle-locked";
                reticleBadge.textContent = "✔ INJECTED";
                reticleBadge.className = "reticle-badge badge-locked";
                reticleBadge.style.display = "block";
                hudLaser.style.opacity = "0";

                playScanChime();
                triggerHaptic([60]);
                showToast(data.text, data.format, false);

                setTimeout(() => {
                  if (!isFrozen) hudLaser.style.opacity = "1";
                }, 600);
              } else if (data.duplicate) {
                // 2. DUPLICATE SCAN HELD IN VIEW (SUPPRESSED)
                lockedBarcode = data.text;
                lockExpiresAt = Date.now() + cooldownDurationMs;

                reticleWrap.className = "hud-reticle-wrap reticle-duplicate";
                reticleBadge.textContent = "DUP FILTERED";
                reticleBadge.className = "reticle-badge badge-dup";
                reticleBadge.style.display = "block";

                triggerHaptic([25, 25]);
                showToast(data.text, data.format, true);
              }
            } else {
              // 3. NO BARCODE DETECTED IN THIS FRAME
              consecutiveEmptyFrames++;
              if (consecutiveEmptyFrames >= 3) {
                // Object has left camera view! Clear lock and re-arm
                if (lockedBarcode !== "") {
                  lockedBarcode = "";
                  lockExpiresAt = 0;
                  reticleWrap.className = "hud-reticle-wrap";
                  reticleBadge.style.display = "none";
                  hudLaser.style.opacity = "1";
                }
              }
            }
          } catch (e) {
          } finally {
            isSending = false;
          }
        }, "image/jpeg", 0.65);
      }, 75);

    } catch (err) {
      document.getElementById("hud-fps-text").textContent = "Camera Fallback";
      document.getElementById("snap-btn").style.borderColor = "var(--cyan)";
    }
  }

  // Camera Power Controls (On / Off / Standby)
  async function stopCamera() {
    isCameraOn = false;

    if (streamInterval) {
      clearInterval(streamInterval);
      streamInterval = null;
    }

    if (isTorchOn && videoTrack) {
      try {
        await videoTrack.applyConstraints({ advanced: [{ torch: false }] });
      } catch(e) {}
      isTorchOn = false;
      const torchBtn = document.getElementById("torch-btn");
      if (torchBtn) torchBtn.classList.remove("active");
    }

    if (videoTrack) {
      try { videoTrack.stop(); } catch(e) {}
      videoTrack = null;
    }
    const video = document.getElementById("camera-feed");
    if (video) {
      try { video.pause(); } catch(e) {}
      video.srcObject = null;
    }

    if (isFrozen) {
      unfreezeScanner();
    }

    const camBtn = document.getElementById("camera-toggle-btn");
    if (camBtn) {
      camBtn.classList.remove("active");
      camBtn.classList.add("camera-off");
      document.getElementById("cam-icon-on").style.display = "none";
      document.getElementById("cam-icon-off").style.display = "block";
    }

    const camSettingTxt = document.getElementById("camera-status-text");
    if (camSettingTxt) {
      camSettingTxt.textContent = "OFF (STANDBY)";
      camSettingTxt.style.color = "var(--danger)";
    }

    const hudLaser = document.getElementById("hud-laser");
    if (hudLaser) hudLaser.style.display = "none";

    const overlay = document.getElementById("camera-off-overlay");
    if (overlay) overlay.style.display = "flex";

    document.getElementById("hud-fps-text").textContent = "Camera Off (Standby)";

    triggerHaptic([30]);
  }

  async function resumeCamera() {
    isCameraOn = true;
    const overlay = document.getElementById("camera-off-overlay");
    if (overlay) overlay.style.display = "none";

    await startScanner();
    triggerHaptic([40]);
  }

  async function toggleCamera() {
    if (isCameraOn) {
      await stopCamera();
    } else {
      await resumeCamera();
    }
  }

  // Freeze & Confirm Handlers
  function freezeScanner(text, format) {
    if (isFrozen) return;
    isFrozen = true;
    pendingBarcode = { text, format };

    // 1. Instant capture of the exact decoded frame onto our freeze display canvas
    const video = document.getElementById("camera-feed");
    const canvas = document.getElementById("capture-canvas");
    const frozenCanvas = document.getElementById("frozen-frame-canvas");
    if (frozenCanvas && canvas && canvas.width > 0) {
      frozenCanvas.width = canvas.width;
      frozenCanvas.height = canvas.height;
      const fCtx = frozenCanvas.getContext("2d");
      fCtx.drawImage(canvas, 0, 0);
      frozenCanvas.style.display = "block";
    }

    try { video.pause(); } catch(e) {}
    document.querySelector(".camera-viewport").classList.add("camera-frozen");

    const reticleWrap = document.getElementById("reticle-wrap");
    const reticleBadge = document.getElementById("reticle-badge");
    const hudLaser = document.getElementById("hud-laser");

    reticleWrap.className = "hud-reticle-wrap reticle-locked";
    reticleBadge.textContent = "REVIEW CODE";
    reticleBadge.className = "reticle-badge badge-locked";
    reticleBadge.style.display = "block";
    
    // Stop and hide the scan line animation completely while frozen
    if (hudLaser) {
      hudLaser.style.display = "none";
    }

    document.getElementById("confirm-format").textContent = format || "BARCODE";
    document.getElementById("confirm-value").textContent = text;
    document.getElementById("confirm-panel").style.display = "block";

    triggerHaptic([40, 40]);
  }

  function unfreezeScanner() {
    isFrozen = false;
    pendingBarcode = null;
    document.getElementById("confirm-panel").style.display = "none";
    document.querySelector(".camera-viewport").classList.remove("camera-frozen");

    const frozenCanvas = document.getElementById("frozen-frame-canvas");
    if (frozenCanvas) {
      frozenCanvas.style.display = "none";
    }

    const reticleWrap = document.getElementById("reticle-wrap");
    const reticleBadge = document.getElementById("reticle-badge");
    const hudLaser = document.getElementById("hud-laser");

    reticleWrap.className = "hud-reticle-wrap";
    reticleBadge.style.display = "none";
    if (hudLaser) {
      hudLaser.style.display = "block";
    }

    const video = document.getElementById("camera-feed");
    try { video.play(); } catch(e) {}
  }

  async function handleConfirmContinue() {
    if (!pendingBarcode) return;
    const item = pendingBarcode;

    const continueBtn = document.getElementById("confirm-continue-btn");
    continueBtn.disabled = true;
    continueBtn.style.opacity = "0.7";

    try {
      const resp = await fetch(`/api/confirm_scan?token=${encodeURIComponent(sessionToken)}`, {
        method: "POST",
        headers: { 
          "Content-Type": "application/json",
          "X-Session-Token": sessionToken || ""
        },
        body: JSON.stringify({ text: item.text, token: sessionToken })
      });
      
      if (!resp.ok) {
        const errText = await resp.text();
        let errMsg = "Server error " + resp.status;
        try {
          const errObj = JSON.parse(errText);
          if (errObj.message) errMsg = errObj.message;
        } catch(e) {}
        alert("PC Error: " + errMsg);
        return;
      }

      const data = await resp.json();
      if (data.injected) {
        scanCount++;
        document.getElementById("hud-count-text").textContent = `${scanCount} Scans Injected`;
        try { playScanChime(); } catch(e) {}
        try { triggerHaptic([60]); } catch(e) {}
        try { showToast(item.text, item.format, false); } catch(e) {}

        // Cooldown buffer so user moving camera away doesn't re-trigger same code
        lockedBarcode = item.text;
        lockExpiresAt = Date.now() + cooldownDurationMs;
      }
    } catch(err) {
      console.error("Confirm scan failed:", err);
      if (err && err.name !== "AbortError") {
        alert("Connection error: " + (err.message || "Failed to reach PC"));
      }
    } finally {
      continueBtn.disabled = false;
      continueBtn.style.opacity = "1";
      unfreezeScanner();
    }
  }

  function handleConfirmCancel() {
    triggerHaptic([25]);
    if (pendingBarcode) {
      lockedBarcode = pendingBarcode.text;
      lockExpiresAt = Date.now() + 1500;
    }
    unfreezeScanner();
  }

  document.getElementById("confirm-continue-btn").addEventListener("click", handleConfirmContinue);
  document.getElementById("confirm-cancel-btn").addEventListener("click", handleConfirmCancel);

  window.addEventListener("keydown", (e) => {
    if (isFrozen) {
      if (e.key === "Enter") {
        e.preventDefault();
        handleConfirmContinue();
      } else if (e.key === "Escape") {
        e.preventDefault();
        handleConfirmCancel();
      }
    }
  });

  // Fallback high-res photo capture
  const fileInput = document.getElementById("camera-file");
  document.getElementById("snap-btn").addEventListener("click", () => fileInput.click());
  fileInput.addEventListener("change", async (e) => {
    const file = e.target.files[0];
    if (!file) return;

    try {
      const autoInject = (scanMode === "continuous" || scanMode === "trigger") ? 1 : 0;
      const resp = await fetch(`/api/scan?token=${encodeURIComponent(sessionToken)}&auto_inject=${autoInject}`, {
        method: "POST",
        headers: { "Content-Type": file.type || "image/jpeg" },
        body: file
      });
      const data = await resp.json();
      if (data.scanned) {
        if (data.needsConfirm) {
          freezeScanner(data.text, data.format);
        } else if (data.injected) {
          scanCount++;
          document.getElementById("hud-count-text").textContent = `${scanCount} Scans Injected`;
          showToast(data.text, data.format, false);
          playScanChime();
          triggerHaptic([60]);
        } else {
          showToast(data.text, data.format, true);
          triggerHaptic([25, 25]);
        }
      } else {
        alert("No readable barcode detected in photo.");
      }
    } catch(err) {
      alert("Scan upload failed.");
    }
  });

  // Camera Toggle
  document.getElementById("camera-toggle-btn").addEventListener("click", toggleCamera);
  document.getElementById("resume-cam-btn").addEventListener("click", resumeCamera);
  const toggleCamSettingBtn = document.getElementById("toggle-camera-setting-btn");
  if (toggleCamSettingBtn) {
    toggleCamSettingBtn.addEventListener("click", toggleCamera);
  }

  // Torch Toggle
  document.getElementById("torch-btn").addEventListener("click", async () => {
    if (!videoTrack) return;
    const btn = document.getElementById("torch-btn");
    try {
      const caps = videoTrack.getCapabilities();
      if (caps && caps.torch) {
        isTorchOn = !isTorchOn;
        await videoTrack.applyConstraints({ advanced: [{ torch: isTorchOn }] });
        btn.classList.toggle("active", isTorchOn);
      }
    } catch(e) {}
  });

  // Audio Toggle
  document.getElementById("audio-toggle-btn").addEventListener("click", () => {
    isSoundEnabled = !isSoundEnabled;
    const btn = document.getElementById("audio-toggle-btn");
    btn.classList.toggle("active", isSoundEnabled);
    document.getElementById("audio-icon-on").style.display = isSoundEnabled ? "block" : "none";
    document.getElementById("audio-icon-off").style.display = isSoundEnabled ? "none" : "block";
  });

  // Modals Management
  const settingsModal = document.getElementById("settings-modal");
  const historyModal = document.getElementById("history-modal");

  document.getElementById("settings-btn").addEventListener("click", () => {
    settingsModal.style.display = "flex";
  });
  document.getElementById("close-settings-btn").addEventListener("click", () => {
    settingsModal.style.display = "none";
  });

  document.getElementById("history-toggle-btn").addEventListener("click", () => {
    historyModal.style.display = "flex";
  });
  document.getElementById("close-history-btn").addEventListener("click", () => {
    historyModal.style.display = "none";
  });

  [settingsModal, historyModal].forEach(m => {
    m.addEventListener("click", (e) => {
      if (e.target === m) m.style.display = "none";
    });
  });

  // Cooldown Option Selector
  // Cooldown Option Selector
  async function applyCooldown(ms) {
    cooldownDurationMs = ms;
    document.querySelectorAll("#cooldown-options .opt-pill").forEach(p => {
      const pMs = parseInt(p.getAttribute("data-ms"), 10);
      p.classList.toggle("active", pMs === ms);
    });
    const secVal = (ms / 1000).toFixed(ms % 1000 === 0 ? 1 : 2);
    const activeText = document.getElementById("active-cd-text");
    if (activeText) activeText.textContent = ms === 0 ? "Off (0s)" : `${secVal}s`;
    const customInp = document.getElementById("custom-cooldown-input");
    if (customInp) customInp.value = (ms / 1000);
    try {
      await fetch(`/api/set_cooldown?ms=${ms}`, { method: "POST" });
    } catch(e) {}
    triggerHaptic([35]);
  }

  document.querySelectorAll("#cooldown-options .opt-pill").forEach(pill => {
    pill.addEventListener("click", () => {
      const ms = parseInt(pill.getAttribute("data-ms"), 10);
      applyCooldown(ms);
    });
  });

  const customCdBtn = document.getElementById("btn-set-custom-cooldown");
  if (customCdBtn) {
    customCdBtn.addEventListener("click", () => {
      const inp = document.getElementById("custom-cooldown-input");
      const sec = parseFloat(inp.value);
      if (!isNaN(sec) && sec >= 0 && sec <= 600) {
        applyCooldown(Math.round(sec * 1000));
      } else {
        alert("Please enter a valid cooldown between 0 and 600 seconds.");
      }
    });
  }

  // Scanner Mode Selector
  document.querySelectorAll("#mode-options .opt-pill").forEach(pill => {
    pill.addEventListener("click", () => {
      document.querySelectorAll("#mode-options .opt-pill").forEach(p => p.classList.remove("active"));
      pill.classList.add("active");
      scanMode = pill.getAttribute("data-mode");
      triggerHaptic([35]);
    });
  });

  // Haptic Toggle
  const toggleHapticBtn = document.getElementById("toggle-haptic-btn");
  toggleHapticBtn.addEventListener("click", () => {
    isHapticEnabled = !isHapticEnabled;
    const txt = document.getElementById("haptic-status-text");
    txt.textContent = isHapticEnabled ? "ENABLED" : "MUTED";
    txt.style.color = isHapticEnabled ? "var(--accent)" : "var(--text-muted)";
    if (isHapticEnabled) triggerHaptic([50]);
  });

  // Manual Text Sending
  async function sendManualText() {
    const input = document.getElementById("manual-text-input");
    const text = input.value.trim();
    if (!text) return;

    try {
      const resp = await fetch(`/api/send_input?token=${encodeURIComponent(sessionToken)}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ text, suffix: "enter" })
      });
      if (resp.ok) {
        showToast(text, "MANUAL INPUT", false);
        input.value = "";
        settingsModal.style.display = "none";
      }
    } catch(e) {}
  }

  document.getElementById("btn-send-manual-text").addEventListener("click", sendManualText);
  document.getElementById("manual-text-input").addEventListener("keydown", (e) => {
    if (e.key === "Enter") sendManualText();
  });

  // Special Control Keys
  document.querySelectorAll(".key-pill").forEach(pill => {
    pill.addEventListener("click", async () => {
      const key = pill.getAttribute("data-key");
      try {
        await fetch(`/api/send_input?token=${encodeURIComponent(sessionToken)}`, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ key })
        });
        showToast(key.toUpperCase(), "KEY EVENT", false);
      } catch(e) {}
    });
  });

  // Settings Diagnostic Test Pulse
  document.getElementById("settings-test-pulse-btn").addEventListener("click", async () => {
    try {
      const resp = await fetch(`/api/test_type?token=${encodeURIComponent(sessionToken)}`, { method: "POST" });
      const data = await resp.json();
      if (data.typed) {
        showToast(data.text, "TEST PULSE", false);
        playScanChime();
        settingsModal.style.display = "none";
      }
    } catch(e) {}
  });

  // Disconnect
  document.getElementById("settings-disconnect-btn").addEventListener("click", async () => {
    if (confirm("Disconnect scanner session?")) {
      try {
        await fetch(`/api/logout?token=${encodeURIComponent(sessionToken)}`, { method: "POST" });
      } catch(e) {}
      window.location.reload();
    }
  });

  function startHeartbeat() {
    setInterval(async () => {
      if (!sessionToken) return;
      try {
        const resp = await fetch(`/api/heartbeat?token=${encodeURIComponent(sessionToken)}`, { method: "POST" });
        if (resp.status === 401) handleKicked("Session expired.");
      } catch(e) {}
    }, 3000);
  }

  function handleKicked(msg) {
    if (streamInterval) clearInterval(streamInterval);
    if (videoTrack) videoTrack.stop();
    alert(msg || "Session disconnected.");
    window.location.reload();
  }

  async function refreshDeviceInfo() {
    try {
      const resp = await fetch("/api/info");
      const data = await resp.json();
      if (data.deviceName) {
        document.querySelectorAll(".workstation-name-display").forEach(el => el.textContent = data.deviceName);
      }
    } catch(e) {}
  }
</script>
</body>
</html>)html";

    // Inject workstation device name
    size_t pos = 0;
    std::string placeholder = "{{DEVICE_NAME}}";
    while ((pos = html.find(placeholder, pos)) != std::string::npos) {
        html.replace(pos, placeholder.length(), deviceName);
        pos += deviceName.length();
    }

    return html;
}
