#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <algorithm>

#include "domain/User.hpp"
#include "domain/Facility.hpp"
#include "domain/Reservation.hpp"
#include "domain/NotificationService.hpp"
#include "state/ReservationStateMachine.hpp"
#include "state/FacilityStateMachine.hpp"

namespace service {

/**
 * @brief Thread-safe Central Booking System Coordinator and in-memory repository.
 * Coordinates users, facilities, reservations, simulated clock, and state machine transitions.
 */
class BookingSystem {
public:
    /**
     * @brief Helper to construct a local time_t for deterministic simulation dates.
     */
    static std::time_t makeTime(int year, int month, int day, int hour, int minute, int second = 0) {
        std::tm tm{};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1;
        return std::mktime(&tm);
    }

    /**
     * @brief Formats a time_t value into a readable string (YYYY-MM-DD HH:MM:SS).
     */
    static std::string formatTime(std::time_t t) {
        if (t == 0) return "N/A";
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }

    /**
     * @brief Constructor initializing the system with default sample data and simulated clock.
     */
    explicit BookingSystem(std::time_t initialTime = 0) {
        reset(initialTime);
    }

    ~BookingSystem() = default;
    BookingSystem(const BookingSystem&) = delete;
    BookingSystem& operator=(const BookingSystem&) = delete;

    /**
     * @brief Resets the system repository and seeds default facilities and users.
     */
    void reset(std::time_t initialTime = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        facilities_.clear();
        facilityOrder_.clear();
        reservations_.clear();
        reservationOrder_.clear();
        users_.clear();
        userOrder_.clear();
        auditLog_.clear();

        currentTime_ = (initialTime != 0) ? initialTime : makeTime(2026, 9, 1, 9, 0, 0);
        seedDefaultDataLocked();
    }

    // -------------------------------------------------------------------------
    // Simulated Clock Operations
    // -------------------------------------------------------------------------

    std::time_t getCurrentTime() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return currentTime_;
    }

