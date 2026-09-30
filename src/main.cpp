#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <conio.h>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "SessionManager.h"
#include "BarcodeDecoder.h"
#include "KeyboardWedge.h"
#include "WebServer.h"
#include "AppLogger.h"
#include "CloudflareTunnel.h"
#include "TerminalTui.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

struct AdapterInfo {
    std::string name;
    std::string ip;
    bool isWifi;
};

// Enumerate IPv4 addresses, prioritizing Wi-Fi and excluding virtual adapters
std::vector<AdapterInfo> GetNetworkAdapters() {
    std::vector<AdapterInfo> adapters;
    ULONG outBufLen = 15000;
    std::vector<BYTE> buffer(outBufLen);
    PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    DWORD dwRetVal = GetAdaptersAddresses(AF_INET, flags, NULL, pAddresses, &outBufLen);

    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(outBufLen);
        pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
        dwRetVal = GetAdaptersAddresses(AF_INET, flags, NULL, pAddresses, &outBufLen);
    }

    if (dwRetVal == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES pCurr = pAddresses; pCurr != NULL; pCurr = pCurr->Next) {
            if (pCurr->OperStatus != IfOperStatusUp) continue;
            if (pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;

            std::wstring wname = pCurr->FriendlyName ? pCurr->FriendlyName : L"";
            std::string name(wname.begin(), wname.end());

            if (name.find("vEthernet") != std::string::npos ||
                name.find("Virtual") != std::string::npos ||
                name.find("WSL") != std::string::npos ||
                name.find("Pseudo") != std::string::npos) {
                continue;
            }

            for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress; pUnicast != NULL; pUnicast = pUnicast->Next) {
                sockaddr* sa = pUnicast->Address.lpSockaddr;
                if (sa->sa_family == AF_INET) {
                    char ipStr[INET_ADDRSTRLEN] = {0};
                    sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(sa);
                    inet_ntop(AF_INET, &(sin->sin_addr), ipStr, INET_ADDRSTRLEN);

                    std::string ip(ipStr);
                    if (ip.rfind("127.", 0) != 0 && ip.rfind("169.254.", 0) != 0) {
                        bool isWifi = (pCurr->IfType == IF_TYPE_IEEE80211) ||
                                      (name.find("Wi-Fi") != std::string::npos) ||
                                      (name.find("Wireless") != std::string::npos);

                        if (isWifi) {
                            adapters.insert(adapters.begin(), { name, ip, true });
                        } else {
                            adapters.push_back({ name, ip, false });
                        }
                    }
                }
            }
        }
    }
    return adapters;
}

static PROCESS_INFORMATION g_proxyPi = {};
static BOOL g_proxyStarted = FALSE;
static CloudflareTunnel g_tunnel;
static std::atomic<bool> g_needRedraw{true};

BOOL WINAPI ConsoleHandler(DWORD signal) {
    g_tunnel.Stop();
    if (g_proxyStarted && g_proxyPi.hProcess) {
        TerminateProcess(g_proxyPi.hProcess, 0);
        CloseHandle(g_proxyPi.hProcess);
        CloseHandle(g_proxyPi.hThread);
    }
    return FALSE;
}

