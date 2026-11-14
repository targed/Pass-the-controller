#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <ctime>
#include <chrono>
#include <atomic>
#include <algorithm>

namespace domain {

/**
 * @brief Represents a dispatched notification, alert, or audit notice.
 */
struct Notification {
    std::string id;
    std::string recipientEmail;
    std::string title;
    std::string message;
    std::time_t timestamp{0};
};

/**
 * @brief Thread-safe singleton notification manager maintaining an in-memory queue
 * of dispatched receipts, security alerts, and custodial turnover notices.
 */
class NotificationService {
public:
    static NotificationService& getInstance() {
        static NotificationService instance;
        return instance;
    }

    NotificationService() = default;
    ~NotificationService() = default;

    NotificationService(const NotificationService&) = delete;
    NotificationService& operator=(const NotificationService&) = delete;

    /**
     * @brief Dispatches a notification struct and appends to the queue.
     */
    void sendNotification(Notification notification) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (notification.id.empty()) {
            notification.id = generateId();
        }
        if (notification.timestamp == 0) {
            notification.timestamp = std::time(nullptr);
        }
        notifications_.push_back(std::move(notification));
    }

    /**
     * @brief Dispatches a notification by recipient, title, and body message.
     */
    void sendNotification(std::string recipientEmail, std::string title, std::string message) {
        Notification notification;
        notification.id = generateId();
        notification.recipientEmail = std::move(recipientEmail);
        notification.title = std::move(title);
        notification.message = std::move(message);
        notification.timestamp = std::time(nullptr);
        sendNotification(std::move(notification));
    }

    /**
     * @brief Sends formal reservation approval confirmation to requester.
     */
    void sendConfirmation(const std::string& recipientEmail,
                          const std::string& reservationId,
                          const std::string& details) {
        sendNotification(recipientEmail,
                         "Reservation Confirmed: " + reservationId,
                         "Your reservation request " + reservationId + " has been approved. " + details);
    }

    /**
     * @brief Dispatches security alerts for high-risk events or lockdown events.
     */
    void sendSecurityAlert(const std::string& reservationId, const std::string& details) {
        sendNotification("security-ops@campus.edu",
                         "SECURITY ALERT: Reservation " + reservationId,
                         "Security action triggered for reservation " + reservationId + ". Details: " + details);
    }

    /**
     * @brief Dispatches custodial notice for facility setup or turnover.
     */
    void sendCustodialNotice(const std::string& facilityId, const std::string& details) {
        sendNotification("custodial-facilities@campus.edu",
                         "Custodial Turnover Notice: " + facilityId,
                         "Facility turnover required for room " + facilityId + ". Details: " + details);
    }

    /**
     * @brief Retrieves up to `limit` most recent notifications.
     */
    std::vector<Notification> getRecent(size_t limit = 50) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (notifications_.size() <= limit) {
            return notifications_;
        }
        return std::vector<Notification>(notifications_.end() - static_cast<ptrdiff_t>(limit), notifications_.end());
    }

    /**
     * @brief Retrieves all notifications in queue.
     */
    std::vector<Notification> getAllNotifications() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return notifications_;
    }

    /**
     * @brief Clears all queued notifications (primarily used in test fixtures).
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        notifications_.clear();
    }

    /**
     * @brief Total number of notifications stored.
     */
    size_t count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return notifications_.size();
    }

private:
    std::string generateId() {
        static std::atomic<uint64_t> counter{1000};
        return "notif-" + std::to_string(++counter);
    }

    mutable std::mutex mutex_;
    std::vector<Notification> notifications_;
};

} // namespace domain
