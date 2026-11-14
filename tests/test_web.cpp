#include <iostream>
#include <cassert>
#include <string>
#include <chrono>
#include <thread>

#include "web/HttpServer.hpp"
#include "web/WebDashboard.hpp"

using namespace smartcampus::web;

void assertOrThrow(bool condition, const std::string& msg) {
    if (!condition) {
        std::cerr << "Assertion FAILED: " << msg << std::endl;
        std::exit(1);
    }
}

int main() {
    std::cout << "=== Running Web Dashboard & Server Verification Tests ===" << std::endl;

    // 1. Verify DASHBOARD_HTML contents
    std::cout << "[Test 1] Verifying DASHBOARD_HTML content...";
    assertOrThrow(!DASHBOARD_HTML.empty(), "DASHBOARD_HTML must not be empty");
    assertOrThrow(DASHBOARD_HTML.find("<!DOCTYPE html>") != std::string_view::npos, "Missing DOCTYPE in HTML");
    assertOrThrow(DASHBOARD_HTML.find("SmartCampus") != std::string_view::npos, "Missing brand in HTML");
    assertOrThrow(DASHBOARD_HTML.find("/api/state") != std::string_view::npos, "Missing /api/state polling in HTML");
    assertOrThrow(DASHBOARD_HTML.find("kanbanBoard") != std::string_view::npos, "Missing kanbanBoard in HTML");
    assertOrThrow(DASHBOARD_HTML.find("facilityGrid") != std::string_view::npos, "Missing facilityGrid in HTML");
    assertOrThrow(DASHBOARD_HTML.find("logStream") != std::string_view::npos, "Missing logStream in HTML");
    std::cout << " PASS\n";

    // 2. Initialize HttpServer and verify request handling
    std::cout << "[Test 2] Verifying HttpServer initialization and request router...";
    auto sys = std::make_shared<service::BookingSystem>();
    HttpServer server(sys, 18080);
    assertOrThrow(server.getPort() == 18080, "Port should be 18080");
    assertOrThrow(!server.isRunning(), "Server should not be running yet");
    std::cout << " PASS\n";

    // 3. Test GET /
    std::cout << "[Test 3] Testing GET / (Dashboard HTML)...";
    std::string respRoot = server.handleHttpRequest("GET / HTTP/1.1\r\nHost: localhost:18080\r\n\r\n");
    assertOrThrow(respRoot.find("HTTP/1.1 200 OK") != std::string::npos, "GET / must return 200 OK");
    assertOrThrow(respRoot.find("Content-Type: text/html") != std::string::npos, "GET / must have text/html");
    assertOrThrow(respRoot.find("SmartCampus") != std::string_view::npos, "GET / response body must contain dashboard");
    std::cout << " PASS\n";

    // 4. Test GET /api/state
    std::cout << "[Test 4] Testing GET /api/state...";
    std::string respState = server.handleHttpRequest("GET /api/state HTTP/1.1\r\nHost: localhost:18080\r\n\r\n");
    assertOrThrow(respState.find("HTTP/1.1 200 OK") != std::string::npos, "GET /api/state must return 200 OK");
    assertOrThrow(respState.find("Content-Type: application/json") != std::string::npos, "Content-Type must be json");
    assertOrThrow(respState.find("\"facilities\"") != std::string::npos, "JSON must contain facilities array");
    assertOrThrow(respState.find("\"reservations\"") != std::string::npos, "JSON must contain reservations array");
    assertOrThrow(respState.find("\"currentTime\"") != std::string::npos, "JSON must contain currentTime");
    std::cout << " PASS\n";

    // 5. Test POST /api/advance-time
    std::cout << "[Test 5] Testing POST /api/advance-time...";
    std::string respAdv = server.handleHttpRequest(
        "POST /api/advance-time HTTP/1.1\r\n"
        "Host: localhost:18080\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 17\r\n\r\n"
        "{\"seconds\": 3600}"
    );
    assertOrThrow(respAdv.find("HTTP/1.1 200 OK") != std::string::npos, "POST /api/advance-time must return 200");
    assertOrThrow(respAdv.find("\"success\": true") != std::string::npos, "advance-time must report success");
    std::cout << " PASS\n";

    // 6. Test POST /api/book
    std::cout << "[Test 6] Testing POST /api/book...";
    std::string respBook = server.handleHttpRequest(
        "POST /api/book HTTP/1.1\r\n"
        "Host: localhost:18080\r\n"
        "Content-Type: application/json\r\n\r\n"
        "{\"requesterId\": \"USR-001\", \"facilityId\": \"FAC-104\", \"title\": \"AI Workshop\", \"attendance\": 25}"
    );
    assertOrThrow(respBook.find("HTTP/1.1 200 OK") != std::string::npos, "POST /api/book must return 200");
    assertOrThrow(respBook.find("\"success\": true") != std::string::npos, "book must report success");
    assertOrThrow(respBook.find("\"reservationId\"") != std::string::npos, "book must return reservationId");

    // Extract reservationId from response
    size_t idPos = respBook.find("\"reservationId\": \"");
    assertOrThrow(idPos != std::string::npos, "Must contain reservationId key");
    size_t idStart = idPos + 18;
    size_t idEnd = respBook.find("\"", idStart);
    std::string createdId = respBook.substr(idStart, idEnd - idStart);
    std::cout << " created " << createdId << " PASS\n";

    // 7. Test POST /api/route-review
    std::cout << "[Test 7] Testing POST /api/route-review...";
    std::string respRoute = server.handleHttpRequest(
        "POST /api/route-review HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{\"id\": \"" + createdId + "\"}"
    );
    assertOrThrow(respRoute.find("HTTP/1.1 200 OK") != std::string::npos, "route-review must return 200");
    std::cout << " PASS\n";

    // 8. Test POST /api/approve
    std::cout << "[Test 8] Testing POST /api/approve...";
    std::string respApprove = server.handleHttpRequest(
        "POST /api/approve HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{\"id\": \"" + createdId + "\", \"approverId\": \"USR-005\"}"
    );
    assertOrThrow(respApprove.find("HTTP/1.1 200 OK") != std::string::npos, "approve must return 200");
    std::cout << " PASS\n";

    // 9. Test Emergency Lockdown & All-Clear
    std::cout << "[Test 9] Testing POST /api/emergency and /api/all-clear...";
    std::string respEmerg = server.handleHttpRequest(
        "POST /api/emergency HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{\"reason\": \"Test severe weather lockdown\"}"
    );
    assertOrThrow(respEmerg.find("HTTP/1.1 200 OK") != std::string::npos, "emergency must return 200");
    auto fac104 = sys->getFacility("FAC-104");
    assertOrThrow(fac104 && fac104->getStatus() == domain::RoomStatus::EmergencyHold, "FAC-104 should be in EmergencyHold");

    std::string respClear = server.handleHttpRequest(
        "POST /api/all-clear HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{}"
    );
    assertOrThrow(respClear.find("HTTP/1.1 200 OK") != std::string::npos, "all-clear must return 200");
    fac104 = sys->getFacility("FAC-104");
    assertOrThrow(fac104 && fac104->getStatus() == domain::RoomStatus::Available, "FAC-104 should be restored to Available");
    std::cout << " PASS\n";

    // 10. Test Facility Damage and Repair
    std::cout << "[Test 10] Testing POST /api/report-damage and /api/certify-repair...";
    std::string respDmg = server.handleHttpRequest(
        "POST /api/report-damage HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{\"facilityId\": \"FAC-101\", \"issue\": \"Projector bulb blown\"}"
    );
    assertOrThrow(respDmg.find("HTTP/1.1 200 OK") != std::string::npos, "report-damage must return 200");
    auto fac101 = sys->getFacility("FAC-101");
    assertOrThrow(fac101 && fac101->getStatus() == domain::RoomStatus::OutOfService, "FAC-101 should be OutOfService");

    std::string respRepair = server.handleHttpRequest(
        "POST /api/certify-repair HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{\"facilityId\": \"FAC-101\"}"
    );
    assertOrThrow(respRepair.find("HTTP/1.1 200 OK") != std::string::npos, "certify-repair must return 200");
    fac101 = sys->getFacility("FAC-101");
    assertOrThrow(fac101 && fac101->getStatus() == domain::RoomStatus::Available, "FAC-101 should be Available");
    std::cout << " PASS\n";

    // 11. Test Scenario Execution endpoint
    std::cout << "[Test 11] Testing POST /api/scenario...";
    std::string respScen = server.handleHttpRequest(
        "POST /api/scenario HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{\"scenario\": 1}"
    );
    assertOrThrow(respScen.find("HTTP/1.1 200 OK") != std::string::npos, "scenario must return 200");
    assertOrThrow(respScen.find("\"success\": true") != std::string::npos, "scenario 1 must succeed");
    std::cout << " PASS\n";

    // 12. Test Reset endpoint
    std::cout << "[Test 12] Testing POST /api/reset...";
    std::string respReset = server.handleHttpRequest("POST /api/reset HTTP/1.1\r\nHost: localhost:18080\r\n\r\n{}");
    assertOrThrow(respReset.find("HTTP/1.1 200 OK") != std::string::npos, "reset must return 200");
    std::cout << " PASS\n";

    // 13. Test Real Socket Networking (Background server thread & loopback client)
    std::cout << "[Test 13] Testing real socket listening, accept, and shutdown...";
    bool started = server.start(true);
    assertOrThrow(started, "Server must start successfully on port 18080");
    assertOrThrow(server.isRunning(), "Server isRunning() must be true");

    // Give socket listener a moment to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Connect via loopback client socket
    socket_t clientSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assertOrThrow(clientSock != INVALID_SOCK, "Client socket creation failed");

    sockaddr_in saddr{};
    saddr.sin_family = AF_INET;
#if defined(_WIN32)
    ::InetPtonA(AF_INET, "127.0.0.1", &saddr.sin_addr);
#else
    ::inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr);
