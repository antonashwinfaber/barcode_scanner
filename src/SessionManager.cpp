#include "SessionManager.h"
#include <random>
#include <sstream>
#include <iomanip>

SessionManager::SessionManager(const std::string& customPin) {
    if (!customPin.empty()) {
        m_pin = customPin;
    } else {
        m_pin = GenerateRandomPin();
    }
}

std::string SessionManager::GenerateRandomPin() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1000, 9999);
    return std::to_string(dis(gen));
}

std::string SessionManager::GenerateToken() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(16) << dis(gen)
       << std::setw(16) << dis(gen);
    return ss.str();
}

bool SessionManager::TryLogin(const std::string& pin, const std::string& clientIp, std::string& outToken, std::string& outErrorMsg) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Verify PIN
    if (pin != m_pin) {
        outErrorMsg = "Invalid PIN code. Please verify the PIN shown on the PC screen.";
        return false;
    }

    // Check if session is already taken by another device
    if (m_isActive) {
        // If the same IP reconnects, allow session refresh
        if (clientIp == m_activeClientIp) {
            outToken = m_activeToken;
            m_lastHeartbeat = std::chrono::steady_clock::now();
            return true;
        }

        outErrorMsg = "Session Locked: Another device (" + m_activeClientIp + ") is currently connected as scanner.";
        return false;
    }

    // Grant exclusive session
    m_activeToken = GenerateToken();
    m_activeClientIp = clientIp;
    m_lastHeartbeat = std::chrono::steady_clock::now();
    m_isActive = true;

    outToken = m_activeToken;
    return true;
}

bool SessionManager::ValidateSession(const std::string& token) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isActive || token.empty() || token != m_activeToken) {
        return false;
    }
    m_lastHeartbeat = std::chrono::steady_clock::now();
    return true;
}

bool SessionManager::RecordHeartbeat(const std::string& token) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isActive || token != m_activeToken) {
        return false;
    }
    m_lastHeartbeat = std::chrono::steady_clock::now();
    return true;
}

void SessionManager::Logout(const std::string& token) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isActive && token == m_activeToken) {
        m_isActive = false;
        m_activeToken.clear();
        m_activeClientIp.clear();
    }
}

void SessionManager::KickCurrentDevice() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_isActive = false;
    m_activeToken.clear();
    m_activeClientIp.clear();
}

bool SessionManager::CheckLiveness(int timeoutSeconds) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isActive) return false;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastHeartbeat).count();

    if (elapsed > timeoutSeconds) {
        m_isActive = false;
        m_activeToken.clear();
        m_activeClientIp.clear();
        return true; // Session expired
    }
    return false;
}

std::string SessionManager::GetPin() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pin;
}

bool SessionManager::IsDeviceConnected() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isActive;
}

std::string SessionManager::GetActiveClientIp() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeClientIp;
}
