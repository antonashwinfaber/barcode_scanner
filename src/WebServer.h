#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include "SessionManager.h"
#include "BarcodeDecoder.h"
#include "KeyboardWedge.h"

// Forward declaration of internal server implementation
namespace httplib { class Server; }

class WebServer {
public:
    WebServer(int port, SessionManager& sessionMgr, BarcodeDecoder& decoder, KeyboardWedge& wedge, const std::string& deviceName = "WORKSTATION-01");
    ~WebServer();

    bool Start();
    void Stop();

    int GetPort() const { return m_port; }
    void SetDeviceName(const std::string& name) { m_deviceName = name; }
    std::string GetDeviceName() const { return m_deviceName; }
    KeyboardWedge& GetWedge() { return m_wedge; }
    const KeyboardWedge& GetWedge() const { return m_wedge; }

private:
    void RunServer();

    int m_port;
    std::string m_deviceName;
    SessionManager& m_sessionMgr;
    BarcodeDecoder& m_decoder;
    KeyboardWedge& m_wedge;

    std::unique_ptr<httplib::Server> m_server;
    std::unique_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{false};
};
