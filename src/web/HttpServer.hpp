#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdint>

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCK = INVALID_SOCKET;
    constexpr int SOCK_ERR = SOCKET_ERROR;
    inline int closeSocket(socket_t s) { return ::closesocket(s); }
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <poll.h>
    using socket_t = int;
    constexpr socket_t INVALID_SOCK = -1;
    constexpr int SOCK_ERR = -1;
    inline int closeSocket(socket_t s) { return ::close(s); }
#endif

#include "service/BookingSystem.hpp"
#include "service/ScenarioRunner.hpp"
#include "web/WebDashboard.hpp"

namespace smartcampus::web {

/**
 * @brief Parsed HTTP Request representation.
 */
struct HttpRequest {
    std::string method;
    std::string rawPath;
    std::string path;
    std::string queryString;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> queryParams;
    std::string body;
};

/**
 * @brief Cross-platform Embedded Socket HTTP Server and REST API provider.
 * Coordinates with BookingSystem and ScenarioRunner to serve DASHBOARD_HTML
 * and handle interactive REST endpoints.
 */
class HttpServer {
public:
    explicit HttpServer(std::shared_ptr<service::BookingSystem> system = nullptr, int port = 8080)
        : system_(system ? system : std::make_shared<service::BookingSystem>()),
          port_(port) {
        initSubsystem();
    }

    explicit HttpServer(service::BookingSystem& system, int port = 8080)
        : system_(std::shared_ptr<service::BookingSystem>(&system, [](service::BookingSystem*) {})),
          port_(port) {
        initSubsystem();
    }

    explicit HttpServer(int port)
        : system_(std::make_shared<service::BookingSystem>()),
          port_(port) {
        initSubsystem();
    }

    ~HttpServer() {
        stop();
        cleanupSubsystem();
    }

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    // -------------------------------------------------------------------------
    // Lifecycle Management
    // -------------------------------------------------------------------------

    /**
     * @brief Starts the server. If background is true, spawns a dedicated worker thread.
     */
    bool start(bool background = true) {
        if (running_.load()) {
            return true;
        }

        serverSock_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSock_ == INVALID_SOCK) {
            std::cerr << "[HttpServer] Failed to create socket\n";
            return false;
        }

