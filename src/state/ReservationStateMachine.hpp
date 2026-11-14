#pragma once

#include <string>
#include <ctime>
#include <utility>
#include "domain/Reservation.hpp"
#include "domain/Facility.hpp"
#include "domain/NotificationService.hpp"

namespace state {

/**
 * @brief Encapsulates the outcome of a reservation state transition.
 */
struct TransitionResult {
    bool success{false};
    std::string message;
    domain::ReservationStatus previousStatus{domain::ReservationStatus::Draft};
    domain::ReservationStatus newStatus{domain::ReservationStatus::Draft};

    TransitionResult() = default;
    TransitionResult(bool s, std::string msg, domain::ReservationStatus prev, domain::ReservationStatus next)
        : success(s), message(std::move(msg)), previousStatus(prev), newStatus(next) {}
};

/**
 * @brief State Machine Engine governing the 11 formal states of the Reservation Lifecycle.
 */
class ReservationStateMachine {
public:
    ReservationStateMachine() = default;

    /**
     * @brief Checks structural validity of a transition between two states.
     */
    bool canTransition(domain::ReservationStatus from, domain::ReservationStatus to) const {
        if (from == to) {
            return false;
        }

        switch (from) {
            case domain::ReservationStatus::Draft:
                return (to == domain::ReservationStatus::Submitted ||
                        to == domain::ReservationStatus::Cancelled ||
                        to == domain::ReservationStatus::Rejected);

            case domain::ReservationStatus::Submitted:
                return (to == domain::ReservationStatus::UnderReview ||
                        to == domain::ReservationStatus::Cancelled ||
                        to == domain::ReservationStatus::Expired ||
                        to == domain::ReservationStatus::Rejected);

            case domain::ReservationStatus::UnderReview:
                return (to == domain::ReservationStatus::SecurityReview ||
                        to == domain::ReservationStatus::Approved ||
                        to == domain::ReservationStatus::NeedsRevision ||
                        to == domain::ReservationStatus::Rejected ||
                        to == domain::ReservationStatus::Cancelled);

            case domain::ReservationStatus::SecurityReview:
                return (to == domain::ReservationStatus::Approved ||
                        to == domain::ReservationStatus::Rejected ||
                        to == domain::ReservationStatus::Cancelled);

            case domain::ReservationStatus::NeedsRevision:
                return (to == domain::ReservationStatus::Submitted ||
                        to == domain::ReservationStatus::Expired ||
                        to == domain::ReservationStatus::Cancelled ||
                        to == domain::ReservationStatus::Rejected);

            case domain::ReservationStatus::Approved:
                return (to == domain::ReservationStatus::InProgress ||
                        to == domain::ReservationStatus::Cancelled);

            case domain::ReservationStatus::InProgress:
                return (to == domain::ReservationStatus::Completed);

            case domain::ReservationStatus::Expired:
            case domain::ReservationStatus::Rejected:
            case domain::ReservationStatus::Completed:
            case domain::ReservationStatus::Cancelled:
                // Terminal states
                return false;
        }
        return false;
    }