#endif
    saddr.sin_port = htons(18080);

    int conn = ::connect(clientSock, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
    assertOrThrow(conn != SOCK_ERR, "Client connection to localhost:18080 failed");

    std::string req = "GET /api/state HTTP/1.1\r\nHost: localhost:18080\r\nConnection: close\r\n\r\n";
    ::send(clientSock, req.data(), static_cast<int>(req.size()), 0);

    char netBuf[4096];
    int recvBytes = ::recv(clientSock, netBuf, sizeof(netBuf) - 1, 0);
    assertOrThrow(recvBytes > 0, "Should have received response bytes over socket");
    netBuf[recvBytes] = '\0';
    std::string netResp(netBuf, recvBytes);
    assertOrThrow(netResp.find("HTTP/1.1 200 OK") != std::string::npos, "Socket response must be 200 OK");
    assertOrThrow(netResp.find("\"facilities\"") != std::string::npos, "Socket response must contain facilities JSON");

    closeSocket(clientSock);

    // Stop server and verify clean termination
    server.stop();
    assertOrThrow(!server.isRunning(), "Server isRunning() must be false after stop");
    std::cout << " PASS\n";

    std::cout << "\n>>> ALL 13 WEB SERVER & DASHBOARD TESTS PASSED SUCCESSFULLY! <<<\n";
    return 0;
}