    void setTime(std::time_t t) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentTime_ = t;
        checkExpirationsLocked();
    }

    void advanceTime(std::time_t seconds) {
        std::lock_guard<std::mutex> lock(mutex_);
        currentTime_ += seconds;
        checkExpirationsLocked();
    }

    // -------------------------------------------------------------------------
    // Repository Management (Users & Facilities)
    // -------------------------------------------------------------------------

    void addFacility(domain::Facility facility) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string id = facility.getFacilityId();
        if (facilities_.find(id) == facilities_.end()) {
            facilityOrder_.push_back(id);
        }
        facilities_.insert_or_assign(id, std::move(facility));
    }

    void addUser(std::shared_ptr<domain::User> user) {
        if (!user) return;
        std::lock_guard<std::mutex> lock(mutex_);
        std::string id = user->getUserId();
        if (users_.find(id) == users_.end()) {
            userOrder_.push_back(id);
        }
        users_.insert_or_assign(id, std::move(user));
    }

    // -------------------------------------------------------------------------
    // Reservation Operations (Coordinated with State Machines)
    // -------------------------------------------------------------------------

    /**
     * @brief Creates a new reservation in Draft status.
     * @return Generated or assigned reservation ID, or empty string on validation failure.
     */
    std::string createReservation(
        const std::string& requesterId,
        const std::string& facilityId,
        const std::string& eventTitle,
        const std::string& eventType,
        int expectedAttendance,
        std::time_t startTime,
        std::time_t endTime,
        const std::vector<domain::EquipmentRequest>& equipment = {},
        const std::vector<domain::SupportRequest>& support = {},
        const std::string& customReservationId = ""
    ) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto facIt = facilities_.find(facilityId);
        if (facIt == facilities_.end()) {
            addAuditLogLocked("Reservation creation failed: Facility " + facilityId + " not found.");
            return "";
        }

        if (startTime >= endTime) {
            addAuditLogLocked("Reservation creation failed: startTime must precede endTime.");
            return "";
        }

        std::string resId = customReservationId;
        if (resId.empty()) {
            resId = "RES-" + std::to_string(++reservationCounter_);
        }

        domain::Reservation res(
            resId,
            requesterId,
            facilityId,
            eventTitle,
            eventType,
            expectedAttendance,
            startTime,
            endTime,
            currentTime_,
            domain::ReservationStatus::Draft
        );

        for (const auto& eq : equipment) {
            res.addEquipment(eq);
        }
        for (const auto& sup : support) {
            res.addSupportRequest(sup);
        }

        reservations_.insert_or_assign(resId, std::move(res));
        reservationOrder_.push_back(resId);

        addAuditLogLocked("Created reservation " + resId + " [Draft] for \"" + eventTitle + "\" in " + facilityId + ".");
        return resId;
    }

    std::string createReservation(domain::Reservation reservation) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string resId = reservation.getReservationId();
        if (resId.empty()) {
            resId = "RES-" + std::to_string(++reservationCounter_);
            reservation.setReservationId(resId);
        }
        reservation.setStatus(domain::ReservationStatus::Draft);
        if (reservation.getCreatedAt() == 0) {
            reservation.setCreatedAt(currentTime_);
        }

        reservations_.insert_or_assign(resId, std::move(reservation));
        if (std::find(reservationOrder_.begin(), reservationOrder_.end(), resId) == reservationOrder_.end()) {
            reservationOrder_.push_back(resId);
        }

        addAuditLogLocked("Created reservation " + resId + " [Draft].");
        return resId;
    }

    /**
     * @brief Submits a reservation from Draft to Submitted, applying a 15-minute soft hold.
     */
    state::TransitionResult submitReservation(const std::string& reservationId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        bool hasConflict = hasConflictLocked(resIt->second);
        state::TransitionResult res = reservationStateMachine_.submit(resIt->second, !hasConflict, currentTime_);
        if (res.success) {
            addAuditLogLocked("Submitted reservation " + reservationId + ": 15-minute soft lock active until " + formatTime(resIt->second.getHoldExpiresAt()) + ".");
        } else {
            addAuditLogLocked("Failed to submit reservation " + reservationId + ": " + res.message);
        }
        return res;
    }

    /**
     * @brief Routes submitted reservation to event approver queue (UnderReview).
     */
    state::TransitionResult routeToReview(const std::string& reservationId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.routeToReview(resIt->second, currentTime_);
        if (res.success) {
            addAuditLogLocked("Reservation " + reservationId + " routed to Event Approver queue [UnderReview].");
        }
        return res;
    }

    /**
     * @brief Evaluates whether high-risk gate applies (headcount > 100 or after-hours).
     * If true, auto-transitions UnderReview -> SecurityReview.
     */
    state::TransitionResult evaluateSecurity(const std::string& reservationId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.evaluateSecurity(resIt->second);
        if (res.success && res.newStatus == domain::ReservationStatus::SecurityReview) {
            addAuditLogLocked("Reservation " + reservationId + " escalated to SecurityReview (headcount: " +
                              std::to_string(resIt->second.getExpectedAttendance()) +
                              ", after-hours: " + (resIt->second.isAfterHours() ? "yes" : "no") + ").");
        }
        return res;
    }

    /**
     * @brief Approves reservation from UnderReview and marks facility Reserved.
     */
    state::TransitionResult approveReservation(const std::string& reservationId, const std::string& approverId, const std::string& comments = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.approve(resIt->second, approverId, comments, currentTime_);
        if (res.success) {
            auto facIt = facilities_.find(resIt->second.getFacilityId());
            if (facIt != facilities_.end() && facIt->second.getStatus() == domain::RoomStatus::Available) {
                facilityStateMachine_.reservationConfirmed(facIt->second);
            }
            addAuditLogLocked("Reservation " + reservationId + " approved by " + approverId + ". Facility calendar slot confirmed.");
        }
        return res;
    }

    /**
     * @brief Security officer clearance: clears high-risk reservation from SecurityReview to Approved.
     */
    state::TransitionResult securityClear(const std::string& reservationId, const std::string& officerId, const std::string& notes = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.securityClear(resIt->second, officerId, notes, currentTime_);
        if (res.success) {
            auto facIt = facilities_.find(resIt->second.getFacilityId());
            if (facIt != facilities_.end() && facIt->second.getStatus() == domain::RoomStatus::Available) {
                facilityStateMachine_.reservationConfirmed(facIt->second);
            }
            addAuditLogLocked("Reservation " + reservationId + " cleared by Security Officer " + officerId + ": " + notes);
        }
        return res;
    }

    /**
     * @brief Security officer denial: denies high-risk reservation to Rejected.
     */
    state::TransitionResult securityDeny(const std::string& reservationId, const std::string& officerId, const std::string& reason = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.securityDeny(resIt->second, officerId, reason, currentTime_);
        if (res.success) {
            addAuditLogLocked("Reservation " + reservationId + " denied by Security Officer " + officerId + ": " + reason);
        }
        return res;
    }

    /**
     * @brief Approver requests modifications, transitioning UnderReview -> NeedsRevision with a deadline.
     */
    state::TransitionResult requestChanges(const std::string& reservationId, const std::string& approverId, const std::string& notes, std::time_t deadline) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.requestRevision(resIt->second, approverId, notes, deadline, currentTime_);
        if (res.success) {
            addAuditLogLocked("Revision requested for " + reservationId + " by " + approverId + ". Deadline: " + formatTime(deadline));
        }
        return res;
    }

    /**
     * @brief Resubmits a modified reservation from NeedsRevision back to Submitted.
     */
    state::TransitionResult resubmitReservation(const std::string& reservationId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        bool hasConflict = hasConflictLocked(resIt->second);
        state::TransitionResult res = reservationStateMachine_.resubmit(resIt->second, !hasConflict, currentTime_);
        if (res.success) {
            addAuditLogLocked("Reservation " + reservationId + " resubmitted. 15-minute soft lock refreshed until " + formatTime(resIt->second.getHoldExpiresAt()) + ".");
        }
        return res;
    }

    /**
     * @brief Rejects a reservation with approver justification.
     */
    state::TransitionResult rejectReservation(const std::string& reservationId, const std::string& approverId, const std::string& reason) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.reject(resIt->second, approverId, reason, currentTime_);
        if (res.success) {
            addAuditLogLocked("Reservation " + reservationId + " rejected by " + approverId + ": " + reason);
        }
        return res;
    }

    /**
     * @brief Checks into facility when event start time arrives: InProgress, facility Occupied, smart doors unlocked.
     */
    state::TransitionResult checkInReservation(const std::string& reservationId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        auto facIt = facilities_.find(resIt->second.getFacilityId());
        if (facIt == facilities_.end()) {
            return {false, "Associated facility not found: " + resIt->second.getFacilityId(), domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.checkIn(resIt->second, facIt->second, currentTime_);
        if (res.success) {
            addAuditLogLocked("Checked in reservation " + reservationId + ". Venue " + facIt->second.getFacilityId() + " Occupied; smart doors unlocked.");
        }
        return res;
    }

    /**
     * @brief Concludes event: Completed, facility in Turnover, smart doors locked, custodial turnover triggered.
     */
    state::TransitionResult concludeReservation(const std::string& reservationId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        auto facIt = facilities_.find(resIt->second.getFacilityId());
        if (facIt == facilities_.end()) {
            return {false, "Associated facility not found: " + resIt->second.getFacilityId(), domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        state::TransitionResult res = reservationStateMachine_.conclude(resIt->second, facIt->second, currentTime_);
        if (res.success) {
            addAuditLogLocked("Concluded reservation " + reservationId + ". Venue " + facIt->second.getFacilityId() + " in Turnover; smart doors locked; custodial notice sent.");
        }
        return res;
    }

    /**
     * @brief Cancels reservation. If approved, enforces strict 24-hour lead time and releases facility lock.
     */
    state::TransitionResult cancelReservation(const std::string& reservationId, const std::string& userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto resIt = reservations_.find(reservationId);
        if (resIt == reservations_.end()) {
            return {false, "Reservation not found: " + reservationId, domain::ReservationStatus::Draft, domain::ReservationStatus::Draft};
        }

        domain::ReservationStatus prevStatus = resIt->second.getStatus();
        state::TransitionResult res = reservationStateMachine_.cancel(resIt->second, userId, currentTime_);
        if (res.success) {
            auto facIt = facilities_.find(resIt->second.getFacilityId());
            if (facIt != facilities_.end()) {
                if (facIt->second.getStatus() == domain::RoomStatus::Reserved || facIt->second.getStatus() == domain::RoomStatus::Setup) {
                    facIt->second.setStatus(domain::RoomStatus::Available);
                    facIt->second.setDoorsLocked(false);
                }
            }
            addAuditLogLocked("Reservation " + reservationId + " cancelled by " + userId + ". Facility lock released.");
        }
        return res;
    }

    // -------------------------------------------------------------------------
    // Facility Coordination & Emergency Operations
    // -------------------------------------------------------------------------

    /**
     * @brief Declares a campus-wide emergency lockdown: all facilities locked in EmergencyHold.
     */
    state::FacilityTransitionResult declareCampusEmergency(const std::string& reason) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [id, fac] : facilities_) {
            facilityStateMachine_.declareEmergency(fac, reason);
        }
        addAuditLogLocked("CAMPUS EMERGENCY DECLARED: " + reason + ". All facilities locked in EmergencyHold.");
        return {true, "Campus emergency declared: " + reason, domain::RoomStatus::Available, domain::RoomStatus::EmergencyHold};
    }

    /**
     * @brief Restores all facilities under EmergencyHold back to Available state.
     */
    state::FacilityTransitionResult clearCampusEmergency() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [id, fac] : facilities_) {
            if (fac.getStatus() == domain::RoomStatus::EmergencyHold) {
                facilityStateMachine_.allClearGiven(fac, true);
            }
        }
        addAuditLogLocked("Campus emergency cleared. All facilities restored to Available.");
        return {true, "Campus emergency lifted: facilities restored", domain::RoomStatus::EmergencyHold, domain::RoomStatus::Available};
    }

    /**
     * @brief Facility logistics: opens setup staging window (Reserved -> Setup).
     */
    state::FacilityTransitionResult openSetupWindow(const std::string& facilityId) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = facilities_.find(facilityId);
        if (it == facilities_.end()) {
            return {false, "Facility not found: " + facilityId, domain::RoomStatus::Available, domain::RoomStatus::Available};
        }
        state::FacilityTransitionResult res = facilityStateMachine_.setupWindowOpens(it->second);
        if (res.success) {
            addAuditLogLocked("Setup staging window opened for " + facilityId + " [Setup].");
        }
        return res;
    }

    /**
     * @brief Facility logistics: turnover inspection passed (Turnover -> Available).
     */
    state::FacilityTransitionResult completeInspection(const std::string& facilityId, bool readyForUse = true) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = facilities_.find(facilityId);
        if (it == facilities_.end()) {
            return {false, "Facility not found: " + facilityId, domain::RoomStatus::Available, domain::RoomStatus::Available};
        }
        state::FacilityTransitionResult res = facilityStateMachine_.inspectionPassed(it->second, readyForUse);
        if (res.success) {
            addAuditLogLocked("Custodial turnover inspection passed for " + facilityId + " [Available].");
        }
        return res;
    }

    /**
     * @brief Reports damage or maintenance blackout on a facility.
     */
    state::FacilityTransitionResult reportFacilityDamage(const std::string& facilityId, const std::string& issue) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = facilities_.find(facilityId);
        if (it == facilities_.end()) {
            return {false, "Facility not found: " + facilityId, domain::RoomStatus::Available, domain::RoomStatus::Available};
        }
        state::FacilityTransitionResult res = facilityStateMachine_.reportDamage(it->second, issue);
        if (res.success) {
            addAuditLogLocked("Damage reported for " + facilityId + ": " + issue + " [OutOfService].");
        }
        return res;
    }

    /**
     * @brief Certifies maintenance repairs on an OutOfService facility.
     */
    state::FacilityTransitionResult certifyFacilityRepair(const std::string& facilityId, bool safetyPass = true) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = facilities_.find(facilityId);
        if (it == facilities_.end()) {
            return {false, "Facility not found: " + facilityId, domain::RoomStatus::Available, domain::RoomStatus::Available};
        }
        state::FacilityTransitionResult res = facilityStateMachine_.repairCertified(it->second, safetyPass);
        if (res.success) {
            addAuditLogLocked("Repairs certified for " + facilityId + " [Available].");
        }
        return res;
    }

    // -------------------------------------------------------------------------
    // Conflict Detection
    // -------------------------------------------------------------------------

    bool hasConflict(const domain::Reservation& res) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return hasConflictLocked(res);
    }

    // -------------------------------------------------------------------------
    // Accessors
    // -------------------------------------------------------------------------

    std::vector<domain::Facility> getFacilities() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<domain::Facility> list;
        list.reserve(facilityOrder_.size());
        for (const auto& id : facilityOrder_) {
            auto it = facilities_.find(id);
            if (it != facilities_.end()) {
                list.push_back(it->second);
            }
        }
        return list;
    }

    std::vector<domain::Reservation> getReservations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<domain::Reservation> list;
        list.reserve(reservationOrder_.size());
        for (const auto& id : reservationOrder_) {
            auto it = reservations_.find(id);
            if (it != reservations_.end()) {
                list.push_back(it->second);
            }
        }
        return list;
    }

    std::optional<domain::Facility> getFacility(const std::string& id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = facilities_.find(id);
        if (it != facilities_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<domain::Reservation> getReservation(const std::string& id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = reservations_.find(id);
        if (it != reservations_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::vector<domain::Reservation> getPendingApprovals() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<domain::Reservation> list;
        for (const auto& id : reservationOrder_) {
            auto it = reservations_.find(id);
            if (it != reservations_.end() && it->second.getStatus() == domain::ReservationStatus::UnderReview) {
                list.push_back(it->second);
            }
        }
        return list;
    }

    std::vector<domain::Reservation> getPendingSecurity() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<domain::Reservation> list;
        for (const auto& id : reservationOrder_) {
            auto it = reservations_.find(id);
            if (it != reservations_.end() && it->second.getStatus() == domain::ReservationStatus::SecurityReview) {
                list.push_back(it->second);
            }
        }
        return list;
    }

    std::vector<std::string> getAuditLog() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return auditLog_;
    }

    std::vector<std::shared_ptr<domain::User>> getUsers() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::shared_ptr<domain::User>> list;
        list.reserve(userOrder_.size());
        for (const auto& id : userOrder_) {
            auto it = users_.find(id);
            if (it != users_.end()) {
                list.push_back(it->second);
            }
        }
        return list;
    }

    std::shared_ptr<domain::User> getUser(const std::string& id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = users_.find(id);
        if (it != users_.end()) {
            return it->second;
        }
        return nullptr;
    }

    // -------------------------------------------------------------------------
    // JSON Serialization for Web Dashboard / REST API
    // -------------------------------------------------------------------------

    std::string toJson() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ostringstream ss;
        ss << "{\n";
        ss << "  \"currentTime\": " << currentTime_ << ",\n";
        ss << "  \"formattedTime\": \"" << formatTime(currentTime_) << "\",\n";

        // Facilities
        ss << "  \"facilities\": [\n";
        for (size_t i = 0; i < facilityOrder_.size(); ++i) {
            const auto& fac = facilities_.at(facilityOrder_[i]);
            ss << "    {\n";
            ss << "      \"id\": \"" << fac.getFacilityId() << "\",\n";
            ss << "      \"name\": \"" << escapeJson(fac.getName()) << "\",\n";
            ss << "      \"building\": \"" << escapeJson(fac.getBuilding()) << "\",\n";
            ss << "      \"capacity\": " << fac.getCapacity() << ",\n";
            ss << "      \"hourlyRate\": " << fac.getHourlyRate() << ",\n";
            ss << "      \"status\": \"" << domain::roomStatusToString(fac.getStatus()) << "\",\n";
            ss << "      \"doorsLocked\": " << (fac.areDoorsLocked() ? "true" : "false") << ",\n";
            ss << "      \"equipment\": [";
            const auto& eq = fac.getSupportedEquipment();
            for (size_t j = 0; j < eq.size(); ++j) {
                ss << "\"" << escapeJson(eq[j]) << "\"" << (j + 1 < eq.size() ? ", " : "");
            }
            ss << "]\n";
            ss << "    }" << (i + 1 < facilityOrder_.size() ? "," : "") << "\n";
        }
        ss << "  ],\n";

        // Reservations
        ss << "  \"reservations\": [\n";
        for (size_t i = 0; i < reservationOrder_.size(); ++i) {
            const auto& res = reservations_.at(reservationOrder_[i]);
            ss << "    {\n";
            ss << "      \"id\": \"" << res.getReservationId() << "\",\n";
            ss << "      \"requesterId\": \"" << res.getRequesterId() << "\",\n";
            ss << "      \"facilityId\": \"" << res.getFacilityId() << "\",\n";
            ss << "      \"title\": \"" << escapeJson(res.getEventTitle()) << "\",\n";
            ss << "      \"eventType\": \"" << escapeJson(res.getEventType()) << "\",\n";
            ss << "      \"attendance\": " << res.getExpectedAttendance() << ",\n";
            ss << "      \"startTime\": " << res.getStartTime() << ",\n";
            ss << "      \"endTime\": " << res.getEndTime() << ",\n";
            ss << "      \"formattedStart\": \"" << formatTime(res.getStartTime()) << "\",\n";
            ss << "      \"formattedEnd\": \"" << formatTime(res.getEndTime()) << "\",\n";
            ss << "      \"status\": \"" << domain::reservationStatusToString(res.getStatus()) << "\",\n";
            ss << "      \"requiresSecurity\": " << (res.getRequiresSecurity() ? "true" : "false") << ",\n";
            ss << "      \"holdExpiresAt\": " << res.getHoldExpiresAt() << ",\n";
            ss << "      \"revisionDeadline\": " << res.getRevisionDeadline() << "\n";
            ss << "    }" << (i + 1 < reservationOrder_.size() ? "," : "") << "\n";
        }
        ss << "  ],\n";

        // Users
        ss << "  \"users\": [\n";
        for (size_t i = 0; i < userOrder_.size(); ++i) {
            const auto& user = users_.at(userOrder_[i]);
            ss << "    {\n";
            ss << "      \"id\": \"" << user->getUserId() << "\",\n";
            ss << "      \"name\": \"" << escapeJson(user->getFullName()) << "\",\n";
            ss << "      \"email\": \"" << escapeJson(user->getEmail()) << "\",\n";
            ss << "      \"department\": \"" << escapeJson(user->getDepartment()) << "\",\n";
            ss << "      \"role\": \"" << domain::userRoleToString(user->getRole()) << "\",\n";
            ss << "      \"roleName\": \"" << escapeJson(user->getRoleName()) << "\"\n";
            ss << "    }" << (i + 1 < userOrder_.size() ? "," : "") << "\n";
        }
        ss << "  ],\n";

        // Audit Log
        ss << "  \"auditLog\": [\n";
        for (size_t i = 0; i < auditLog_.size(); ++i) {
            ss << "    \"" << escapeJson(auditLog_[i]) << "\"" << (i + 1 < auditLog_.size() ? "," : "") << "\n";
        }
        ss << "  ]\n";
        ss << "}\n";
        return ss.str();
    }