std::string LoadDeviceName(const std::string& exeDir) {
    std::string configPath = exeDir + "\\device_name.txt";
    std::ifstream f(configPath);
    if (f.is_open()) {
        std::string name;
        std::getline(f, name);
        while (!name.empty() && (name.back() == '\r' || name.back() == '\n' || name.back() == ' ')) name.pop_back();
        if (!name.empty()) return name;
    }
    char buf[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD sz = sizeof(buf);
    if (GetComputerNameA(buf, &sz)) {
        return std::string(buf);
    }
    return "WORKSTATION-01";
}

void SaveDeviceName(const std::string& exeDir, const std::string& name) {
    std::string configPath = exeDir + "\\device_name.txt";
    std::ofstream f(configPath);
    if (f.is_open()) {
        f << name;
    }
}

int main(int argc, char* argv[]) {
    try {
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);

        // Resolve application directory
        char exePath[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string exeDir = exePath;
        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            exeDir = exeDir.substr(0, lastSlash);
        }

        // Support running from build/ subdirectory as well as root
        std::string baseDir = exeDir;
        std::ifstream testF(baseDir + "\\https_server.js");
        if (!testF.good()) {
            std::ifstream testParent(baseDir + "\\..\\https_server.js");
            if (testParent.good()) {
                baseDir = baseDir + "\\..";
            }
        }

        std::string proxyScript = baseDir + "\\https_server.js";

        // Launch HTTPS proxy in background silently without opening extra window
        STARTUPINFOA si = { sizeof(si) };
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        std::string proxyCmdStr = "node \"" + proxyScript + "\"";
        std::vector<char> cmdVec(proxyCmdStr.begin(), proxyCmdStr.end());
        cmdVec.push_back('\0');
        g_proxyStarted = CreateProcessA(NULL, cmdVec.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, baseDir.c_str(), &si, &g_proxyPi);

        // Try setting up ADB reverse for USB mode automatically
        system("adb reverse tcp:8080 tcp:8080 >nul 2>&1");

        std::string deviceName = LoadDeviceName(baseDir);
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "--name" || arg == "-n" || arg == "--device") && i + 1 < argc) {
                deviceName = argv[++i];
                SaveDeviceName(baseDir, deviceName);
            }
        }

        SessionManager sessionManager;
        BarcodeDecoder decoder;
        KeyboardWedge wedge(3000); // 3.0s standard industrial debounce

        int port = 8080;
        WebServer server(port, sessionManager, decoder, wedge, deviceName);

        if (!server.Start()) {
            std::cerr << "\n[ERROR] Failed to start barcode server on port " << port << "!\n";
            std::cerr << "Port " << port << " might already be used by another application or previous instance.\n";
            std::cerr << "Press Enter to exit...";
            std::cin.get();
            return 1;
        }

        auto adapters = GetNetworkAdapters();
        std::string localIp = "127.0.0.1";
        if (!adapters.empty()) {
            localIp = adapters.front().ip;
        }

        // Initialize the Multi-Window ncurses-style TUI
        TerminalTui tui(sessionManager, wedge, g_tunnel, baseDir, port);
        tui.SetDeviceName(deviceName);
        tui.SetLocalIp(localIp);
        tui.Init();

    AppLogger::Instance().SetOnLogCallback([&]() {
        g_needRedraw = true;
    });

    AppLogger::Instance().Log("[SYS] Server listening on HTTP :8080 & HTTPS :8443", LogLevel::Success);
    AppLogger::Instance().Log("[SYS] Workstation identity: " + deviceName, LogLevel::Info);
    if (!adapters.empty()) {
        AppLogger::Instance().Log("[SYS] Primary adapter: " + adapters.front().name + " (" + localIp + ")", LogLevel::Info);
    }

    bool lastConnected = false;
    bool running = true;

    while (running) {
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        DWORD numEvents = 0;
        if (GetNumberOfConsoleInputEvents(hIn, &numEvents) && numEvents > 0) {
            std::vector<INPUT_RECORD> inBuf(numEvents);
            DWORD numRead = 0;
            if (ReadConsoleInputW(hIn, inBuf.data(), numEvents, &numRead)) {
                for (DWORD i = 0; i < numRead; ++i) {
                    const auto& rec = inBuf[i];
                    if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
                        WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
                        WCHAR wch = rec.Event.KeyEvent.uChar.UnicodeChar;

                        if (vk == VK_UP) { tui.HandleKey(0, 72); g_needRedraw = true; }
                        else if (vk == VK_DOWN) { tui.HandleKey(0, 80); g_needRedraw = true; }
                        else if (vk == VK_LEFT) { tui.HandleKey(0, 75); g_needRedraw = true; }
                        else if (vk == VK_RIGHT) { tui.HandleKey(0, 77); g_needRedraw = true; }
                        else if (vk == VK_PRIOR) { tui.HandleKey(0, 73); g_needRedraw = true; }
                        else if (vk == VK_NEXT) { tui.HandleKey(0, 81); g_needRedraw = true; }
                        else if (vk == VK_HOME) { tui.HandleKey(0, 71); g_needRedraw = true; }
                        else if (vk >= VK_F1 && vk <= VK_F4) { tui.HandleKey(0, 59 + (vk - VK_F1)); g_needRedraw = true; }
                        else if (wch != 0) {
                            int ch = static_cast<int>(wch);
                            bool handledByTab = tui.HandleKey(ch, 0);
                            if (handledByTab) {
                                g_needRedraw = true;
                            } else if (ch == 'q' || ch == 'Q') {
                                AppLogger::Instance().Log("[SYS] Shutdown command received.", LogLevel::Info);
                                running = false;
                                break;
                            } else if (ch == 'h' || ch == 'H') {
                                tui.SetActiveMode(ActiveMode::LiveWifiHttps);
                                g_needRedraw = true;
                            } else if (ch == 'u' || ch == 'U') {
                                system("adb reverse tcp:8080 tcp:8080 >nul 2>&1");
                                AppLogger::Instance().Log("[USB] ADB port forward active (http://localhost:8080)", LogLevel::Info);
                                tui.SetActiveMode(ActiveMode::UsbCable);
                                g_needRedraw = true;
                            } else if (ch == 'w' || ch == 'W') {
                                tui.SetActiveMode(ActiveMode::LanHttp);
                                g_needRedraw = true;
                            } else if (ch == 'd' || ch == 'D') {
                                int cur = wedge.GetCooldown();
                                int nextCd = (cur == 1500) ? 3000 : ((cur == 3000) ? 5000 : ((cur == 5000) ? 999999 : 1500));
                                wedge.SetCooldown(nextCd);
                                AppLogger::Instance().Log("[CONFIG] Debounce filter cycled to: " + std::to_string(nextCd) + "ms", LogLevel::Info);
                                g_needRedraw = true;
                            } else if (ch == 'n' || ch == 'N') {
                                tui.SetTab(TuiTab::Config);
                                g_needRedraw = true;
                            } else if (ch == 't' || ch == 'T') {
                                tui.SetActiveMode(ActiveMode::CloudflareTunnel);
                                if (!g_tunnel.IsRunning()) {
                                    AppLogger::Instance().Log("[TUNNEL] Activating Cloudflare Quick Tunnel...", LogLevel::Info);
                                    g_tunnel.Start(baseDir, port, [&](const std::string& url) {
                                        g_needRedraw = true;
                                    });
                                }
                                g_needRedraw = true;
                            } else if (ch == 'l' || ch == 'L') {
                                g_needRedraw = true;
                            } else if (ch == 'k' || ch == 'K') {
                                sessionManager.KickCurrentDevice();
                                AppLogger::Instance().Log("[OVERRIDE] Disconnected current device. Session slot freed.", LogLevel::Warning);
                                g_needRedraw = true;
                            } else if (ch == 'c' || ch == 'C') {
                                wedge.ResetCooldown();
                                AppLogger::Instance().Clear();
                                AppLogger::Instance().Log("[STATUS] Debounce filter cache & logs cleared.", LogLevel::Info);
                                g_needRedraw = true;
                            }
                        }
                    } else if (rec.EventType == MOUSE_EVENT) {
                        const auto& me = rec.Event.MouseEvent;
                        if (tui.HandleMouse(me.dwMousePosition.X, me.dwMousePosition.Y, me.dwButtonState, me.dwEventFlags)) {
                            g_needRedraw = true;
                        }
                    } else if (rec.EventType == WINDOW_BUFFER_SIZE_EVENT) {
                        g_needRedraw = true;
                    }
                }
            }
        }

        if (tui.IsQuitRequested()) {
            AppLogger::Instance().Log("[SYS] Shutdown requested via mouse action.", LogLevel::Info);
            running = false;
            break;
        }

        if (sessionManager.CheckLiveness(30)) {
            AppLogger::Instance().Log("[STATUS] Connected device timed out. Session unlocked.", LogLevel::Warning);
            g_needRedraw = true;
        }

        bool currentConnected = sessionManager.IsDeviceConnected();
        if (currentConnected != lastConnected) {
            lastConnected = currentConnected;
            g_needRedraw = true;
        }

        // Sync device name if modified from config window
        if (tui.GetDeviceName() != deviceName) {
            deviceName = tui.GetDeviceName();
            server.SetDeviceName(deviceName);
            SaveDeviceName(exeDir, deviceName);
            g_needRedraw = true;
        }

        if (g_needRedraw.exchange(false)) {
            tui.Render();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }

        tui.Shutdown();
        server.Stop();
        g_tunnel.Stop();
        if (g_proxyStarted && g_proxyPi.hProcess) {
            TerminateProcess(g_proxyPi.hProcess, 0);
            CloseHandle(g_proxyPi.hProcess);
            CloseHandle(g_proxyPi.hThread);
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "\n======================================================\n";
        std::cerr << "[CRITICAL ERROR] Application Exception: " << ex.what() << "\n";
        std::cerr << "======================================================\n";
        std::cerr << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    } catch (...) {
        std::cerr << "\n======================================================\n";
        std::cerr << "[CRITICAL ERROR] An unhandled system exception occurred.\n";
        std::cerr << "======================================================\n";
        std::cerr << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }
}

