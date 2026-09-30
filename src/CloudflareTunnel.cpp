#include "CloudflareTunnel.h"
#include "AppLogger.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

CloudflareTunnel::CloudflareTunnel() = default;

CloudflareTunnel::~CloudflareTunnel() {
    Stop();
}

std::string CloudflareTunnel::GetBinaryPath(const std::string& appDir) {
    std::string localBin = appDir + "\\cloudflared.exe";
    DWORD attr = GetFileAttributesA(localBin.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return localBin;
    }
    // Check if cloudflared is on system PATH
    char foundPath[MAX_PATH] = {0};
    DWORD res = SearchPathA(NULL, "cloudflared.exe", NULL, MAX_PATH, foundPath, NULL);
    if (res > 0) {
        return std::string(foundPath);
    }
    return "";
}

bool CloudflareTunnel::IsInstalled(const std::string& appDir) {
    return !GetBinaryPath(appDir).empty();
}

static bool RunSilentProcess(const std::string& cmdLine, DWORD timeoutMs = 120000) {
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi{};
    std::vector<char> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back('\0');

    if (!CreateProcessA(NULL, cmdBuf.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }

    DWORD waitRes = WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD exitCode = 1;
    if (waitRes == WAIT_OBJECT_0) {
        GetExitCodeProcess(pi.hProcess, &exitCode);
    } else {
        TerminateProcess(pi.hProcess, 1);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (waitRes == WAIT_OBJECT_0 && exitCode == 0);
}

bool CloudflareTunnel::DownloadBinary(const std::string& appDir, std::function<void(const std::string&)> statusCb) {
    std::string targetPath = appDir + "\\cloudflared.exe";
    std::string downloadUrl = "https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-windows-amd64.exe";

    m_isDownloading = true;

    if (statusCb) statusCb("Connecting to Cloudflare release repository via curl...");

    // Try curl.exe first (standard on Windows 10/11) with silent flag
    std::string curlCmd = "curl.exe -sSL -o \"" + targetPath + "\" \"" + downloadUrl + "\"";
    bool success = RunSilentProcess(curlCmd, 120000);

    DWORD attr = GetFileAttributesA(targetPath.c_str());
    if (success && attr != INVALID_FILE_ATTRIBUTES) {
        HANDLE hFile = CreateFileA(targetPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER sz;
            GetFileSizeEx(hFile, &sz);
            CloseHandle(hFile);
            if (sz.QuadPart > 10 * 1024 * 1024) { // Valid binary > 10MB
                if (statusCb) statusCb("Cloudflare tunnel binary verified successfully!");
                m_isDownloading = false;
                return true;
            }
        }
    }

    // PowerShell fallback if curl is not present or failed
    if (statusCb) statusCb("Falling back to PowerShell WebClient download...");
    std::string psCmd = "powershell.exe -NoProfile -NonInteractive -Command \"[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; (New-Object Net.WebClient).DownloadFile('" + downloadUrl + "', '" + targetPath + "')\"";
    success = RunSilentProcess(psCmd, 180000);

    attr = GetFileAttributesA(targetPath.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) {
        HANDLE hFile = CreateFileA(targetPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            LARGE_INTEGER sz;
            GetFileSizeEx(hFile, &sz);
            CloseHandle(hFile);
            if (sz.QuadPart > 10 * 1024 * 1024) {
                if (statusCb) statusCb("Cloudflare tunnel binary downloaded successfully!");
                m_isDownloading = false;
                return true;
            }
        }
    }

    m_isDownloading = false;
    if (statusCb) statusCb("Failed to download cloudflared binary.");
    return false;
}

bool CloudflareTunnel::Start(const std::string& appDir, int localPort, std::function<void(const std::string&)> onUrlReady) {
    Stop();

    std::string binPath = GetBinaryPath(appDir);
    if (binPath.empty()) {
        if (m_isDownloading) {
            AppLogger::Instance().Log("[TUNNEL] Download already in progress. Please wait...", LogLevel::Warning);
            return false;
        }
        AppLogger::Instance().Log("[TUNNEL] cloudflared.exe not found! Starting automatic background download...", LogLevel::Warning);
        std::thread([this, appDir, localPort, onUrlReady]() {
            bool ok = DownloadBinary(appDir, [](const std::string& msg) {
                AppLogger::Instance().Log("[TUNNEL] " + msg, LogLevel::Info);
            });
            if (ok) {
                AppLogger::Instance().Log("[TUNNEL] Binary installed successfully! Launching Quick Tunnel...", LogLevel::Success);
                Start(appDir, localPort, onUrlReady);
            } else {
                AppLogger::Instance().Log("[TUNNEL ERROR] Automatic download failed. Run download_cloudflared.bat or check internet.", LogLevel::Error);
            }
        }).detach();
        return false;
    }

    m_onUrlReady = onUrlReady;
    {
        std::lock_guard<std::mutex> lock(m_urlMutex);
        m_tunnelUrl.clear();
    }

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&m_hChildStdOutRead, &m_hChildStdOutWrite, &saAttr, 0)) {
        AppLogger::Instance().Log("[TUNNEL ERROR] Failed to create stdout pipe.", LogLevel::Error);
        return false;
    }

    // Ensure read handle is not inherited by child
    SetHandleInformation(m_hChildStdOutRead, HANDLE_FLAG_INHERIT, 0);

    // CRITICAL: Redirect child's STDIN to NUL so cloudflared cannot steal the console keyboard input
    HANDLE hNulIn = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &saAttr, OPEN_EXISTING, 0, NULL);

    STARTUPINFOA si = { sizeof(si) };
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = (hNulIn != INVALID_HANDLE_VALUE) ? hNulIn : NULL;
    si.hStdOutput = m_hChildStdOutWrite;
    si.hStdError = m_hChildStdOutWrite;

    std::string cmd = "\"" + binPath + "\" tunnel --url http://127.0.0.1:" + std::to_string(localPort);
    std::vector<char> cmdVec(cmd.begin(), cmd.end());
    cmdVec.push_back('\0');

    BOOL success = CreateProcessA(
        NULL,
        cmdVec.data(),
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        appDir.c_str(),
        &si,
        &m_pi
    );

    // Close pipe write end and NUL handle in parent process
    CloseHandle(m_hChildStdOutWrite);
    m_hChildStdOutWrite = NULL;

    if (hNulIn != INVALID_HANDLE_VALUE) {
        CloseHandle(hNulIn);
    }

    if (!success) {
        CloseHandle(m_hChildStdOutRead);
        m_hChildStdOutRead = NULL;
        AppLogger::Instance().Log("[TUNNEL ERROR] Failed to spawn cloudflared process.", LogLevel::Error);
        return false;
    }

    m_running = true;
    m_readerThread = std::make_unique<std::thread>(&CloudflareTunnel::ReaderWorker, this);
    AppLogger::Instance().Log("[TUNNEL] Initializing Cloudflare Quick Tunnel...", LogLevel::Info);
    return true;
}

