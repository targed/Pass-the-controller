#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <ctime>
#include <atomic>
#include <utility>

namespace domain {

/**
 * @brief Eleven formal states governing the end-to-end lifecycle of an event reservation.
 */
enum class ReservationStatus {
    Draft,          ///< Requester selects room, date, headcount, equipment; invisible to approvers
    Submitted,      ///< Conflict check passed; temporary 15-minute soft lock placed on room calendar
    UnderReview,    ///< Routed to Event Approver queue for initial policy evaluation
    SecurityReview, ///< High-risk gate: headcount > 100 or after-hours, requires security officer
    Approved,       ///< Formally approved and confirmed; room calendar slot locked permanently
    NeedsRevision,  ///< Approver requested adjustments; requester must revise before deadline
    Expired,        ///< Revision deadline passed without resubmission; soft hold released
    Rejected,       ///< Request denied due to policy violation or unresolvable conflict; terminal
    InProgress,     ///< Event start time reached; smart doors unlocked; attendees present
    Completed,      ///< Event concluded; smart doors locked; custodial turnover triggered; terminal
    Cancelled       ///< Booking cancelled by requester (enforces 24h lead time if approved); terminal
};

/**
 * @brief Convert ReservationStatus enum to human-readable string representation.
 */
inline std::string reservationStatusToString(ReservationStatus status) {
    switch (status) {
        case ReservationStatus::Draft:          return "Draft";
        case ReservationStatus::Submitted:      return "Submitted";
        case ReservationStatus::UnderReview:    return "UnderReview";
        case ReservationStatus::SecurityReview: return "SecurityReview";
        case ReservationStatus::Approved:       return "Approved";
        case ReservationStatus::NeedsRevision:  return "NeedsRevision";
        case ReservationStatus::Expired:        return "Expired";
        case ReservationStatus::Rejected:       return "Rejected";
        case ReservationStatus::InProgress:     return "InProgress";
        case ReservationStatus::Completed:      return "Completed";
        case ReservationStatus::Cancelled:      return "Cancelled";
    }
    return "Unknown";
}

/**
 * @brief Convert string representation back to ReservationStatus enum.
 */
inline ReservationStatus stringToReservationStatus(std::string_view str) {
    if (str == "Draft")          return ReservationStatus::Draft;
    if (str == "Submitted")      return ReservationStatus::Submitted;
    if (str == "UnderReview")    return ReservationStatus::UnderReview;
    if (str == "SecurityReview") return ReservationStatus::SecurityReview;
    if (str == "Approved")       return ReservationStatus::Approved;
    if (str == "NeedsRevision")  return ReservationStatus::NeedsRevision;
    if (str == "Expired")        return ReservationStatus::Expired;
    if (str == "Rejected")       return ReservationStatus::Rejected;
    if (str == "InProgress")     return ReservationStatus::InProgress;
    if (str == "Completed")      return ReservationStatus::Completed;
    if (str == "Cancelled")      return ReservationStatus::Cancelled;
    return ReservationStatus::Draft;
}

/**
 * @brief Represents an equipment request item associated with a reservation.
 */
class EquipmentRequest {
public:
    EquipmentRequest(std::string requestId,
                     std::string equipmentType,
                     int quantity = 1,
                     bool isFulfilled = false)
        : requestId_(std::move(requestId)),
          equipmentType_(std::move(equipmentType)),
          quantity_(quantity),
          isFulfilled_(isFulfilled) {}

    const std::string& getRequestId() const { return requestId_; }
    const std::string& getEquipmentType() const { return equipmentType_; }
    int getQuantity() const { return quantity_; }
    bool isFulfilled() const { return isFulfilled_; }

    void setFulfilled(bool fulfilled) { isFulfilled_ = fulfilled; }
    void setQuantity(int quantity) { quantity_ = quantity; }

private:
    std::string requestId_;
    std::string equipmentType_;
    int quantity_{1};
    bool isFulfilled_{false};
};

/**
 * @brief Represents staffing or service support (Security, Custodial, AV).
 */
class SupportRequest {
public:
    SupportRequest(std::string requestId,
                   std::string supportType,
                   std::string notes = "",
                   bool isAssigned = false,
                   std::string assignedOfficerId = "")
        : requestId_(std::move(requestId)),
          supportType_(std::move(supportType)),
          notes_(std::move(notes)),
          isAssigned_(isAssigned),
          assignedOfficerId_(std::move(assignedOfficerId)) {}

