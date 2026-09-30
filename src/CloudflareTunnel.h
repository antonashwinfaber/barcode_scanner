#pragma once

#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <windows.h>

class CloudflareTunnel {
public:
    CloudflareTunnel();
    ~CloudflareTunnel();

    // Check if cloudflared binary exists in appDir or on PATH
    bool IsInstalled(const std::string& appDir);

    // Download the official standalone cloudflared binary
    bool DownloadBinary(const std::string& appDir, std::function<void(const std::string&)> statusCb);

    // Start cloudflared quick tunnel to localPort
    bool Start(const std::string& appDir, int localPort, std::function<void(const std::string&)> onUrlReady);

    // Stop cloudflared process
    void Stop();

    // Status queries
    bool IsRunning() const { return m_running; }
    bool IsDownloading() const { return m_isDownloading; }
    std::string GetUrl() const;
    std::string GetBinaryPath(const std::string& appDir);

private:
    void ReaderWorker();

    std::atomic<bool> m_running{false};
    std::atomic<bool> m_isDownloading{false};
    std::string m_tunnelUrl;
    mutable std::mutex m_urlMutex;

    PROCESS_INFORMATION m_pi{};
    HANDLE m_hChildStdOutRead = NULL;
    HANDLE m_hChildStdOutWrite = NULL;
    std::unique_ptr<std::thread> m_readerThread;
    std::function<void(const std::string&)> m_onUrlReady;
};
