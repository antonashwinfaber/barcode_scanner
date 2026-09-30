#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <functional>

enum class LogLevel {
    Info,
    Success,
    Warning,
    Error
};

struct LogEntry {
    int index;
    std::string timestamp;
    std::string tag;
    std::string message;
    LogLevel level;
};

class AppLogger {
public:
    static AppLogger& Instance() {
        static AppLogger s_instance;
        return s_instance;
    }

    void Log(const std::string& msg, LogLevel level = LogLevel::Info) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &timeT);

        std::ostringstream ss;
        ss << std::put_time(&tm, "%H:%M:%S");

        LogEntry entry;
        entry.index = ++m_counter;
        entry.timestamp = ss.str();
        entry.level = level;

        if (!msg.empty() && msg[0] == '[') {
            size_t close = msg.find(']');
            if (close != std::string::npos) {
                entry.tag = msg.substr(0, close + 1);
                entry.message = msg.substr(close + 1);
                while (!entry.message.empty() && entry.message.front() == ' ') {
                    entry.message.erase(entry.message.begin());
                }
            } else {
                entry.tag = "[SYS]";
                entry.message = msg;
            }
        } else {
            entry.tag = "[SYS]";
            entry.message = msg;
        }

        m_entries.push_back(entry);
        if (m_entries.size() > m_maxLogs) {
            m_entries.pop_front();
        }

        // Print directly only if direct print mode is enabled
        if (m_directPrint) {
            std::string badgeColor = "\033[1;37m";
            if (level == LogLevel::Success) badgeColor = "\033[1;32m";
            else if (level == LogLevel::Warning) badgeColor = "\033[1;33m";
            else if (level == LogLevel::Error) badgeColor = "\033[1;31m";

            std::cout << "│  " << std::setw(2) << std::setfill('0') << entry.index << "  "
                      << "\033[90m" << entry.timestamp << "\033[0m  "
                      << badgeColor << std::left << std::setw(14) << std::setfill(' ') << entry.tag << "\033[0m "
                      << entry.message << "\n" << std::flush;
        }

        if (m_onLogCallback) {
            m_onLogCallback();
        }
    }

    std::deque<LogEntry> GetRecentEntries() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_entries;
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
        m_counter = 0;
    }

    void SetDirectPrint(bool enable) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_directPrint = enable;
    }

    void SetOnLogCallback(std::function<void()> cb) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_onLogCallback = cb;
    }

private:
    AppLogger() = default;
    std::mutex m_mutex;
    std::deque<LogEntry> m_entries;
    std::function<void()> m_onLogCallback;
    bool m_directPrint = true;
    int m_counter = 0;
    const size_t m_maxLogs = 300;
};