        int opt = 1;
#if defined(_WIN32)
        ::setsockopt(serverSock_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
        ::setsockopt(serverSock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port = htons(static_cast<uint16_t>(port_));

        if (::bind(serverSock_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCK_ERR) {
            std::cerr << "[HttpServer] Failed to bind to port " << port_ << "\n";
            closeSocket(serverSock_);
            serverSock_ = INVALID_SOCK;
            return false;
        }

        if (::listen(serverSock_, 16) == SOCK_ERR) {
            std::cerr << "[HttpServer] Failed to listen on port " << port_ << "\n";
            closeSocket(serverSock_);
            serverSock_ = INVALID_SOCK;
            return false;
        }

        running_.store(true);

        if (background) {
            workerThread_ = std::thread([this]() { workerLoop(); });
        } else {
            workerLoop();
        }

        return true;
    }

    /**
     * @brief Thread-safe stop mechanism to shut down socket and join worker thread.
     */
    void stop() {
        if (!running_.exchange(false)) {
            return;
        }

        if (serverSock_ != INVALID_SOCK) {
            socket_t s = serverSock_;
            serverSock_ = INVALID_SOCK;
            closeSocket(s);
        }

        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    void run() {
        start(false);
    }

    bool isRunning() const {
        return running_.load();
    }

    int getPort() const {
        return port_;
    }

    std::string getUrl() const {
        return "http://localhost:" + std::to_string(port_);
    }

    std::shared_ptr<service::BookingSystem> getSystem() const {
        return system_;
    }

    // -------------------------------------------------------------------------
    // HTTP Request Handler & Router
    // -------------------------------------------------------------------------

    /**
     * @brief Parse raw HTTP request into structured HttpRequest object.
     * Extracts method, path, query string, query params, headers (lowercase keys,
     * trimmed whitespace), and request body.
     */
    static HttpRequest parseRequest(const std::string& raw) {
        HttpRequest req;
        if (raw.empty()) return req;

        size_t firstLineEnd = raw.find("\r\n");
        size_t headerStart = 0;
        if (firstLineEnd != std::string::npos) {
            headerStart = firstLineEnd + 2;
        } else {
            firstLineEnd = raw.find('\n');
            if (firstLineEnd != std::string::npos) {
                headerStart = firstLineEnd + 1;
            } else {
                firstLineEnd = raw.size();
                headerStart = raw.size();
            }
        }

        std::string firstLine = raw.substr(0, firstLineEnd);
        std::istringstream flss(firstLine);
        flss >> req.method >> req.rawPath;

        size_t qPos = req.rawPath.find('?');
        if (qPos != std::string::npos) {
            req.path = req.rawPath.substr(0, qPos);
            req.queryString = req.rawPath.substr(qPos + 1);
            parseQueryString(req.queryString, req.queryParams);
        } else {
            req.path = req.rawPath;
        }

        // Locate end of headers / start of body
        size_t headersEnd = raw.find("\r\n\r\n");
        size_t bodyStart = std::string::npos;
        if (headersEnd != std::string::npos) {
            bodyStart = headersEnd + 4;
        } else {
            headersEnd = raw.find("\n\n");
            if (headersEnd != std::string::npos) {
                bodyStart = headersEnd + 2;
            } else {
                headersEnd = raw.size();
            }
        }

        if (bodyStart != std::string::npos && bodyStart <= raw.size()) {
            req.body = raw.substr(bodyStart);
        }

        // Parse HTTP header lines into req.headers[key] = value
        if (headersEnd > headerStart) {
            std::string headerSection = raw.substr(headerStart, headersEnd - headerStart);
            std::istringstream hss(headerSection);
            std::string line;
            while (std::getline(hss, line)) {
                // Trim trailing \r, spaces, tabs
                while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) {
                    line.pop_back();
                }
                size_t lStart = line.find_first_not_of(" \t");
                if (lStart == std::string::npos) continue;
                line = line.substr(lStart);

                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string key = line.substr(0, colon);
                    std::string val = line.substr(colon + 1);

                    // Trim key whitespace
                    while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
                    size_t kStart = key.find_first_not_of(" \t");
                    if (kStart != std::string::npos) key = key.substr(kStart);
                    else key.clear();

                    // Convert header key to lowercase
                    std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });

                    // Trim value whitespace
                    while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
                    size_t vStart = val.find_first_not_of(" \t");
                    if (vStart != std::string::npos) val = val.substr(vStart);
                    else val.clear();

                    if (!key.empty()) {
                        req.headers[key] = val;
                    }
                }
            }
        }

        return req;
    }