    /**
     * @brief Submits a draft reservation, enforcing time conflict guards and applying a 15-minute soft hold.
     */
    TransitionResult submit(domain::Reservation& reservation, bool noTimeConflicts, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::Draft) {
            return {false, "Cannot submit reservation: must be in Draft state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (!noTimeConflicts) {
            return {false, "Cannot submit reservation: schedule conflict detected", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Submitted);
        reservation.setHoldExpiresAt(currentTime + 900); // 15-minute soft hold

        return {true, "Reservation submitted successfully; 15-minute soft lock placed", current, domain::ReservationStatus::Submitted};
    }

    /**
     * @brief Routes submitted reservation to event approver queue if soft lock has not expired.
     */
    TransitionResult routeToReview(domain::Reservation& reservation, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::Submitted) {
            return {false, "Cannot route to review: reservation must be in Submitted state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (currentTime > reservation.getHoldExpiresAt()) {
            return {false, "Cannot route to review: 15-minute soft hold has expired", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::UnderReview);
        return {true, "Reservation routed to event approver review queue", current, domain::ReservationStatus::UnderReview};
    }

    /**
     * @brief Evaluates whether high-risk gate applies (attendance > 100 or after-hours).
     * If true, transitions UnderReview -> SecurityReview.
     */
    TransitionResult evaluateSecurity(domain::Reservation& reservation) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::UnderReview) {
            return {false, "Security evaluation requires UnderReview state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        bool requiresSecurity = reservation.evaluateSecurityRequirement();
        if (requiresSecurity) {
            reservation.setStatus(domain::ReservationStatus::SecurityReview);
            domain::NotificationService::getInstance().sendSecurityAlert(
                reservation.getReservationId(),
                "High-risk booking requires security assignment (headcount: " +
                std::to_string(reservation.getExpectedAttendance()) +
                ", after-hours: " + (reservation.isAfterHours() ? "yes" : "no") + ")"
            );
            return {true, "High-risk criteria met: transitioned to SecurityReview", current, domain::ReservationStatus::SecurityReview};
        }

        return {true, "Standard risk profile: remains in UnderReview", current, current};
    }

    /**
     * @brief Approves reservation from UnderReview if security clearance is NOT required.
     */
    TransitionResult approve(domain::Reservation& reservation, const std::string& approverId, const std::string& comments, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::UnderReview) {
            return {false, "Standard approval requires UnderReview state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (reservation.getRequiresSecurity() || reservation.evaluateSecurityRequirement()) {
            return {false, "Direct approval blocked: high-risk event requires SecurityReview clearance", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Approved);
        reservation.recordApproval(approverId, "Approved", comments, currentTime);
        domain::NotificationService::getInstance().sendConfirmation(reservation.getRequesterId(), reservation.getReservationId(), comments);

        return {true, "Reservation formally approved; calendar slot locked permanently", current, domain::ReservationStatus::Approved};
    }

    /**
     * @brief Clears high-risk reservation from SecurityReview, assigning security officer and approving.
     */
    TransitionResult securityClear(domain::Reservation& reservation, const std::string& officerId, const std::string& notes, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::SecurityReview) {
            return {false, "Security clearance requires SecurityReview state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        // Assign officer to security support request or add new assigned request
        bool foundRequest = false;
        for (auto& req : reservation.getSupportRequests()) {
            if (req.getSupportType() == "Security") {
                req.assignOfficer(officerId);
                req.setNotes(notes);
                foundRequest = true;
                break;
            }
        }
        if (!foundRequest) {
            reservation.addSupportRequest(domain::SupportRequest(
                "sec-" + reservation.getReservationId(),
                "Security",
                notes,
                true,
                officerId
            ));
        }

        reservation.setStatus(domain::ReservationStatus::Approved);
        reservation.recordApproval(officerId, "Security Cleared", notes, currentTime);
        domain::NotificationService::getInstance().sendConfirmation(
            reservation.getRequesterId(),
            reservation.getReservationId(),
            "Security cleared by Officer " + officerId + ": " + notes
        );

        return {true, "Security cleared and officer assigned: reservation approved", current, domain::ReservationStatus::Approved};
    }

    /**
     * @brief Denies high-risk reservation from SecurityReview, transitioning to Rejected.
     */
    TransitionResult securityDeny(domain::Reservation& reservation, const std::string& officerId, const std::string& reason, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::SecurityReview) {
            return {false, "Security denial requires SecurityReview state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Rejected);
        reservation.recordApproval(officerId, "Security Denied", reason, currentTime);
        domain::NotificationService::getInstance().sendSecurityAlert(
            reservation.getReservationId(),
            "Security clearance denied by Officer " + officerId + ": " + reason
        );

        return {true, "Security clearance denied: reservation rejected", current, domain::ReservationStatus::Rejected};
    }

    /**
     * @brief Requests requester modifications, transitioning UnderReview -> NeedsRevision with a deadline.
     */
    TransitionResult requestRevision(domain::Reservation& reservation, const std::string& approverId, const std::string& feedback, std::time_t deadline, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::UnderReview) {
            return {false, "Revision request requires UnderReview state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (deadline <= currentTime) {
            return {false, "Revision deadline must be in the future", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::NeedsRevision);
        reservation.setRevisionDeadline(deadline);
        reservation.recordApproval(approverId, "Revision Requested", feedback, currentTime);
        domain::NotificationService::getInstance().sendNotification(
            reservation.getRequesterId(),
            "Action Required: Revision Requested for " + reservation.getReservationId(),
            feedback
        );

        return {true, "Revision requested: transitioned to NeedsRevision", current, domain::ReservationStatus::NeedsRevision};
    }

    /**
     * @brief Resubmits a revised reservation if within the revision deadline and no conflicts exist.
     */
    TransitionResult resubmit(domain::Reservation& reservation, bool noConflicts, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::NeedsRevision) {
            return {false, "Resubmission requires NeedsRevision state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (currentTime > reservation.getRevisionDeadline()) {
            return {false, "Cannot resubmit: revision deadline has expired", current, current};
        }

        if (!noConflicts) {
            return {false, "Cannot resubmit: schedule conflict detected", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Submitted);
        reservation.setHoldExpiresAt(currentTime + 900); // Refresh 15-minute soft lock

        return {true, "Reservation resubmitted with refreshed 15-minute soft hold", current, domain::ReservationStatus::Submitted};
    }

    /**
     * @brief Expires a reservation when revision deadline or soft lock buffer has elapsed.
     */
    TransitionResult expire(domain::Reservation& reservation, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current == domain::ReservationStatus::NeedsRevision) {
            if (currentTime <= reservation.getRevisionDeadline()) {
                return {false, "Cannot expire: revision deadline has not passed", current, current};
            }
        } else if (current == domain::ReservationStatus::Submitted) {
            if (currentTime <= reservation.getHoldExpiresAt()) {
                return {false, "Cannot expire: soft hold buffer has not passed", current, current};
            }
        } else {
            return {false, "Cannot expire reservation from status: " + domain::reservationStatusToString(current), current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Expired);
        return {true, "Reservation expired due to elapsed deadline; hold released", current, domain::ReservationStatus::Expired};
    }

    /**
     * @brief Rejects a reservation with approver justification.
     */
    TransitionResult reject(domain::Reservation& reservation, const std::string& approverId, const std::string& reason, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (!canTransition(current, domain::ReservationStatus::Rejected)) {
            return {false, "Cannot reject reservation in status: " + domain::reservationStatusToString(current), current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Rejected);
        reservation.recordApproval(approverId, "Rejected", reason, currentTime);
        domain::NotificationService::getInstance().sendNotification(
            reservation.getRequesterId(),
            "Reservation Rejected: " + reservation.getReservationId(),
            reason
        );

        return {true, "Reservation rejected: " + reason, current, domain::ReservationStatus::Rejected};
    }

    /**
     * @brief Checks into facility when event start time arrives, moving to InProgress and unlocking smart doors.
     */
    TransitionResult checkIn(domain::Reservation& reservation, domain::Facility& facility, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::Approved) {
            return {false, "Check-in requires Approved state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (facility.getFacilityId() != reservation.getFacilityId()) {
            return {false, "Facility ID mismatch", current, current};
        }

        if (facility.getStatus() == domain::RoomStatus::EmergencyHold || facility.getStatus() == domain::RoomStatus::OutOfService) {
            return {false, "Cannot check into facility under maintenance or emergency lockdown", current, current};
        }

        if (currentTime < reservation.getStartTime()) {
            return {false, "Cannot check in: scheduled event start time has not been reached", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::InProgress);
        facility.setStatus(domain::RoomStatus::Occupied);
        facility.setDoorsLocked(false);

        return {true, "Event checked in: reservation InProgress, facility Occupied with smart doors unlocked", current, domain::ReservationStatus::InProgress};
    }

    /**
     * @brief Concludes event when end time is reached, locking doors and triggering custodial turnover.
     */
    TransitionResult conclude(domain::Reservation& reservation, domain::Facility& facility, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();
        if (current != domain::ReservationStatus::InProgress) {
            return {false, "Conclude requires InProgress state (current: " + domain::reservationStatusToString(current) + ")", current, current};
        }

        if (currentTime < reservation.getEndTime()) {
            return {false, "Cannot conclude: scheduled event end time has not been reached", current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Completed);
        facility.setStatus(domain::RoomStatus::Turnover);
        facility.setDoorsLocked(true);
        domain::NotificationService::getInstance().sendCustodialNotice(
            facility.getFacilityId(),
            "Event concluded for reservation " + reservation.getReservationId() + ". Facility locked for turnover."
        );

        return {true, "Event concluded: reservation Completed, facility in Turnover with doors locked", current, domain::ReservationStatus::Completed};
    }

    /**
     * @brief Cancels reservation. Permitted from Submitted, NeedsRevision, Draft, UnderReview, SecurityReview.
     * From Approved, strictly enforces 24-hour lead time guard.
     */
    TransitionResult cancel(domain::Reservation& reservation, const std::string& userId, std::time_t currentTime) {
        domain::ReservationStatus current = reservation.getStatus();

        if (current == domain::ReservationStatus::Approved) {
            // Enforce 24-hour lead time rule: (startTime - currentTime) >= 24 * 3600
            if ((reservation.getStartTime() - currentTime) < 24 * 3600) {
                return {false, "Cancellation rejected: approved bookings must be cancelled at least 24 hours prior to start time", current, current};
            }
        } else if (current != domain::ReservationStatus::Draft &&
                   current != domain::ReservationStatus::Submitted &&
                   current != domain::ReservationStatus::UnderReview &&
                   current != domain::ReservationStatus::NeedsRevision &&
                   current != domain::ReservationStatus::SecurityReview) {
            return {false, "Cannot cancel reservation in state: " + domain::reservationStatusToString(current), current, current};
        }

        reservation.setStatus(domain::ReservationStatus::Cancelled);
        domain::NotificationService::getInstance().sendNotification(
            reservation.getRequesterId(),
            "Reservation Cancelled: " + reservation.getReservationId(),
            "Booking cancelled by user " + userId
        );

        return {true, "Reservation cancelled successfully; all holds released", current, domain::ReservationStatus::Cancelled};
    }
};

} // namespace state
