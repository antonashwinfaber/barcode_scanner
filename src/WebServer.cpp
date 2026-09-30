#include "WebServer.h"
#include "MobileUI.h"
#include "AppLogger.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "third_party/httplib.h"

#include <iostream>

WebServer::WebServer(int port, SessionManager& sessionMgr, BarcodeDecoder& decoder, KeyboardWedge& wedge, const std::string& deviceName)
    : m_port(port), m_deviceName(deviceName), m_sessionMgr(sessionMgr), m_decoder(decoder), m_wedge(wedge) {}

WebServer::~WebServer() {
    Stop();
}

bool WebServer::Start() {
    m_running = true;
    m_server = std::make_unique<httplib::Server>();

    // Serve Mobile Web App with dynamic workstation branding
    m_server->Get("/", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_content(GetMobileHtml(m_deviceName), "text/html; charset=utf-8");
    });

    // GET /api/info
    m_server->Get("/api/info", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_content("{\"status\":\"ok\",\"deviceName\":\"" + m_deviceName + "\"}", "application/json");
    });

    // POST /api/login: { "pin": "1234" }
    m_server->Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
        std::string clientIp = req.remote_addr;
        std::string pin = "";

        // Extract PIN from JSON payload
        size_t pinPos = req.body.find("\"pin\"");
        if (pinPos != std::string::npos) {
            size_t colon = req.body.find(':', pinPos);
            if (colon != std::string::npos) {
                size_t firstQuote = req.body.find('\"', colon);
                if (firstQuote != std::string::npos) {
                    size_t secondQuote = req.body.find('\"', firstQuote + 1);
                    if (secondQuote != std::string::npos) {
                        pin = req.body.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                    }
                }
            }
        }

        std::string token;
        std::string errorMsg;
        bool ok = m_sessionMgr.TryLogin(pin, clientIp, token, errorMsg);

        if (ok) {
            AppLogger::Instance().Log("[AUTH] Scanner connected from IP: " + clientIp, LogLevel::Success);
            res.status = 200;
            res.set_content("{\"status\":\"ok\",\"token\":\"" + token + "\"}", "application/json");
        } else {
            AppLogger::Instance().Log("[AUTH] Connection rejected for " + clientIp + ": " + errorMsg, LogLevel::Warning);
            res.status = 409;
            res.set_content("{\"status\":\"error\",\"message\":\"" + errorMsg + "\"}", "application/json");
        }
    });

    // POST /api/scan?token=...
    m_server->Post("/api/scan", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_param_value("token");
        if (!m_sessionMgr.ValidateSession(token)) {
            res.status = 401;
            res.set_content("{\"status\":\"kicked\",\"message\":\"Invalid or expired session\"}", "application/json");
            return;
        }

        std::string autoInjectParam = req.get_param_value("auto_inject");
        bool autoInject = (autoInjectParam == "1" || autoInjectParam == "true");

        const uint8_t* imgData = reinterpret_cast<const uint8_t*>(req.body.data());
        size_t imgLen = req.body.size();

        DecodeResult dec = m_decoder.DecodeFromMemory(imgData, imgLen);
        if (dec.success) {
            if (autoInject) {
                bool wasDuplicate = false;
                bool typed = m_wedge.TypeBarcode(dec.text, KeySuffix::Enter, &wasDuplicate);
                if (typed) {
                    AppLogger::Instance().Log("[INJECTED] >>> [" + dec.text + "] (" + dec.format + ") <<<", LogLevel::Success);
                    res.set_content("{\"status\":\"ok\",\"scanned\":true,\"injected\":true,\"text\":\"" + dec.text + "\",\"format\":\"" + dec.format + "\"}", "application/json");
                } else {
                    AppLogger::Instance().Log("[DUPLICATE BLOCKED] Ignored repeat scan: [" + dec.text + "] (Debounce lock)", LogLevel::Warning);
                    res.set_content("{\"status\":\"ok\",\"scanned\":true,\"injected\":false,\"duplicate\":true,\"text\":\"" + dec.text + "\",\"format\":\"" + dec.format + "\"}", "application/json");
                }
            } else {
                // Freeze & Confirm Mode: Return detected code for operator thumb review
                AppLogger::Instance().Log("[CODE DETECTED] [" + dec.text + "] (" + dec.format + ") - Screen frozen for review", LogLevel::Info);
                res.set_content("{\"status\":\"ok\",\"scanned\":true,\"injected\":false,\"needsConfirm\":true,\"text\":\"" + dec.text + "\",\"format\":\"" + dec.format + "\"}", "application/json");
            }
        } else {
            m_wedge.RegisterNoBarcodeFrame();
            // Keep stream quiet and responsive without console clutter
            res.set_content("{\"status\":\"ok\",\"scanned\":false}", "application/json");
        }
    });

    // POST /api/confirm_scan
    m_server->Post("/api/confirm_scan", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_param_value("token");
        if (token.empty()) {
            token = req.get_header_value("X-Session-Token");
        }
        if (token.empty()) {
            size_t tokPos = req.body.find("\"token\"");
            if (tokPos != std::string::npos) {
                size_t colon = req.body.find(':', tokPos);
                if (colon != std::string::npos) {
                    size_t q1 = req.body.find('\"', colon);
                    if (q1 != std::string::npos) {
                        size_t q2 = req.body.find('\"', q1 + 1);
                        if (q2 != std::string::npos) {
                            token = req.body.substr(q1 + 1, q2 - q1 - 1);
                        }
                    }
                }
            }
        }

        if (!m_sessionMgr.ValidateSession(token)) {
            AppLogger::Instance().Log("[AUTH] /api/confirm_scan rejected: invalid session token", LogLevel::Warning);
            res.status = 401;
            res.set_content("{\"status\":\"error\",\"message\":\"Unauthorized or invalid session\"}", "application/json");
            return;
        }

        std::string text;
        size_t textPos = req.body.find("\"text\"");
        if (textPos != std::string::npos) {
            size_t colon = req.body.find(':', textPos);
            if (colon != std::string::npos) {
                size_t q1 = req.body.find('\"', colon);
                if (q1 != std::string::npos) {
                    size_t q2 = req.body.find('\"', q1 + 1);
                    if (q2 != std::string::npos) {
                        text = req.body.substr(q1 + 1, q2 - q1 - 1);
                    }
                }
            }
        }

        if (!text.empty()) {
            // Write directly to PC with NO need to check dedupe!
            m_wedge.TypeBarcode(text, KeySuffix::Enter, nullptr, /*bypassCooldown=*/true);
            AppLogger::Instance().Log("[OPERATOR CONFIRMED] >>> [" + text + "] Injected to PC (dedupe bypassed) <<<", LogLevel::Success);
            res.set_content("{\"status\":\"ok\",\"injected\":true,\"text\":\"" + text + "\"}", "application/json");
        } else {
            res.status = 400;
            res.set_content("{\"status\":\"error\",\"message\":\"Empty barcode text\"}", "application/json");
        }
    });

    // POST /api/test_type?token=...
    m_server->Post("/api/test_type", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_param_value("token");
        if (!m_sessionMgr.ValidateSession(token)) {
            res.status = 401;
            res.set_content("{\"status\":\"error\",\"message\":\"Unauthorized\"}", "application/json");
            return;
        }

        std::string testString = "TEST-BARCODE-12345";
        m_wedge.ResetCooldown();
        m_wedge.TypeBarcode(testString, KeySuffix::Enter);
        Beep(1000, 150);
        AppLogger::Instance().Log("[TEST PULSE] Injected sample keystrokes: [" + testString + "]", LogLevel::Info);
        res.set_content("{\"status\":\"ok\",\"typed\":true,\"text\":\"" + testString + "\"}", "application/json");
    });

    // POST /api/send_input?token=...
    m_server->Post("/api/send_input", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_param_value("token");
        if (!m_sessionMgr.ValidateSession(token)) {
            res.status = 401;
            res.set_content("{\"status\":\"error\",\"message\":\"Unauthorized\"}", "application/json");
            return;
        }

        std::string text;
        std::string key;

        size_t textPos = req.body.find("\"text\":");
        if (textPos != std::string::npos) {
            size_t q1 = req.body.find('"', textPos + 7);
            if (q1 != std::string::npos) {
                size_t q2 = req.body.find('"', q1 + 1);
                if (q2 != std::string::npos) {
                    text = req.body.substr(q1 + 1, q2 - q1 - 1);
                }
            }
        }

        size_t keyPos = req.body.find("\"key\":");
        if (keyPos != std::string::npos) {
            size_t q1 = req.body.find('"', keyPos + 6);
            if (q1 != std::string::npos) {
                size_t q2 = req.body.find('"', q1 + 1);
                if (q2 != std::string::npos) {
                    key = req.body.substr(q1 + 1, q2 - q1 - 1);
                }
            }
        }

        if (!text.empty()) {
            KeySuffix sfx = KeySuffix::Enter;
            if (req.body.find("\"suffix\":\"none\"") != std::string::npos) sfx = KeySuffix::None;
            else if (req.body.find("\"suffix\":\"tab\"") != std::string::npos) sfx = KeySuffix::Tab;

            m_wedge.TypeManualText(text, sfx);
            Beep(900, 80);
            AppLogger::Instance().Log("[MANUAL INPUT] Injected text: [" + text + "]", LogLevel::Info);
            res.set_content("{\"status\":\"ok\",\"action\":\"typed\",\"text\":\"" + text + "\"}", "application/json");
            return;
        }

        if (!key.empty()) {
            m_wedge.SendSpecialKey(key);
            Beep(800, 60);
            AppLogger::Instance().Log("[KEY EVENT] Injected key: [" + key + "]", LogLevel::Info);
            res.set_content("{\"status\":\"ok\",\"action\":\"key\",\"key\":\"" + key + "\"}", "application/json");
            return;
        }

        res.status = 400;
        res.set_content("{\"status\":\"error\",\"message\":\"Missing text or key\"}", "application/json");
    });

    // POST /api/heartbeat?token=...
    m_server->Post("/api/heartbeat", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_param_value("token");
        if (m_sessionMgr.RecordHeartbeat(token)) {
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } else {
            res.status = 401;
            res.set_content("{\"status\":\"kicked\"}", "application/json");
        }
    });

    // POST /api/logout?token=...
    m_server->Post("/api/logout", [this](const httplib::Request& req, httplib::Response& res) {
        std::string token = req.get_param_value("token");
        m_sessionMgr.Logout(token);
        AppLogger::Instance().Log("[AUTH] Device logged out voluntarily.", LogLevel::Info);
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // GET /api/stats
    m_server->Get("/api/stats", [this](const httplib::Request&, httplib::Response& res) {
        auto stats = m_wedge.GetStats();
        int cd = m_wedge.GetCooldown();
        std::string json = "{"
            "\"scanned\":" + std::to_string(stats.totalScanned) + ","
            "\"injected\":" + std::to_string(stats.totalInjected) + ","
            "\"duplicatesBlocked\":" + std::to_string(stats.totalDuplicatesBlocked) + ","
            "\"cooldownMs\":" + std::to_string(cd) + ","
            "\"lastBarcode\":\"" + stats.lastBarcode + "\""
            "}";
        res.set_content(json, "application/json");
    });

    // POST /api/set_cooldown?ms=...
    m_server->Post("/api/set_cooldown", [this](const httplib::Request& req, httplib::Response& res) {
        std::string msStr = req.get_param_value("ms");
        if (!msStr.empty()) {
            try {
                int ms = std::stoi(msStr);
                if (ms >= 0 && ms <= 600000) {
                    m_wedge.SetCooldown(ms);
                    AppLogger::Instance().Log("[CONFIG] Debounce filter updated to: " + std::to_string(ms) + "ms", LogLevel::Info);
                    res.set_content("{\"status\":\"ok\",\"cooldownMs\":" + std::to_string(ms) + "}", "application/json");
                    return;
                }
            } catch (...) {}
        }
        res.status = 400;
        res.set_content("{\"status\":\"error\",\"message\":\"Invalid ms parameter\"}", "application/json");
    });

    m_thread = std::make_unique<std::thread>([this]() {
        RunServer();
    });

    return true;
}

void WebServer::RunServer() {
    if (m_server) {
        m_server->listen("0.0.0.0", m_port);
    }
}

void WebServer::Stop() {
    if (m_running.exchange(false)) {
        if (m_server) {
            m_server->stop();
        }
        if (m_thread && m_thread->joinable()) {
            m_thread->join();
        }
    }
}