    /**
     * @brief Pure request parser and dispatcher. Takes raw HTTP request text
     * and returns complete HTTP response string (status line, headers, body).
     */
    std::string handleHttpRequest(const std::string& rawRequest) {
        HttpRequest req = parseRequest(rawRequest);

        // Preflight CORS
        if (req.method == "OPTIONS") {
            return makeResponse(204, "No Content", "text/plain", "");
        }

        // Validate Content-Length if present in headers
        auto clIt = req.headers.find("content-length");
        if (clIt != req.headers.end()) {
            try {
                long long cl = std::stoll(clIt->second);
                constexpr long long MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
                if (cl < 0) {
                    return jsonResponse("{\"error\": \"Invalid Content-Length\"}", 400, "Bad Request");
                }
                if (cl > MAX_BODY_SIZE) {
                    return jsonResponse("{\"error\": \"Payload Too Large (>10MB)\"}", 413, "Payload Too Large");
                }
            } catch (...) {
                return jsonResponse("{\"error\": \"Invalid Content-Length\"}", 400, "Bad Request");
            }
        }

        // Router
        if ((req.method == "GET" || req.method == "HEAD") && (req.path.empty() || req.path == "/" || req.path == "/index.html")) {
            return htmlResponse(std::string(DASHBOARD_HTML));
        }

        if (req.path == "/api/state") {
            return jsonResponse(system_->toJson());
        }

        if (req.path == "/api/book") {
            return handleBook(req);
        }

        if (req.path == "/api/advance-time") {
            return handleAdvanceTime(req);
        }

        if (req.path == "/api/route-review") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            auto r = system_->routeToReview(id);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/approve") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            std::string approverId = getParam(req, "approverId", "USR-005");
            std::string comments = getParam(req, "comments", "Approved via Web Dashboard");
            auto r = system_->approveReservation(id, approverId, comments);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/reject") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            std::string approverId = getParam(req, "approverId", "USR-005");
            std::string reason = getParam(req, "reason", "Rejected via Web Dashboard");
            auto r = system_->rejectReservation(id, approverId, reason);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/request-changes") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            std::string approverId = getParam(req, "approverId", "USR-005");
            std::string notes = getParam(req, "notes", "Revision requested via Web Dashboard");
            std::time_t deadline = getIntParam(req, "deadline", static_cast<int>(system_->getCurrentTime() + 48 * 3600));
            auto r = system_->requestChanges(id, approverId, notes, deadline);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/resubmit") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            auto r = system_->resubmitReservation(id);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/security-clear") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            std::string officerId = getParam(req, "officerId", "USR-006");
            std::string notes = getParam(req, "notes", "Security clearance approved via Web Dashboard");
            auto r = system_->securityClear(id, officerId, notes);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/security-deny") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            std::string officerId = getParam(req, "officerId", "USR-006");
            std::string reason = getParam(req, "reason", "Security clearance denied via Web Dashboard");
            auto r = system_->securityDeny(id, officerId, reason);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/check-in") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            auto r = system_->checkInReservation(id);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/conclude") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            auto r = system_->concludeReservation(id);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/cancel") {
            std::string id = getParam(req, "id", getParam(req, "reservationId"));
            std::string userId = getParam(req, "userId", "USR-001");
            auto r = system_->cancelReservation(id, userId);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/emergency") {
            std::string reason = getParam(req, "reason", "Campus emergency lockdown declared via Web Dashboard");
            auto r = system_->declareCampusEmergency(reason);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/all-clear") {
            auto r = system_->clearCampusEmergency();
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/report-damage") {
            std::string facilityId = getParam(req, "facilityId", "FAC-101");
            std::string issue = getParam(req, "issue", "Facility damage reported via Web Dashboard");
            auto r = system_->reportFacilityDamage(facilityId, issue);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/certify-repair") {
            std::string facilityId = getParam(req, "facilityId", "FAC-101");
            auto r = system_->certifyFacilityRepair(facilityId, true);
            return jsonResult(r.success, r.message);
        }

        if (req.path == "/api/scenario") {
            return handleScenario(req);
        }

        if (req.path == "/api/reset") {
            system_->reset();
            return jsonResponse("{\"success\": true, \"message\": \"Simulation reset to initial default seed data\"}");
        }

        return jsonResponse("{\"error\": \"Endpoint not found\", \"path\": \"" + escapeJson(req.path) + "\"}", 404, "Not Found");
    }