private:
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
            else out += c;
        }
        return out;
    }

    void addAuditLogLocked(const std::string& entry) {
        std::string stamped = "[" + formatTime(currentTime_) + "] " + entry;
        auditLog_.push_back(std::move(stamped));
    }

    bool hasConflictLocked(const domain::Reservation& res) const {
        auto facIt = facilities_.find(res.getFacilityId());
        if (facIt != facilities_.end()) {
            if (!facIt->second.checkAvailability(res.getStartTime(), res.getEndTime())) {
                return true;
            }
        }

        for (const auto& [id, other] : reservations_) {
            if (id == res.getReservationId()) continue;
            if (other.getFacilityId() != res.getFacilityId()) continue;

            auto s = other.getStatus();
            bool active = (s == domain::ReservationStatus::Submitted ||
                           s == domain::ReservationStatus::UnderReview ||
                           s == domain::ReservationStatus::SecurityReview ||
                           s == domain::ReservationStatus::Approved ||
                           s == domain::ReservationStatus::InProgress);
            if (active) {
                if (res.getStartTime() < other.getEndTime() && other.getStartTime() < res.getEndTime()) {
                    return true;
                }
            }
        }
        return false;
    }

    void checkExpirationsLocked() {
        for (auto& [id, res] : reservations_) {
            if (res.getStatus() == domain::ReservationStatus::Submitted && res.getHoldExpiresAt() > 0) {
                if (currentTime_ > res.getHoldExpiresAt()) {
                    reservationStateMachine_.expire(res, currentTime_);
                    addAuditLogLocked("Reservation " + id + " soft hold expired [Expired].");
                }
            } else if (res.getStatus() == domain::ReservationStatus::NeedsRevision && res.getRevisionDeadline() > 0) {
                if (currentTime_ > res.getRevisionDeadline()) {
                    reservationStateMachine_.expire(res, currentTime_);
                    addAuditLogLocked("Reservation " + id + " revision deadline passed [Expired].");
                }
            }
        }
    }

    void seedDefaultDataLocked() {
        // 4 Facilities
        facilities_.emplace("FAC-101", domain::Facility(
            "FAC-101", "Centennial Auditorium", "Centennial Hall", 250, 150.0,
            domain::RoomStatus::Available,
            {"Stage", "AV System", "Projector", "Wireless Microphones", "Stage Lighting"}
        ));
        facilityOrder_.push_back("FAC-101");

        facilities_.emplace("FAC-102", domain::Facility(
            "FAC-102", "STEM Lab", "Science Complex", 40, 60.0,
            domain::RoomStatus::Available,
            {"Workstations", "3D Printers", "Smart Board", "Oscilloscopes"}
        ));
        facilityOrder_.push_back("FAC-102");

        facilities_.emplace("FAC-103", domain::Facility(
            "FAC-103", "Student Union Ballroom", "Student Union", 400, 250.0,
            domain::RoomStatus::Available,
            {"Stage", "Sound System", "Banquet Tables", "Podium", "Projector"}
        ));
        facilityOrder_.push_back("FAC-103");

        facilities_.emplace("FAC-104", domain::Facility(
            "FAC-104", "Seminar Room", "Academic Center", 25, 30.0,
            domain::RoomStatus::Available,
            {"Whiteboard", "Video Conference System", "Podium"}
        ));
        facilityOrder_.push_back("FAC-104");

        // 6 Users representing all roles (+ 1 admin)
        users_.emplace("USR-001", std::make_shared<domain::StudentOrgRepresentative>(
            "USR-001", "Alice Chen", "alice@studentorg.campus.edu", "Engineering", "Robotics Club", "pass123"
        ));
        userOrder_.push_back("USR-001");

        users_.emplace("USR-002", std::make_shared<domain::StudentOrgRepresentative>(
            "USR-002", "Bob Smith", "bob@studentorg.campus.edu", "Business", "Debate Society", "pass123"
        ));
        userOrder_.push_back("USR-002");

        users_.emplace("USR-003", std::make_shared<domain::FacultyStaffMember>(
            "USR-003", "Dr. Clara Oswald", "clara@faculty.campus.edu", "Computer Science", "FAC-EMP-882", "pass123"
        ));
        userOrder_.push_back("USR-003");

        users_.emplace("USR-004", std::make_shared<domain::FacilityCoordinator>(
            "USR-004", "Dan Miller", "dan@facilities.campus.edu", "Campus Operations",
            std::vector<std::string>{"Centennial Hall", "Science Complex", "Student Union", "Academic Center"},
            "pass123"
        ));
        userOrder_.push_back("USR-004");

        users_.emplace("USR-005", std::make_shared<domain::EventApprover>(
            "USR-005", "Dean Evelyn Ross", "evelyn@administration.campus.edu", "Student Affairs", 1, "pass123"
        ));
        userOrder_.push_back("USR-005");

        users_.emplace("USR-006", std::make_shared<domain::SecurityOfficer>(
            "USR-006", "Officer Frank Castle", "frank@security.campus.edu", "Campus Police", "BADGE-404", "pass123"
        ));
        userOrder_.push_back("USR-006");

        users_.emplace("USR-007", std::make_shared<domain::Administrator>(
            "USR-007", "Grace Hopper", "admin@campus.edu", "IT Administration", "admin123"
        ));
        userOrder_.push_back("USR-007");

        addAuditLogLocked("System initialized with 4 venues and 7 user accounts.");
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::string, domain::Facility> facilities_;
    std::vector<std::string> facilityOrder_;
    std::unordered_map<std::string, domain::Reservation> reservations_;
    std::vector<std::string> reservationOrder_;
    std::unordered_map<std::string, std::shared_ptr<domain::User>> users_;
    std::vector<std::string> userOrder_;
    std::vector<std::string> auditLog_;
    std::time_t currentTime_{0};
    std::atomic<uint64_t> reservationCounter_{100};

    state::ReservationStateMachine reservationStateMachine_;
    state::FacilityStateMachine facilityStateMachine_;
};

} // namespace service