void CloudflareTunnel::ReaderWorker() {
    char buffer[2048];
    DWORD bytesRead = 0;
    std::string accumulated;

    while (m_running && ReadFile(m_hChildStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        accumulated += buffer;

        // Search for .trycloudflare.com
        size_t cfPos = accumulated.find(".trycloudflare.com");
        if (cfPos != std::string::npos) {
            size_t httpPos = accumulated.rfind("https://", cfPos);
            if (httpPos != std::string::npos) {
                size_t endPos = cfPos + std::strlen(".trycloudflare.com");
                std::string rawUrl = accumulated.substr(httpPos, endPos - httpPos);

                // Clean any trailing whitespace or box characters
                while (!rawUrl.empty() && (rawUrl.back() == ' ' || rawUrl.back() == '\r' || rawUrl.back() == '\n' || rawUrl.back() == '|')) {
                    rawUrl.pop_back();
                }

                std::string urlToNotify;
                {
                    std::lock_guard<std::mutex> lock(m_urlMutex);
                    if (m_tunnelUrl.empty()) {
                        m_tunnelUrl = rawUrl;
                        urlToNotify = rawUrl;
                    }
                }

                // Call notification callback OUTSIDE lock to prevent deadlocks
                if (!urlToNotify.empty()) {
                    AppLogger::Instance().Log("[TUNNEL] Public Tunnel URL ready: " + urlToNotify, LogLevel::Success);
                    if (m_onUrlReady) {
                        m_onUrlReady(urlToNotify);
                    }
                }
            }
        }

        // Keep accumulated buffer bounded
        if (accumulated.size() > 16384) {
            accumulated = accumulated.substr(accumulated.size() - 4096);
        }
    }
}

std::string CloudflareTunnel::GetUrl() const {
    std::lock_guard<std::mutex> lock(m_urlMutex);
    return m_tunnelUrl;
}

void CloudflareTunnel::Stop() {
    m_running = false;

    if (m_pi.hProcess) {
        TerminateProcess(m_pi.hProcess, 0);
        CloseHandle(m_pi.hProcess);
        CloseHandle(m_pi.hThread);
        m_pi.hProcess = NULL;
        m_pi.hThread = NULL;
    }

    if (m_hChildStdOutRead) {
        CloseHandle(m_hChildStdOutRead);
        m_hChildStdOutRead = NULL;
    }

    if (m_hChildStdOutWrite) {
        CloseHandle(m_hChildStdOutWrite);
        m_hChildStdOutWrite = NULL;
    }

    if (m_readerThread && m_readerThread->joinable()) {
        m_readerThread->join();
        m_readerThread.reset();
    }

    {
        std::lock_guard<std::mutex> lock(m_urlMutex);
        m_tunnelUrl.clear();
    }
}