    const std::string& getRequestId() const { return requestId_; }
    const std::string& getSupportType() const { return supportType_; }
    const std::string& getNotes() const { return notes_; }
    bool isAssigned() const { return isAssigned_; }
    const std::string& getAssignedOfficerId() const { return assignedOfficerId_; }

    void assignOfficer(std::string officerId) {
        assignedOfficerId_ = std::move(officerId);
        isAssigned_ = true;
    }

    void setNotes(std::string notes) { notes_ = std::move(notes); }
    void setIsAssigned(bool assigned) { isAssigned_ = assigned; }

private:
    std::string requestId_;
    std::string supportType_;
    std::string notes_;
    bool isAssigned_{false};
    std::string assignedOfficerId_;
};

/**
 * @brief Audit record of an approver's decision on a reservation.
 */
class Approval {
public:
    Approval(std::string approvalId,
             std::string approverId,
             std::string decision,
             std::string comments = "",
             std::time_t decidedAt = 0)
        : approvalId_(std::move(approvalId)),
          approverId_(std::move(approverId)),
          decision_(std::move(decision)),
          comments_(std::move(comments)),
          decidedAt_(decidedAt == 0 ? std::time(nullptr) : decidedAt) {}

    const std::string& getApprovalId() const { return approvalId_; }
    const std::string& getApproverId() const { return approverId_; }
    const std::string& getDecision() const { return decision_; }
    const std::string& getComments() const { return comments_; }
    std::time_t getDecidedAt() const { return decidedAt_; }

private:
    std::string approvalId_;
    std::string approverId_;
    std::string decision_;
    std::string comments_;
    std::time_t decidedAt_{0};
};

/**
 * @brief Aggregate Root coordinating the event booking lifecycle, approvals,
 * equipment, support, and security evaluation.
 */
class Reservation {
public:
    Reservation(std::string reservationId,
                std::string requesterId,
                std::string facilityId,
                std::string eventTitle,
                std::string eventType,
                int expectedAttendance,
                std::time_t startTime,
                std::time_t endTime,
                std::time_t createdAt = 0,
                ReservationStatus status = ReservationStatus::Draft)
        : reservationId_(std::move(reservationId)),
          requesterId_(std::move(requesterId)),
          facilityId_(std::move(facilityId)),
          eventTitle_(std::move(eventTitle)),
          eventType_(std::move(eventType)),
          expectedAttendance_(expectedAttendance),
          startTime_(startTime),
          endTime_(endTime),
          createdAt_(createdAt == 0 ? std::time(nullptr) : createdAt),
          status_(status),
          holdExpiresAt_(0),
          requiresSecurity_(false) {
        evaluateSecurityRequirement();
    }

    // Default constructor for in-memory collections
    Reservation()
        : expectedAttendance_(0),
          startTime_(0),
          endTime_(0),
          createdAt_(std::time(nullptr)),
          status_(ReservationStatus::Draft),
          holdExpiresAt_(0),
          requiresSecurity_(false) {}

    /**
     * @brief Checks if the reservation time window falls outside normal campus hours (08:00 - 20:00).
     */
    bool isAfterHours() const {
        if (startTime_ == 0 && endTime_ == 0) {
            return false;
        }
        std::tm startTm{};
        std::tm endTm{};
#if defined(_WIN32)
        localtime_s(&startTm, &startTime_);
        localtime_s(&endTm, &endTime_);
#else
        localtime_r(&startTime_, &startTm);
        localtime_r(&endTime_, &endTm);
#endif
        // Normal campus hours: 08:00 (8 AM) to 20:00 (8 PM)
        if (startTm.tm_hour < 8 || startTm.tm_hour >= 20) return true;
        if (endTm.tm_hour < 8 || endTm.tm_hour > 20 || (endTm.tm_hour == 20 && endTm.tm_min > 0)) return true;
        if (startTm.tm_mday != endTm.tm_mday || startTm.tm_mon != endTm.tm_mon || startTm.tm_year != endTm.tm_year) return true;
        return false;
    }

    /**
     * @brief Evaluates whether high-risk gate applies (headcount > 100 or after-hours).
     */
    bool evaluateSecurityRequirement() {
        bool afterHours = isAfterHours();
        requiresSecurity_ = (expectedAttendance_ > 100 || afterHours);
        return requiresSecurity_;
    }

