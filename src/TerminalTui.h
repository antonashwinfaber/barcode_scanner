#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>

#include "SessionManager.h"
#include "KeyboardWedge.h"
#include "AppLogger.h"
#include "CloudflareTunnel.h"
#include "third_party/qrcodegen.hpp"

using qrcodegen::QrCode;

enum class ActiveMode {
    LiveWifiHttps,
    CloudflareTunnel,
    UsbCable,
    LanHttp
};

enum class TuiTab {
    Dashboard = 0,
    Logs = 1,
    Config = 2,
    Diagnostics = 3
};

class TerminalTui {
public:
    TerminalTui(
        SessionManager& sessionMgr,
        KeyboardWedge& wedge,
        CloudflareTunnel& tunnel,
        const std::string& exeDir,
        int port
    ) : m_sessionMgr(sessionMgr),
        m_wedge(wedge),
        m_tunnel(tunnel),
        m_exeDir(exeDir),
        m_port(port),
        m_currentTab(TuiTab::Dashboard),
        m_activeMode(ActiveMode::LiveWifiHttps),
        m_configRow(0),
        m_logScrollOffset(0),
        m_logFilter(0),
        m_tabChanged(true),
        m_quitRequested(false),
        m_origInMode(0),
        m_lastMouseButtons(0)
    {}

    void AutoSizeConsole(int targetCols, int targetRows) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return;

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return;

        int curCols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int curRows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        int newCols = (std::max)(curCols, targetCols);
        int newRows = (std::max)(curRows, targetRows);

        // Adjust buffer size first
        COORD bufferSize;
        bufferSize.X = static_cast<SHORT>((std::max)((int)csbi.dwSize.X, newCols));
        bufferSize.Y = static_cast<SHORT>((std::max)((int)csbi.dwSize.Y, 500));
        SetConsoleScreenBufferSize(hOut, bufferSize);

