#pragma once

#include <string>
#include <mutex>
#include <chrono>

class SessionManager {
public:
    explicit SessionManager(const std::string& customPin = "");

    // Validates PIN and establishes an exclusive single-device session.
    // If another device is active, fails and returns error message.
    bool TryLogin(const std::string& pin, const std::string& clientIp, std::string& outToken, std::string& outErrorMsg);

    // Verifies whether the provided token matches the active session.
    // Automatically bumps the liveness heartbeat if valid.
    bool ValidateSession(const std::string& token);

    // Updates heartbeat timestamp for active session.
    bool RecordHeartbeat(const std::string& token);

    // Disconnects active session if token matches.
    void Logout(const std::string& token);

    // Operator override to forcefully disconnect current device.
    void KickCurrentDevice();

    // Checks if the active session timed out (default 30 seconds without scan or ping).
    // Automatically frees the session if timed out.
    bool CheckLiveness(int timeoutSeconds = 30);

    std::string GetPin() const;
    bool IsDeviceConnected() const;
    std::string GetActiveClientIp() const;

private:
    std::string GenerateRandomPin();
    std::string GenerateToken();

    std::string m_pin;
    std::string m_activeToken;
    std::string m_activeClientIp;
    std::chrono::steady_clock::time_point m_lastHeartbeat;
    bool m_isActive = false;

    mutable std::mutex m_mutex;
};