    // Compositional management
    void addEquipment(EquipmentRequest request) {
        equipmentRequests_.push_back(std::move(request));
    }

    void addEquipment(std::string equipmentType, int quantity) {
        static std::atomic<uint64_t> eqCounter{100};
        std::string eqId = "eq-" + std::to_string(++eqCounter);
        equipmentRequests_.emplace_back(std::move(eqId), std::move(equipmentType), quantity);
    }

    void addSupportRequest(SupportRequest request) {
        supportRequests_.push_back(std::move(request));
    }

    void addSupportRequest(std::string supportType, std::string notes) {
        static std::atomic<uint64_t> supCounter{100};
        std::string supId = "sup-" + std::to_string(++supCounter);
        supportRequests_.emplace_back(std::move(supId), std::move(supportType), std::move(notes));
    }

    void recordApproval(Approval approval) {
        approvals_.push_back(std::move(approval));
    }

    void recordApproval(std::string approverId, std::string decision, std::string comments) {
        static std::atomic<uint64_t> appCounter{100};
        std::string appId = "appr-" + std::to_string(++appCounter);
        approvals_.emplace_back(std::move(appId), std::move(approverId), std::move(decision), std::move(comments));
    }

    // Accessors
    const std::string& getReservationId() const { return reservationId_; }
    const std::string& getRequesterId() const { return requesterId_; }
    const std::string& getFacilityId() const { return facilityId_; }
    const std::string& getEventTitle() const { return eventTitle_; }
    const std::string& getEventType() const { return eventType_; }
    int getExpectedAttendance() const { return expectedAttendance_; }
    std::time_t getStartTime() const { return startTime_; }
    std::time_t getEndTime() const { return endTime_; }
    std::time_t getCreatedAt() const { return createdAt_; }
    ReservationStatus getStatus() const { return status_; }
    std::time_t getHoldExpiresAt() const { return holdExpiresAt_; }
    bool getRequiresSecurity() const { return requiresSecurity_; }

    const std::vector<EquipmentRequest>& getEquipmentRequests() const { return equipmentRequests_; }
    std::vector<EquipmentRequest>& getEquipmentRequests() { return equipmentRequests_; }

    const std::vector<SupportRequest>& getSupportRequests() const { return supportRequests_; }
    std::vector<SupportRequest>& getSupportRequests() { return supportRequests_; }

    const std::vector<Approval>& getApprovals() const { return approvals_; }
    std::vector<Approval>& getApprovals() { return approvals_; }

    // Mutators
    void setReservationId(std::string reservationId) { reservationId_ = std::move(reservationId); }
    void setRequesterId(std::string requesterId) { requesterId_ = std::move(requesterId); }
    void setFacilityId(std::string facilityId) { facilityId_ = std::move(facilityId); }
    void setEventTitle(std::string eventTitle) { eventTitle_ = std::move(eventTitle); }
    void setEventType(std::string eventType) { eventType_ = std::move(eventType); }
    void setExpectedAttendance(int attendance) {
        expectedAttendance_ = attendance;
        evaluateSecurityRequirement();
    }
    void setStartTime(std::time_t startTime) {
        startTime_ = startTime;
        evaluateSecurityRequirement();
    }
    void setEndTime(std::time_t endTime) {
        endTime_ = endTime;
        evaluateSecurityRequirement();
    }
    void setCreatedAt(std::time_t createdAt) { createdAt_ = createdAt; }
    void setStatus(ReservationStatus status) { status_ = status; }
    void setHoldExpiresAt(std::time_t holdExpiresAt) { holdExpiresAt_ = holdExpiresAt; }
    void setRequiresSecurity(bool requiresSecurity) { requiresSecurity_ = requiresSecurity; }

private:
    std::string reservationId_;
    std::string requesterId_;
    std::string facilityId_;
    std::string eventTitle_;
    std::string eventType_;
    int expectedAttendance_{0};
    std::time_t startTime_{0};
    std::time_t endTime_{0};
    std::time_t createdAt_{0};
    ReservationStatus status_{ReservationStatus::Draft};
    std::time_t holdExpiresAt_{0};
    bool requiresSecurity_{false};

    std::vector<EquipmentRequest> equipmentRequests_;
    std::vector<SupportRequest> supportRequests_;
    std::vector<Approval> approvals_;
};

} // namespace domain
