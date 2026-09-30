#pragma once

#include <string>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

enum class KeySuffix {
    None,
    Enter,
    Tab
};

struct WedgeStats {
    uint64_t totalScanned = 0;
    uint64_t totalInjected = 0;
    uint64_t totalDuplicatesBlocked = 0;
    std::string lastBarcode;
    std::chrono::steady_clock::time_point lastInjectedTime;
};

class KeyboardWedge {
public:
    explicit KeyboardWedge(int cooldownMs = 3000);

    // Attempts to type the string at the current cursor location.
    // Returns true if typed, false if ignored due to debounce cooldown.
    // wasDuplicate returns whether this was rejected as a duplicate.
    // bypassCooldown forces immediate injection without checking deduplication.
    bool TypeBarcode(const std::string& text, KeySuffix suffix = KeySuffix::Enter, bool* wasDuplicate = nullptr, bool bypassCooldown = false);

    // Records that a frame contained no barcode (tracks departure from view)
    void RegisterNoBarcodeFrame();

    // Types manual text without debounce suppression
    void TypeManualText(const std::string& text, KeySuffix suffix = KeySuffix::None);

    // Injects a single control key (e.g. "enter", "tab", "backspace")
    void SendSpecialKey(const std::string& keyName);

    void SetCooldown(int cooldownMs);
    int GetCooldown() const;
    void ResetCooldown();

    void SetSuffix(KeySuffix suffix) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_defaultSuffix = suffix;
    }
    KeySuffix GetSuffix() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_defaultSuffix;
    }

    void SetChime(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_chimeEnabled = enable;
    }
    bool IsChimeEnabled() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_chimeEnabled;
    }

    WedgeStats GetStats() const;

private:
    void SendKeystrokes(const std::wstring& wideText, KeySuffix suffix);

    mutable std::mutex m_mutex;
    int m_cooldownMs;
    int m_consecutiveEmptyFrames;
    KeySuffix m_defaultSuffix = KeySuffix::Enter;
    bool m_chimeEnabled = true;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_history;
    WedgeStats m_stats;
};