private:
    std::shared_ptr<service::BookingSystem> system_;
    int port_{8080};
    std::atomic<bool> running_{false};
    socket_t serverSock_{INVALID_SOCK};
    std::thread workerThread_;

    // -------------------------------------------------------------------------
    // Socket Subsystem Helpers
    // -------------------------------------------------------------------------
    static void initSubsystem() {
#if defined(_WIN32)
        static std::atomic<bool> initialized{false};
        if (!initialized.exchange(true)) {
            WSADATA wsaData;
            ::WSAStartup(MAKEWORD(2, 2), &wsaData);
        }
#endif
    }

    static void cleanupSubsystem() {
        // Kept open for static process lifetime
    }

    void workerLoop() {
        while (running_.load()) {
            fd_set readFds;
            FD_ZERO(&readFds);
            FD_SET(serverSock_, &readFds);

            timeval tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms non-blocking check

            int sel = ::select(static_cast<int>(serverSock_ + 1), &readFds, nullptr, nullptr, &tv);
            if (!running_.load()) break;

            if (sel > 0 && FD_ISSET(serverSock_, &readFds)) {
                sockaddr_in clientAddr{};
#if defined(_WIN32)
                int clientLen = sizeof(clientAddr);
#else
                socklen_t clientLen = sizeof(clientAddr);
#endif
                socket_t clientSock = ::accept(serverSock_, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
                if (clientSock != INVALID_SOCK) {
                    std::thread([this, clientSock]() {
                        handleClient(clientSock);
                    }).detach();
                }
            }
        }
    }

    void handleClient(socket_t clientSock) {
#if defined(_WIN32)
        DWORD timeout = 2000;
        ::setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
        timeval tv{2, 0};
        ::setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

        std::string rawReq;
        char buf[4096];
        int bytesRead = ::recv(clientSock, buf, sizeof(buf) - 1, 0);
        if (bytesRead > 0) {
            buf[bytesRead] = '\0';
            rawReq.append(buf, bytesRead);

            // Locate header delimiter
            size_t headerEnd = rawReq.find("\r\n\r\n");
            size_t headerDelim = 4;
            if (headerEnd == std::string::npos) {
                headerEnd = rawReq.find("\n\n");
                headerDelim = 2;
            }

            // Read full body if Content-Length present
            size_t clPos = rawReq.find("Content-Length:");
            if (clPos == std::string::npos) clPos = rawReq.find("content-length:");
            if (clPos == std::string::npos) {
                std::string lowerHeaders = rawReq.substr(0, (headerEnd != std::string::npos ? headerEnd : rawReq.size()));
                for (char& c : lowerHeaders) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                clPos = lowerHeaders.find("content-length:");
            }

            if (clPos != std::string::npos) {
                size_t colonPos = rawReq.find(':', clPos);
                if (colonPos != std::string::npos) {
                    size_t valStart = rawReq.find_first_not_of(" \t", colonPos + 1);
                    size_t lineEnd = (valStart != std::string::npos) ? rawReq.find_first_of("\r\n", valStart) : std::string::npos;
                    if (valStart != std::string::npos && lineEnd != std::string::npos) {
                        long long expectedBody = 0;
                        bool validCl = false;
                        try {
                            std::string lenStr = rawReq.substr(valStart, lineEnd - valStart);
                            size_t s = lenStr.find_first_not_of(" \t\r\n");
                            size_t e = lenStr.find_last_not_of(" \t\r\n");
                            if (s != std::string::npos) {
                                expectedBody = std::stoll(lenStr.substr(s, e - s + 1));
                                validCl = true;
                            }
                        } catch (...) {
                            validCl = false;
                        }

                        // Bounds check Content-Length: clamp/validate (reject negative or > 10MB)
                        constexpr long long MAX_BODY_SIZE = 10 * 1024 * 1024; // 10MB
                        if (!validCl || expectedBody < 0 || expectedBody > MAX_BODY_SIZE) {
                            std::string errResp = (!validCl || expectedBody < 0)
                                ? makeResponse(400, "Bad Request", "application/json", "{\"error\": \"Invalid Content-Length\"}")
                                : makeResponse(413, "Payload Too Large", "application/json", "{\"error\": \"Payload Too Large (>10MB)\"}");
#if defined(_WIN32)
                            ::send(clientSock, errResp.data(), static_cast<int>(errResp.size()), 0);
#else
                            ::send(clientSock, errResp.data(), errResp.size(), 0);
#endif
                            closeSocket(clientSock);
                            return;
                        }

                        if (headerEnd != std::string::npos) {
                            size_t currentBody = rawReq.size() - (headerEnd + headerDelim);
                            while (currentBody < static_cast<size_t>(expectedBody)) {
                                int more = ::recv(clientSock, buf, sizeof(buf) - 1, 0);
                                if (more <= 0) break;
                                rawReq.append(buf, more);
                                currentBody += more;
                            }
                        }
                    }
                }
            }

            std::string resp = handleHttpRequest(rawReq);
#if defined(_WIN32)
            ::send(clientSock, resp.data(), static_cast<int>(resp.size()), 0);
#else
            ::send(clientSock, resp.data(), resp.size(), 0);
#endif
        }

        closeSocket(clientSock);
    }

    // -------------------------------------------------------------------------
    // Request Parsing & Helpers
    // -------------------------------------------------------------------------

    static void parseQueryString(const std::string& qs, std::unordered_map<std::string, std::string>& out) {
        size_t pos = 0;
        while (pos < qs.size()) {
            size_t nextAmp = qs.find('&', pos);
            if (nextAmp == std::string::npos) nextAmp = qs.size();
            std::string pair = qs.substr(pos, nextAmp - pos);
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                std::string k = urlDecode(pair.substr(0, eq));
                std::string v = urlDecode(pair.substr(eq + 1));
                out[k] = v;
            } else if (!pair.empty()) {
                out[urlDecode(pair)] = "";
            }
            pos = nextAmp + 1;
        }
    }

    static std::string urlDecode(std::string_view src) {
        std::string res;
        res.reserve(src.size());
        for (size_t i = 0; i < src.size(); ++i) {
            if (src[i] == '%' && i + 2 < src.size()) {
                auto fromHex = [](char c) -> int {
                    if (c >= '0' && c <= '9') return c - '0';
                    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                    return -1;
                };
                int h1 = fromHex(src[i + 1]);
                int h2 = fromHex(src[i + 2]);
                if (h1 >= 0 && h2 >= 0) {
                    res.push_back(static_cast<char>((h1 << 4) | h2));
                    i += 2;
                    continue;
                }
            } else if (src[i] == '+') {
                res.push_back(' ');
                continue;
            }
            res.push_back(src[i]);
        }
        return res;
    }

    std::string getParam(const HttpRequest& req, const std::string& key, const std::string& defaultVal = "") const {
        // 1. Check in body (JSON or Form)
        if (!req.body.empty()) {
            std::string val = extractField(req.body, key);
            if (!val.empty()) return val;
        }

        // 2. Check in query parameters
        auto it = req.queryParams.find(key);
        if (it != req.queryParams.end() && !it->second.empty()) {
            return it->second;
        }

        return defaultVal;
    }

    int getIntParam(const HttpRequest& req, const std::string& key, int defaultVal = 0) const {
        std::string val = getParam(req, key, "");
        if (val.empty()) return defaultVal;
        try {
            return std::stoi(val);
        } catch (...) {
            return defaultVal;
        }
    }

    static std::string extractField(const std::string& src, const std::string& key) {
        // Try JSON: "key" : "val" or "key" : 123
        std::string pattern = "\"" + key + "\"";
        size_t pos = src.find(pattern);
        if (pos != std::string::npos) {
            size_t colon = src.find(':', pos + pattern.size());
            if (colon != std::string::npos) {
                size_t startVal = src.find_first_not_of(" \t\r\n", colon + 1);
                if (startVal != std::string::npos) {
                    if (src[startVal] == '"') {
                        size_t endQuote = src.find('"', startVal + 1);
                        if (endQuote != std::string::npos) {
                            return src.substr(startVal + 1, endQuote - (startVal + 1));
                        }
                    } else {
                        size_t endVal = src.find_first_of(",}\r\n \t", startVal);
                        if (endVal == std::string::npos) endVal = src.size();
                        return src.substr(startVal, endVal - startVal);
                    }
                }
            }
        }

        // Try Form: key=value
        std::string formPattern = key + "=";
        pos = src.find(formPattern);
        while (pos != std::string::npos) {
            if (pos == 0 || src[pos - 1] == '&' || src[pos - 1] == '?' || src[pos - 1] == '\n' || src[pos - 1] == ' ') {
                size_t valStart = pos + formPattern.size();
                size_t valEnd = src.find_first_of("& \r\n\t", valStart);
                if (valEnd == std::string::npos) valEnd = src.size();
                return urlDecode(src.substr(valStart, valEnd - valStart));
            }
            pos = src.find(formPattern, pos + 1);
        }

        return "";
    }

    static std::string escapeJson(std::string_view s) {
        std::string out;
        out.reserve(s.size() + 16);
        for (char c : s) {
            if (c == '"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\b') out += "\\b";
            else if (c == '\f') out += "\\f";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                out += buf;
            } else {
                out += c;
            }
        }
        return out;
    }

    static std::string makeResponse(int statusCode, const std::string& statusText,
                                    const std::string& contentType, const std::string& body) {
        std::ostringstream ss;
        ss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
        ss << "Content-Type: " << contentType << "\r\n";
        ss << "Content-Length: " << body.size() << "\r\n";
        ss << "Connection: close\r\n";
        ss << "Access-Control-Allow-Origin: *\r\n";
        ss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        ss << "Access-Control-Allow-Headers: Content-Type\r\n";
        ss << "\r\n";
        ss << body;
        return ss.str();
    }

    static std::string jsonResponse(const std::string& json, int status = 200, const std::string& text = "OK") {
        return makeResponse(status, text, "application/json; charset=utf-8", json);
    }

    static std::string htmlResponse(const std::string& html, int status = 200, const std::string& text = "OK") {
        return makeResponse(status, text, "text/html; charset=utf-8", html);
    }

    static std::string jsonResult(bool success, const std::string& message) {
        return jsonResponse(
            "{\"success\": " + std::string(success ? "true" : "false") +
            ", \"message\": \"" + escapeJson(message) + "\"}",
            success ? 200 : 400,
            success ? "OK" : "Bad Request"
        );
    }

    // -------------------------------------------------------------------------
    // Specific Endpoint Handlers
    // -------------------------------------------------------------------------

    std::string handleBook(const HttpRequest& req) {
        std::string requesterId = getParam(req, "requesterId", "USR-001");
        std::string facilityId = getParam(req, "facilityId", "FAC-101");
        std::string title = getParam(req, "title", "New Campus Event");
        std::string eventType = getParam(req, "eventType", "Workshop");
        int attendance = getIntParam(req, "attendance", 25);
        std::string customId = getParam(req, "customId", "");

        std::time_t now = system_->getCurrentTime();
        std::time_t start = getIntParam(req, "startTime", static_cast<int>(now + 24 * 3600));
        std::time_t end = getIntParam(req, "endTime", static_cast<int>(start + 2 * 3600));

        std::string resId = system_->createReservation(
            requesterId, facilityId, title, eventType, attendance, start, end, {}, {}, customId
        );

        if (resId.empty()) {
            return jsonResponse("{\"success\": false, \"message\": \"Failed to create reservation: facility invalid or time conflict\"}", 400, "Bad Request");
        }

        auto subRes = system_->submitReservation(resId);
        if (!subRes.success) {
            return jsonResponse("{\"success\": false, \"reservationId\": \"" + escapeJson(resId) + "\", \"message\": \"" + escapeJson(subRes.message) + "\"}", 400, "Bad Request");
        }

        return jsonResponse("{\"success\": true, \"reservationId\": \"" + escapeJson(resId) + "\", \"message\": \"Reservation created and submitted (15-min soft lock active)\"}");
    }

    std::string handleAdvanceTime(const HttpRequest& req) {
        int seconds = getIntParam(req, "seconds", 900);
        system_->advanceTime(seconds);
        std::time_t t = system_->getCurrentTime();
        return jsonResponse(
            "{\"success\": true, \"currentTime\": " + std::to_string(t) +
            ", \"formattedTime\": \"" + service::BookingSystem::formatTime(t) + "\"}"
        );
    }

    std::string handleScenario(const HttpRequest& req) {
        std::string sVal = getParam(req, "scenario", "1");
        std::ostringstream ss;
        service::ScenarioRunner runner(ss, true);
        bool success = false;

        if (sVal == "all" || sVal == "7") {
            success = runner.runAll();
            applyScenarioToSystem(7);
        } else {
            int num = 1;
            try { num = std::stoi(sVal); } catch (...) { num = 1; }
            switch (num) {
                case 1:
                    success = runner.runHappyPath();
                    applyScenarioToSystem(1);
                    break;
                case 2:
                    success = runner.runLargeEventSecurityScenario();
                    applyScenarioToSystem(2);
                    break;
                case 3:
                    success = runner.runRevisionLoopScenario();
                    applyScenarioToSystem(3);
                    break;
                case 4:
                    success = runner.runFacilityLogisticsScenario();
                    applyScenarioToSystem(4);
                    break;
                case 5:
                    success = runner.runEmergencyLockdownScenario();
                    applyScenarioToSystem(5);
                    break;
                case 6:
                    success = runner.runCancellationRulesScenario();
                    applyScenarioToSystem(6);
                    break;
                default:
                    success = runner.runHappyPath();
                    applyScenarioToSystem(1);
                    break;
            }
        }

        return jsonResponse(
            "{\"success\": " + std::string(success ? "true" : "false") +
            ", \"scenario\": \"" + escapeJson(sVal) +
            "\", \"output\": \"" + escapeJson(ss.str()) +
            "\", \"message\": \"Scenario " + escapeJson(sVal) + " executed\"}"
        );
    }

    void applyScenarioToSystem(int num) {
        if (num == 1) {
            std::time_t start = service::BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
            std::time_t end   = service::BookingSystem::makeTime(2026, 9, 5, 16, 0, 0);
            std::string id = system_->createReservation(
                "USR-001", "FAC-104", "Autonomous Drone Workshop", "Student Workshop",
                20, start, end, {}, {}, "RES-HAPPY-101"
            );
            if (!id.empty()) {
                system_->submitReservation(id);
                system_->routeToReview(id);
                system_->approveReservation(id, "USR-005", "Approved: verified organization eligibility");
            }
        } else if (num == 2) {
            std::time_t start = service::BookingSystem::makeTime(2026, 9, 10, 13, 0, 0);
            std::time_t end   = service::BookingSystem::makeTime(2026, 9, 10, 17, 0, 0);
            std::string id = system_->createReservation(
                "USR-002", "FAC-101", "National Collegiate Debate Championship", "Competition",
                220, start, end, {}, {}, "RES-SEC-201"
            );
            if (!id.empty()) {
                system_->submitReservation(id);
                system_->routeToReview(id);
                system_->evaluateSecurity(id);
                system_->securityClear(id, "USR-006", "Security detail approved: 2 officers assigned.");
            }
        } else if (num == 3) {
            std::time_t start = service::BookingSystem::makeTime(2026, 9, 12, 10, 0, 0);
            std::time_t end   = service::BookingSystem::makeTime(2026, 9, 12, 15, 0, 0);
            std::string id = system_->createReservation(
                "USR-001", "FAC-103", "Robotics Hackathon & Showcase", "Hackathon",
                80, start, end, {}, {}, "RES-REV-301"
            );
            if (!id.empty()) {
                system_->submitReservation(id);
                system_->routeToReview(id);
                system_->requestChanges(id, "USR-005", "Please provide detailed table layout diagram and 36-inch aisle clearance.", system_->getCurrentTime() + 48 * 3600);
            }
        } else if (num == 4) {
            std::time_t start = service::BookingSystem::makeTime(2026, 9, 3, 10, 0, 0);
            std::time_t end   = service::BookingSystem::makeTime(2026, 9, 3, 14, 0, 0);
            std::string id = system_->createReservation(
                "USR-003", "FAC-103", "Annual Faculty Research Symposium", "Symposium",
                50, start, end, {}, {}, "RES-FAC-401"
            );
            if (!id.empty()) {
                system_->submitReservation(id);
                system_->routeToReview(id);
                system_->approveReservation(id, "USR-005", "Approved");
                system_->openSetupWindow("FAC-103");
            }
        } else if (num == 5) {
            system_->declareCampusEmergency("Active perimeter security alert and severe storm");
        } else if (num == 6) {
            std::time_t start = service::BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
            std::time_t end   = service::BookingSystem::makeTime(2026, 9, 5, 17, 0, 0);
            std::string id = system_->createReservation(
                "USR-001", "FAC-102", "STEM Robotics Workshop", "Workshop",
                30, start, end, {}, {}, "RES-CANCEL-601"
            );
            if (!id.empty()) {
                system_->submitReservation(id);
                system_->routeToReview(id);
                system_->approveReservation(id, "USR-005", "Approved");
            }
        } else if (num == 7) {
            applyScenarioToSystem(1);
            applyScenarioToSystem(2);
            applyScenarioToSystem(3);
        }
    }
};

} // namespace smartcampus::web
