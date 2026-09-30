#include "KeyboardWedge.h"
#include <vector>

static std::wstring Utf8ToWide(const std::string& str) {
    if (str.empty()) return std::wstring();
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

KeyboardWedge::KeyboardWedge(int cooldownMs)
    : m_cooldownMs(cooldownMs),
      m_consecutiveEmptyFrames(10) {}

void KeyboardWedge::SetCooldown(int cooldownMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cooldownMs = cooldownMs;
}

int KeyboardWedge::GetCooldown() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cooldownMs;
}

void KeyboardWedge::ResetCooldown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_history.clear();
    m_consecutiveEmptyFrames = 10;
}

void KeyboardWedge::RegisterNoBarcodeFrame() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_consecutiveEmptyFrames < 100) {
        m_consecutiveEmptyFrames++;
    }
}

WedgeStats KeyboardWedge::GetStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_stats;
}

bool KeyboardWedge::TypeBarcode(const std::string& text, KeySuffix suffix, bool* wasDuplicate, bool bypassCooldown) {
    if (text.empty()) return false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalScanned++;
        auto now = std::chrono::steady_clock::now();

        if (!bypassCooldown) {
            auto it = m_history.find(text);
            if (it != m_history.end()) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
                // Suppress duplicate if cooldown enabled (>0) and (within cooldown window OR if object has not left view)
                if (m_cooldownMs > 0 && (elapsed < m_cooldownMs || m_consecutiveEmptyFrames < 3)) {
                    m_stats.totalDuplicatesBlocked++;
                    if (wasDuplicate) *wasDuplicate = true;
                    // Extend the lock timestamp so resting the camera on the code prevents repeat typing indefinitely
                    it->second = now;
                    m_consecutiveEmptyFrames = 0;
                    return false;
                }
            }
        }

        // Fresh recognized scan
        if (wasDuplicate) *wasDuplicate = false;
        m_history[text] = now;
        m_consecutiveEmptyFrames = 0;
        m_stats.totalInjected++;
        m_stats.lastBarcode = text;
        m_stats.lastInjectedTime = now;

        // Cleanup stale history entries older than 30 seconds
        if (m_history.size() > 50) {
            for (auto hit = m_history.begin(); hit != m_history.end(); ) {
                auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - hit->second).count();
                if (diff > 30) {
                    hit = m_history.erase(hit);
                } else {
                    ++hit;
                }
            }
        }
    }

    KeySuffix effSuffix = (suffix == KeySuffix::Enter) ? m_defaultSuffix : suffix;
    std::wstring wideStr = Utf8ToWide(text);
    SendKeystrokes(wideStr, effSuffix);
    if (m_chimeEnabled) {
        Beep(1200, 90);
    }
    return true;
}

void KeyboardWedge::SendKeystrokes(const std::wstring& wideText, KeySuffix suffix) {
    std::vector<INPUT> inputs;
    inputs.reserve(wideText.size() * 2 + 4);

    for (wchar_t ch : wideText) {
        INPUT keyDown = {};
        keyDown.type = INPUT_KEYBOARD;
        keyDown.ki.wScan = ch;
        keyDown.ki.dwFlags = KEYEVENTF_UNICODE;
        inputs.push_back(keyDown);

        INPUT keyUp = {};
        keyUp.type = INPUT_KEYBOARD;
        keyUp.ki.wScan = ch;
        keyUp.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        inputs.push_back(keyUp);
    }

    WORD vkSuffix = 0;
    if (suffix == KeySuffix::Enter) {
        vkSuffix = VK_RETURN;
    } else if (suffix == KeySuffix::Tab) {
        vkSuffix = VK_TAB;
    }

    if (vkSuffix != 0) {
        INPUT suffixDown = {};
        suffixDown.type = INPUT_KEYBOARD;
        suffixDown.ki.wVk = vkSuffix;
        inputs.push_back(suffixDown);

        INPUT suffixUp = {};
        suffixUp.type = INPUT_KEYBOARD;
        suffixUp.ki.wVk = vkSuffix;
        suffixUp.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(suffixUp);
    }

    if (!inputs.empty()) {
        SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    }
}

void KeyboardWedge::TypeManualText(const std::string& text, KeySuffix suffix) {
    if (text.empty()) return;
    std::wstring wideStr = Utf8ToWide(text);
    SendKeystrokes(wideStr, suffix);
}

void KeyboardWedge::SendSpecialKey(const std::string& keyName) {
    WORD vk = 0;
    if (keyName == "enter") vk = VK_RETURN;
    else if (keyName == "tab") vk = VK_TAB;
    else if (keyName == "backspace") vk = VK_BACK;
    else if (keyName == "escape") vk = VK_ESCAPE;
    else if (keyName == "space") vk = VK_SPACE;
    else if (keyName == "up") vk = VK_UP;
    else if (keyName == "down") vk = VK_DOWN;
    else if (keyName == "left") vk = VK_LEFT;
    else if (keyName == "right") vk = VK_RIGHT;

    if (vk != 0) {
        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wVk = vk;
        inputs[1].type = INPUT_KEYBOARD;
        inputs[1].ki.wVk = vk;
        inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(2, inputs, sizeof(INPUT));
    }
}