        // Adjust window size
        SMALL_RECT windowSize;
        windowSize.Left = 0;
        windowSize.Top = 0;
        windowSize.Right = static_cast<SHORT>(newCols - 1);
        windowSize.Bottom = static_cast<SHORT>(newRows - 1);
        SetConsoleWindowInfo(hOut, TRUE, &windowSize);
    }

    void Init() {
        SetConsoleTitleA("John's Barcode");
        AutoSizeConsole(102, 32);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | 0x0004 /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */);
        }
        SetConsoleOutputCP(CP_UTF8);

        // Enable Mouse Input & disable QuickEdit mode (which intercepts mouse clicks)
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (GetConsoleMode(hIn, &m_origInMode)) {
            SetConsoleMode(hIn, (m_origInMode & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
        }

        // Switch to alternate screen buffer, clear screen, hide cursor
        std::cout << "\033[?1049h\033[2J\033[H\033[?25l" << std::flush;
        AppLogger::Instance().SetDirectPrint(false);
    }

    void Shutdown() {
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (m_origInMode != 0) {
            SetConsoleMode(hIn, m_origInMode);
        }
        // Show cursor, restore main screen buffer
        std::cout << "\033[?25h\033[?1049l\033[0m\n" << std::flush;
        AppLogger::Instance().SetDirectPrint(true);
    }

    void SetDeviceName(const std::string& name) { m_deviceName = name; }
    std::string GetDeviceName() const { return m_deviceName; }
    void SetLocalIp(const std::string& ip) { m_localIp = ip; }
    void SetActiveMode(ActiveMode mode) { m_activeMode = mode; }
    ActiveMode GetActiveMode() const { return m_activeMode; }

    void SetTab(TuiTab tab) {
        if (m_currentTab != tab) {
            m_currentTab = tab;
            m_tabChanged = true;
        }
    }

    bool IsQuitRequested() const { return m_quitRequested; }

    void Render() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (cols < 80) cols = 80;
        if (rows < 25) rows = 25;

        std::ostringstream out;

        // If tab just switched, do a complete screen reset to prevent any bleed-through
        if (m_tabChanged) {
            m_tabChanged = false;
            out << "\033[2J\033[H";
        } else {
            out << "\033[H";
        }

        // 1. Top Window Tab Bar (Row 0)
        RenderTabBar(out, cols);

        // 2. Active Window Content (Rows 1 to 22: Top border + exactly 20 content lines + Bottom border)
        switch (m_currentTab) {
            case TuiTab::Dashboard:
                RenderDashboardWindow(out, cols, rows);
                break;
            case TuiTab::Logs:
                RenderLogsWindow(out, cols, rows);
                break;
            case TuiTab::Config:
                RenderConfigWindow(out, cols, rows);
                break;
            case TuiTab::Diagnostics:
                RenderDiagnosticsWindow(out, cols, rows);
                break;
        }

        // 3. Bottom Hotkey Status Bar (Row 23)
        RenderBottomBar(out, cols, rows);

        // 4. Erase any ghost content below the status bar down to buffer bottom
        out << "\033[J";

        std::cout << out.str() << std::flush;
    }

    bool HandleKey(int ch, int extCode = 0) {
        if (extCode != 0) {
            if (extCode == 72) { // Up Arrow
                if (m_currentTab == TuiTab::Config) {
                    if (m_configRow > 0) m_configRow--;
                    return true;
                } else if (m_currentTab == TuiTab::Logs) {
                    if (m_logScrollOffset > 0) m_logScrollOffset--;
                    return true;
                }
            } else if (extCode == 80) { // Down Arrow
                if (m_currentTab == TuiTab::Config) {
                    if (m_configRow < 6) m_configRow++;
                    return true;
                } else if (m_currentTab == TuiTab::Logs) {
                    m_logScrollOffset++;
                    return true;
                }
            } else if (extCode == 75) { // Left Arrow
                if (m_currentTab == TuiTab::Config) {
                    CycleConfigValue(-1);
                    return true;
                }
            } else if (extCode == 77) { // Right Arrow
                if (m_currentTab == TuiTab::Config) {
                    CycleConfigValue(1);
                    return true;
                }
            } else if (extCode == 73) { // Page Up
                if (m_currentTab == TuiTab::Logs) {
                    m_logScrollOffset = (m_logScrollOffset > 10) ? m_logScrollOffset - 10 : 0;
                    return true;
                }
            } else if (extCode == 81) { // Page Down
                if (m_currentTab == TuiTab::Logs) {
                    m_logScrollOffset += 10;
                    return true;
                }
            } else if (extCode == 71) { // Home
                if (m_currentTab == TuiTab::Logs) {
                    m_logScrollOffset = 0;
                    return true;
                }
            } else if (extCode == 59) { SetTab(TuiTab::Dashboard); return true; } // F1
            else if (extCode == 60) { SetTab(TuiTab::Logs); return true; }      // F2
            else if (extCode == 61) { SetTab(TuiTab::Config); return true; }    // F3
            else if (extCode == 62) { SetTab(TuiTab::Diagnostics); return true; } // F4
            return false;
        }

        switch (ch) {
            case '\t':
                SetTab(static_cast<TuiTab>((static_cast<int>(m_currentTab) + 1) % 4));
                return true;
            case '1':
                SetTab(TuiTab::Dashboard);
                return true;
            case '2':
                SetTab(TuiTab::Logs);
                return true;
            case '3':
                SetTab(TuiTab::Config);
                return true;
            case '4':
                SetTab(TuiTab::Diagnostics);
                return true;
            case '\r':
            case '\n':
                if (m_currentTab == TuiTab::Config) {
                    ActivateConfigAction();
                    return true;
                }
                break;
            case ' ':
                if (m_currentTab == TuiTab::Config) {
                    CycleConfigValue(1);
                    return true;
                }
                break;
            case 'f':
            case 'F':
                if (m_currentTab == TuiTab::Logs) {
                    m_logFilter = (m_logFilter + 1) % 4;
                    m_logScrollOffset = 0;
                    return true;
                }
                break;
        }
        return false;
    }

    bool HandleMouse(int mouseX, int mouseY, DWORD buttonState, DWORD eventFlags) {
        bool leftDown = (buttonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
        bool leftClicked = leftDown && ((m_lastMouseButtons & FROM_LEFT_1ST_BUTTON_PRESSED) == 0);
        m_lastMouseButtons = buttonState;

        // 1. Mouse Wheel Scrolling in Logs tab
        if ((eventFlags & MOUSE_WHEELED) != 0) {
            short wheelDelta = static_cast<short>(HIWORD(buttonState));
            if (m_currentTab == TuiTab::Logs) {
                if (wheelDelta > 0) {
                    m_logScrollOffset += 3;
                } else if (wheelDelta < 0) {
                    m_logScrollOffset = (m_logScrollOffset >= 3) ? m_logScrollOffset - 3 : 0;
                }
                return true;
            }
            return false;
        }

        if (!leftClicked) return false;

        // 2. Click on Top Tab Bar (Row 0)
        if (mouseY == 0) {
            // [JOHN'S BARCODE] [1:Dashboard] [2:Logs] [3:Config] [4:Diagnostics]
            if (mouseX >= 16 && mouseX <= 29) {
                SetTab(TuiTab::Dashboard);
                return true;
            } else if (mouseX >= 30 && mouseX <= 38) {
                SetTab(TuiTab::Logs);
                return true;
            } else if (mouseX >= 39 && mouseX <= 49) {
                SetTab(TuiTab::Config);
                return true;
            } else if (mouseX >= 50 && mouseX <= 66) {
                SetTab(TuiTab::Diagnostics);
                return true;
            }
            return false;
        }

        // 3. Click on Bottom Hotkey Bar
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        int screenRows = 28;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            screenRows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
        if (mouseY == 26 || mouseY == 23 || mouseY >= screenRows - 2) {
            if (mouseX <= 21) {
                // [JOHN'S] [1-4] Win -> cycle tab
                SetTab(static_cast<TuiTab>((static_cast<int>(m_currentTab) + 1) % 4));
                return true;
            } else if (mouseX >= 22 && mouseX <= 36) {
                // [H/T/U] Mode -> cycle mode
                int curM = static_cast<int>(m_activeMode);
                m_activeMode = static_cast<ActiveMode>((curM + 1) % 4);
                if (m_activeMode == ActiveMode::CloudflareTunnel && !m_tunnel.IsRunning()) {
                    AppLogger::Instance().Log("[TUNNEL] Activating Cloudflare Quick Tunnel...", LogLevel::Info);
                    m_tunnel.Start(m_exeDir, m_port, [](const std::string&) {});
                }
                return true;
            } else if (mouseX >= 37 && mouseX <= 50) {
                // [D] Debounce -> cycle debounce
                int cur = m_wedge.GetCooldown();
                int nextCd = (cur == 1500) ? 3000 : ((cur == 3000) ? 5000 : ((cur == 5000) ? 999999 : 1500));
                m_wedge.SetCooldown(nextCd);
                AppLogger::Instance().Log("[CONFIG] Debounce filter cycled to: " + std::to_string(nextCd) + "ms", LogLevel::Info);
                return true;
            } else if (mouseX >= 51 && mouseX <= 62) {
                // [N] Rename
                SetTab(TuiTab::Config);
                m_configRow = 0;
                ActivateConfigAction();
                return true;
            } else if (mouseX >= 63 && mouseX <= 72) {
                // [K] Kick
                m_sessionMgr.KickCurrentDevice();
                AppLogger::Instance().Log("[OVERRIDE] Disconnected current device. Session slot freed.", LogLevel::Warning);
                return true;
            } else if (mouseX >= 73 && mouseX <= 82) {
                // [C] Clear
                m_wedge.ResetCooldown();
                AppLogger::Instance().Clear();
                AppLogger::Instance().Log("[STATUS] Debounce filter cache & logs cleared.", LogLevel::Info);
                return true;
            } else if (mouseX >= 83 && mouseX <= 96) {
                // [Q] Quit
                m_quitRequested = true;
                return true;
            }
            return false;
        }

        // 4. Click inside Active Tab
        if (m_currentTab == TuiTab::Config) {
            // Rows 5 to 11 correspond to config rows 0 to 6
            if (mouseY >= 5 && mouseY <= 11) {
                int clickedRow = mouseY - 5;
                m_configRow = clickedRow;
                if (clickedRow == 0 || clickedRow == 5 || clickedRow == 6) {
                    ActivateConfigAction();
                } else {
                    CycleConfigValue(1);
                }
                return true;
            }
        } else if (m_currentTab == TuiTab::Logs) {
            if (mouseY == 2) {
                if (mouseX <= 22) {
                    m_logFilter = (m_logFilter + 1) % 4;
                    m_logScrollOffset = 0;
                    return true;
                } else if (mouseX >= 23 && mouseX <= 46) {
                    AppLogger::Instance().Clear();
                    m_logScrollOffset = 0;
                    return true;
                }
            }
        } else if (m_currentTab == TuiTab::Diagnostics) {
            // Action buttons on Row 16
            if (mouseY == 16) {
                if (mouseX <= 28) {
                    m_sessionMgr.KickCurrentDevice();
                    AppLogger::Instance().Log("[OVERRIDE] Disconnected current device. Session slot freed.", LogLevel::Warning);
                    return true;
                } else if (mouseX >= 29 && mouseX <= 70) {
                    m_wedge.ResetCooldown();
                    m_wedge.TypeBarcode("TEST-BARCODE-12345", KeySuffix::Enter);
                    Beep(1000, 150);
                    AppLogger::Instance().Log("[TEST PULSE] Injected sample keystrokes: [TEST-BARCODE-12345]", LogLevel::Info);
                    return true;
                }
            }
        }

        return false;
    }

private:
    SessionManager& m_sessionMgr;
    KeyboardWedge& m_wedge;
    CloudflareTunnel& m_tunnel;
    std::string m_exeDir;
    int m_port;

    std::string m_deviceName = "WORKSTATION-01";
    std::string m_localIp = "127.0.0.1";
    ActiveMode m_activeMode;
    TuiTab m_currentTab;
    int m_configRow;
    int m_logScrollOffset;
    int m_logFilter;
    bool m_tabChanged;
    bool m_quitRequested;
    DWORD m_origInMode;
    DWORD m_lastMouseButtons;

    static std::string BoxHr(int count) {
        if (count <= 0) return "";
        std::string s;
        s.reserve(count * 3);
        for (int i = 0; i < count; ++i) s += "─";
        return s;
    }

    static size_t VisibleLength(const std::string& str) {
        size_t len = 0;
        bool inEscape = false;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '\033') {
                inEscape = true;
            } else if (inEscape) {
                if (str[i] == 'm') {
                    inEscape = false;
                }
            } else {
                unsigned char c = static_cast<unsigned char>(str[i]);
                if ((c & 0xC0) != 0x80) { // Not a UTF-8 continuation byte
                    len++;
                }
            }
        }
        return len;
    }

    static std::string PadRight(const std::string& str, size_t targetWidth) {
        size_t vLen = VisibleLength(str);
        if (vLen >= targetWidth) return str;
        return str + std::string(targetWidth - vLen, ' ');
    }

    void RenderTabBar(std::ostringstream& out, int cols) {
        out << "\033[44;37m JOHN'S BARCODE \033[0m";

        auto tabBadge = [&](TuiTab tab, const std::string& label) {
            if (m_currentTab == tab) {
                out << "\033[42;30m " << label << " \033[0m";
            } else {
                out << "\033[40;37m " << label << " \033[0m";
            }
        };

        tabBadge(TuiTab::Dashboard, "1:Dashboard");
        tabBadge(TuiTab::Logs, "2:Logs");
        tabBadge(TuiTab::Config, "3:Config");
        tabBadge(TuiTab::Diagnostics, "4:Diagnostics");

        bool connected = m_sessionMgr.IsDeviceConnected();
        std::string statusText = connected ? "ONLINE" : "WAITING";
        std::string statusColor = connected ? "\033[42;30m" : "\033[43;30m";

        out << "\033[40;37m STATION:\033[1;36m" << m_deviceName << "\033[0;37m "
            << statusColor << " " << statusText << " \033[0m\033[K\n";
    }

    std::string GetActivePairingUrl() {
        std::string pin = m_sessionMgr.GetPin();
        switch (m_activeMode) {
            case ActiveMode::CloudflareTunnel: {
                std::string tUrl = m_tunnel.GetUrl();
                return tUrl.empty() ? "" : (tUrl + "/?pin=" + pin);
            }
            case ActiveMode::UsbCable:
                return "http://localhost:" + std::to_string(m_port) + "/?pin=" + pin;
            case ActiveMode::LanHttp:
                return "http://" + m_localIp + ":" + std::to_string(m_port) + "/?pin=" + pin;
            case ActiveMode::LiveWifiHttps:
            default:
                return "https://" + m_localIp + ":8443/?pin=" + pin;
        }
    }

    std::vector<std::string> GenerateQrAsciiLines(const std::string& text) {
        std::vector<std::string> lines;
        if (text.empty()) return lines;

        QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::LOW);
        int border = 1; // 1-module border keeps QR compact so it fits perfectly in 20 lines
        int size = qr.getSize();

        // High-contrast, standard optical QR encoding:
        // Background: White (\033[47m), Foreground: Black (\033[30m)
        for (int y = -border; y < size + border; y += 2) {
            std::string line = "\033[47;30m";
            for (int x = -border; x < size + border; x++) {
                bool upper = (x >= 0 && x < size && y >= 0 && y < size) ? qr.getModule(x, y) : false;
                bool lower = (x >= 0 && x < size && (y + 1) >= 0 && (y + 1) < size) ? qr.getModule(x, y + 1) : false;

                if (upper && lower) {
                    line += "\xE2\x96\x88"; // Full black block
                } else if (upper && !lower) {
                    line += "\xE2\x96\x80"; // Top black, bottom white
                } else if (!upper && lower) {
                    line += "\xE2\x96\x84"; // Top white, bottom black
                } else {
                    line += " ";           // Both white (quiet zone / light module)
                }
            }
            line += "\033[0m";
            lines.push_back(line);
        }
        return lines;
    }

    void RenderDashboardWindow(std::ostringstream& out, int cols, int rows) {
        std::string pairUrl = GetActivePairingUrl();
        auto qrLines = GenerateQrAsciiLines(pairUrl);

        std::string modeTitle;
        switch (m_activeMode) {
            case ActiveMode::CloudflareTunnel: modeTitle = "CLOUDFLARE QUICK TUNNEL (Global SSL)"; break;
            case ActiveMode::UsbCable: modeTitle = "USB WIRED (ADB Reverse Offline)"; break;
            case ActiveMode::LanHttp: modeTitle = "LOCAL HTTP LAN (Port 8080)"; break;
            case ActiveMode::LiveWifiHttps: default: modeTitle = "LIVE WI-FI HTTPS (High-Speed LAN)"; break;
        }

        int leftW = 44;
        int rightW = 48;
        if (cols >= 102) {
            rightW = cols - leftW - 7;
            if (rightW > 65) rightW = 65;
        }

        out << "┌──[ SCANNER PAIRING VIEWPORT ]" << BoxHr(leftW - 27) << "┬──[ ACTIVITY STREAM & DEDUPLICATION ]" << BoxHr(rightW - 35) << "┐\033[K\n";

        auto stats = m_wedge.GetStats();
        auto logs = AppLogger::Instance().GetRecentEntries();

        // 23 lines of content ensures large tunnel QR codes (up to 19 rows) fit completely with borders
        const size_t CONTENT_LINES = 23;

        for (size_t i = 0; i < CONTENT_LINES; ++i) {
            // Left Column Content
            std::string leftStr;
            if (i == 0) {
                leftStr = "\033[1;36m" + modeTitle + "\033[0m";
            } else if (i == 1) {
                leftStr = "\033[90mPIN: \033[1;33m[ " + m_sessionMgr.GetPin() + " ]  \033[90mSTATION: \033[1;32m" + m_deviceName + "\033[0m";
            } else if (i == 2) {
                leftStr = pairUrl.empty() ? "\033[1;35mAllocating Cloudflare domain...\033[0m" : ("\033[4;36m" + pairUrl + "\033[0m");
            } else {
                size_t qrIdx = i - 3;
                if (qrIdx < qrLines.size()) {
                    leftStr = "   " + qrLines[qrIdx];
                } else if (qrIdx == qrLines.size()) {
                    leftStr = " \033[1;33mScan QR code with mobile camera\033[0m";
                }
            }

            // Right Column Content
            std::string rightStr;
            if (i == 0) {
                rightStr = "\033[1;32mINJECTED: " + std::to_string(stats.totalInjected) +
                           "\033[0m │ \033[1;33mBLOCKED: " + std::to_string(stats.totalDuplicatesBlocked) +
                           "\033[0m │ \033[1;35mFILTER: " + std::to_string(m_wedge.GetCooldown()) + "ms\033[0m";
            } else if (i == 1) {
                rightStr = "\033[90mLAST SCAN: \033[1;37m" + (stats.lastBarcode.empty() ? "(none)" : stats.lastBarcode) + "\033[0m";
            } else if (i == 2) {
                rightStr = "\033[90m" + BoxHr(rightW) + "\033[0m";
            } else {
                size_t logRow = i - 3;
                if (logRow < logs.size()) {
                    const auto& entry = logs[logs.size() - 1 - logRow];
                    std::string badgeColor = "\033[1;37m";
                    if (entry.level == LogLevel::Success) badgeColor = "\033[1;32m";
                    else if (entry.level == LogLevel::Warning) badgeColor = "\033[1;33m";
                    else if (entry.level == LogLevel::Error) badgeColor = "\033[1;31m";

                    std::string truncMsg = entry.message;
                    size_t maxMsgW = (rightW > 24) ? (size_t)(rightW - 24) : 10;
                    if (truncMsg.size() > maxMsgW) truncMsg = truncMsg.substr(0, maxMsgW - 3) + "...";

                    rightStr = "\033[90m" + entry.timestamp + "\033[0m " + badgeColor + entry.tag + "\033[0m " + truncMsg;
                }
            }

            out << "│ " << PadRight(leftStr, leftW)
                << " │ " << PadRight(rightStr, rightW)
                << " │\033[K\n";
        }

        out << "└──" << BoxHr(leftW + 1) << "┴──" << BoxHr(rightW + 1) << "┘\033[K\n";
    }

    void RenderLogsWindow(std::ostringstream& out, int cols, int rows) {
        auto logs = AppLogger::Instance().GetRecentEntries();
        std::vector<LogEntry> filtered;
        for (const auto& l : logs) {
            if (m_logFilter == 1 && l.tag.find("INJECTED") == std::string::npos) continue;
            if (m_logFilter == 2 && l.tag.find("DUP") == std::string::npos) continue;
            if (m_logFilter == 3 && l.tag.find("SYS") == std::string::npos && l.tag.find("AUTH") == std::string::npos) continue;
            filtered.push_back(l);
        }

        std::string filterName = (m_logFilter == 0) ? "ALL LOGS" : (m_logFilter == 1 ? "INJECTED ONLY" : (m_logFilter == 2 ? "DUPLICATES ONLY" : "SYS/AUTH ONLY"));

        out << "┌──[ WINDOW 2: ACTIVITY LOGS INSPECTOR ── " << filterName << " ]" << BoxHr(cols - 48) << "┐\033[K\n";

        std::vector<std::string> lines;
        lines.push_back("\033[90m[F] Cycle Filter  │  [C] Clear Log Buffer  │  [↑/↓/Wheel] Scroll  │  Total: " + std::to_string(filtered.size()) + " entries\033[0m");
        lines.push_back(BoxHr(cols - 4));

        int logSlotCount = 21;
        int total = static_cast<int>(filtered.size());
        int startIdx = (std::max)(0, total - logSlotCount - m_logScrollOffset);
        int endIdx = (std::min)(total, startIdx + logSlotCount);

        if (filtered.empty()) {
            lines.push_back("  \033[90m(No log events matching current filter)\033[0m");
        } else {
            for (int i = startIdx; i < endIdx; ++i) {
                const auto& e = filtered[i];
                std::string badgeColor = "\033[1;37m";
                if (e.level == LogLevel::Success) badgeColor = "\033[1;32m";
                else if (e.level == LogLevel::Warning) badgeColor = "\033[1;33m";
                else if (e.level == LogLevel::Error) badgeColor = "\033[1;31m";

                std::string lineStr = e.message;
                if (lineStr.size() > (size_t)(cols - 34)) lineStr = lineStr.substr(0, cols - 37) + "...";

                std::ostringstream lss;
                lss << "  " << std::setw(3) << std::setfill('0') << e.index << "  "
                    << "\033[90m" << e.timestamp << "\033[0m  "
                    << badgeColor << std::left << std::setw(15) << std::setfill(' ') << e.tag << "\033[0m "
                    << lineStr;
                lines.push_back(lss.str());
            }
        }

        // Pad to 23 lines
        while (lines.size() < 23) {
            lines.push_back("");
        }

        for (size_t i = 0; i < 23; ++i) {
            out << "│ " << PadRight(lines[i], cols - 4) << " │\033[K\n";
        }

        out << "└──" << BoxHr(cols - 4) << "┘\033[K\n";
    }

    void RenderConfigWindow(std::ostringstream& out, int cols, int rows) {
        out << "┌──[ WINDOW 3: INTERACTIVE SYSTEM & WEDGE CONFIGURATION ]" << BoxHr(cols - 59) << "┐\033[K\n";

        std::vector<std::string> lines;
        lines.push_back("\033[90mUse [↑/↓] or Mouse Click to select, [←/→/Click/Space] to modify parameters\033[0m");
        lines.push_back(BoxHr(cols - 4));
        lines.push_back("");

        auto makeRow = [&](int idx, const std::string& label, const std::string& val, const std::string& hint) {
            bool isSelected = (m_configRow == idx);
            std::string cursor = isSelected ? "\033[1;32m ► \033[42;30m" : "    \033[1;37m";
            std::string endColor = "\033[0m";

            return cursor + " " + PadRight(label, 26) + " : " + PadRight(val, 24) + endColor + " \033[90m" + hint + "\033[0m";
        };

        lines.push_back(makeRow(0, "Workstation / PC Name", "[" + m_deviceName + "]", "(Click or Enter to rename)"));

        int cd = m_wedge.GetCooldown();
        std::string cdStr;
        if (cd == 1500) cdStr = "< 1.5s (Fast) >";
        else if (cd == 3000) cdStr = "< 3.0s (Standard) >";
        else if (cd == 5000) cdStr = "< 5.0s (Strict) >";
        else if (cd >= 999999) cdStr = "< Must Leave View >";
        else cdStr = "< " + std::to_string(cd) + "ms >";
        lines.push_back(makeRow(1, "Duplicate Scan Filter", cdStr, "(Click to cycle cooldown)"));

        KeySuffix suf = m_wedge.GetSuffix();
        std::string sufStr = (suf == KeySuffix::Enter) ? "< Enter (0x0D) >" : ((suf == KeySuffix::Tab) ? "< Tab (0x09) >" : "< None >");
        lines.push_back(makeRow(2, "Keystroke Suffix", sufStr, "(Click to change suffix)"));

        std::string chimeStr = m_wedge.IsChimeEnabled() ? "< Enabled >" : "< Muted >";
        lines.push_back(makeRow(3, "PC Speaker Audio Beep", chimeStr, "(Click to toggle sound)"));

        std::string modeStr;
        switch (m_activeMode) {
            case ActiveMode::CloudflareTunnel: modeStr = "< Cloudflare Tunnel >"; break;
            case ActiveMode::UsbCable: modeStr = "< USB ADB Cable >"; break;
            case ActiveMode::LanHttp: modeStr = "< Plain HTTP LAN >"; break;
            case ActiveMode::LiveWifiHttps: default: modeStr = "< Wi-Fi HTTPS >"; break;
        }
        lines.push_back(makeRow(4, "Active Connection Mode", modeStr, "(Click to switch mode)"));

        std::string tStatus;
        if (m_tunnel.IsRunning()) {
            tStatus = "< RUNNING >";
        } else if (m_tunnel.IsDownloading()) {
            tStatus = "< DOWNLOADING... >";
        } else if (m_tunnel.IsInstalled(m_exeDir)) {
            tStatus = "< INSTALLED (Stopped) >";
        } else {
            tStatus = "< NOT DOWNLOADED (Click to Download) >";
        }
        lines.push_back(makeRow(5, "Cloudflare Quick Tunnel", tStatus, "(Click/Enter to toggle or download)"));

        lines.push_back(makeRow(6, "Reset Deduplication Shield", "[ PRESS ENTER TO CLEAR ]", "(Resets counts and locks)"));

        lines.push_back("");
        lines.push_back("\033[1;36mNOTE:\033[0m Changes are synchronized in real-time with both the PC wedge and phone.");

        while (lines.size() < 23) {
            lines.push_back("");
        }

        for (size_t i = 0; i < 23; ++i) {
            out << "│ " << PadRight(lines[i], cols - 4) << " │\033[K\n";
        }

        out << "└──" << BoxHr(cols - 4) << "┘\033[K\n";
    }

    void RenderDiagnosticsWindow(std::ostringstream& out, int cols, int rows) {
        out << "┌──[ WINDOW 4: SCANNER CLIENT & HARDWARE DIAGNOSTICS ]" << BoxHr(cols - 56) << "┐\033[K\n";

        std::vector<std::string> lines;
        lines.push_back("");

        bool connected = m_sessionMgr.IsDeviceConnected();
        std::string clientIp = connected ? m_sessionMgr.GetActiveClientIp() : "None";
        std::string statusStr = connected ? "\033[1;32mCONNECTED & PAIRED\033[0m" : "\033[1;33mAWAITING MOBILE SCANNER\033[0m";

        lines.push_back("  \033[1;37mScanner Client Status:\033[0m   " + statusStr);
        lines.push_back("  \033[1;37mClient IP Address:\033[0m       " + clientIp);
        lines.push_back("  \033[1;37mSession Security PIN:\033[0m    \033[1;33m[ " + m_sessionMgr.GetPin() + " ]\033[0m");
        lines.push_back("  \033[1;37mLocal Host IP:\033[0m           " + m_localIp + " (Port: " + std::to_string(m_port) + ")");
        lines.push_back("");

        auto stats = m_wedge.GetStats();
        lines.push_back("├──[ KEYSTROKE WEDGE PERFORMANCE METRICS ]" + BoxHr(cols - 46) + "┤");
        lines.push_back("  \033[1;32mTotal Keystrokes Injected:\033[0m   " + std::to_string(stats.totalInjected));
        lines.push_back("  \033[1;33mDuplicates Intercepted:\033[0m      " + std::to_string(stats.totalDuplicatesBlocked));
        lines.push_back("  \033[1;36mCamera Frames Processed:\033[0m     " + std::to_string(stats.totalScanned));
        lines.push_back("  \033[1;35mDebounce Filter Interval:\033[0m    " + std::to_string(m_wedge.GetCooldown()) + "ms");
        lines.push_back("  \033[1;37mLast Injected Barcode:\033[0m       " + (stats.lastBarcode.empty() ? "(none)" : stats.lastBarcode));
        lines.push_back("");

        lines.push_back("├──[ ACTIONS ]" + BoxHr(cols - 18) + "┤");
        lines.push_back("  \033[1;31m[K]\033[0m Force Kick Client      \033[1;36m[T]\033[0m Send Test Keystroke (TEST-BARCODE-12345)");

        while (lines.size() < 23) {
            lines.push_back("");
        }

        for (size_t i = 0; i < 23; ++i) {
            if (lines[i].rfind("├──", 0) == 0) {
                out << lines[i] << "\033[K\n";
            } else {
                out << "│ " << PadRight(lines[i], cols - 4) << " │\033[K\n";
            }
        }

        out << "└──" << BoxHr(cols - 4) << "┘\033[K\n";
    }

    void RenderBottomBar(std::ostringstream& out, int cols, int rows) {
        out << "\033[42;30m [JOHN'S] \033[44;37m [1-4] Win \033[45;37m [H/T/U] Mode \033[46;30m [D] Debounce \033[43;30m [N] Rename \033[41;37m [K] Kick \033[47;30m [C] Clear \033[41;37m [Q] Quit \033[0m\033[K\n";
    }

    void CycleConfigValue(int dir) {
        if (m_configRow == 1) { // Debounce cooldown
            int cur = m_wedge.GetCooldown();
            int next = 3000;
            if (dir > 0) {
                if (cur == 1500) next = 3000;
                else if (cur == 3000) next = 5000;
                else if (cur == 5000) next = 999999;
                else next = 1500;
            } else {
                if (cur >= 999999) next = 5000;
                else if (cur == 5000) next = 3000;
                else if (cur == 3000) next = 1500;
                else next = 999999;
            }
            m_wedge.SetCooldown(next);
            AppLogger::Instance().Log("[CONFIG] Cooldown filter set to: " + std::to_string(next) + "ms", LogLevel::Info);
        } else if (m_configRow == 2) { // Key Suffix
            KeySuffix s = m_wedge.GetSuffix();
            KeySuffix nextS = (s == KeySuffix::Enter) ? KeySuffix::Tab : ((s == KeySuffix::Tab) ? KeySuffix::None : KeySuffix::Enter);
            m_wedge.SetSuffix(nextS);
            std::string sufName = (nextS == KeySuffix::Enter) ? "Enter" : ((nextS == KeySuffix::Tab) ? "Tab" : "None");
            AppLogger::Instance().Log("[CONFIG] Keystroke suffix set to: " + sufName, LogLevel::Info);
        } else if (m_configRow == 3) { // Chime
            bool cur = m_wedge.IsChimeEnabled();
            m_wedge.SetChime(!cur);
            AppLogger::Instance().Log("[CONFIG] PC Speaker Beep: " + std::string(!cur ? "ENABLED" : "MUTED"), LogLevel::Info);
        } else if (m_configRow == 4) { // Mode
            int m = static_cast<int>(m_activeMode);
            m = (m + (dir > 0 ? 1 : 3)) % 4;
            m_activeMode = static_cast<ActiveMode>(m);
            if (m_activeMode == ActiveMode::CloudflareTunnel && !m_tunnel.IsRunning()) {
                AppLogger::Instance().Log("[TUNNEL] Activating Cloudflare Quick Tunnel...", LogLevel::Info);
                m_tunnel.Start(m_exeDir, m_port, [](const std::string&) {});
            }
        }
    }

    void ActivateConfigAction() {
        if (m_configRow == 0) {
            // Rename Workstation
            std::cout << "\033[?25h"; // show cursor
            std::cout << "\n\033[1;36mEnter new Workstation Name: \033[0m" << std::flush;
            std::string newName;
            std::getline(std::cin, newName);
            while (!newName.empty() && (newName.back() == '\r' || newName.back() == '\n' || newName.back() == ' ')) newName.pop_back();
            if (!newName.empty()) {
                m_deviceName = newName;
                AppLogger::Instance().Log("[SYS] Workstation renamed to: " + newName, LogLevel::Success);
            }
            std::cout << "\033[?25l"; // hide cursor
        } else if (m_configRow == 5) {
            // Toggle / Download Tunnel
            if (m_tunnel.IsRunning()) {
                m_tunnel.Stop();
                AppLogger::Instance().Log("[TUNNEL] Cloudflare Tunnel stopped.", LogLevel::Info);
            } else if (m_tunnel.IsDownloading()) {
                AppLogger::Instance().Log("[TUNNEL] Download in progress, please wait...", LogLevel::Warning);
            } else {
                AppLogger::Instance().Log("[TUNNEL] Activating Cloudflare Quick Tunnel...", LogLevel::Info);
                m_tunnel.Start(m_exeDir, m_port, [](const std::string&) {});
                m_activeMode = ActiveMode::CloudflareTunnel;
            }
        } else if (m_configRow == 6) {
            m_wedge.ResetCooldown();
            AppLogger::Instance().Log("[STATUS] Debounce filter cache & stats reset.", LogLevel::Info);
        }
    }
};
