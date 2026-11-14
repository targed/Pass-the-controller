#pragma once

#include <string>
#include <utility>
#include "domain/Facility.hpp"
#include "domain/NotificationService.hpp"

namespace state {

/**
 * @brief Encapsulates the outcome of a facility operational state transition.
 */
struct FacilityTransitionResult {
    bool success{false};
    std::string message;
    domain::RoomStatus previousStatus{domain::RoomStatus::Available};
    domain::RoomStatus newStatus{domain::RoomStatus::Available};

    FacilityTransitionResult() = default;
    FacilityTransitionResult(bool s, std::string msg, domain::RoomStatus prev, domain::RoomStatus next)
        : success(s), message(std::move(msg)), previousStatus(prev), newStatus(next) {}
};

/**
 * @brief State Machine Engine governing the 7 continuous cyclic operational states of a facility.
 */
class FacilityStateMachine {
public:
    FacilityStateMachine() = default;

    /**
     * @brief Checks structural validity of a transition between two facility operational states.
     */
    bool canTransition(domain::RoomStatus from, domain::RoomStatus to) const {
        if (from == to) {
            return false;
        }

        // Campus emergency lockdown can preempt any active state
        if (to == domain::RoomStatus::EmergencyHold) {
            return true;
        }

        switch (from) {
            case domain::RoomStatus::Available:
                return (to == domain::RoomStatus::Reserved ||
                        to == domain::RoomStatus::OutOfService);

            case domain::RoomStatus::Reserved:
                return (to == domain::RoomStatus::Setup ||
                        to == domain::RoomStatus::Available ||
                        to == domain::RoomStatus::OutOfService);

            case domain::RoomStatus::Setup:
                return (to == domain::RoomStatus::Occupied ||
                        to == domain::RoomStatus::Available ||
                        to == domain::RoomStatus::OutOfService);

            case domain::RoomStatus::Occupied:
                return (to == domain::RoomStatus::Turnover);

            case domain::RoomStatus::Turnover:
                return (to == domain::RoomStatus::Available ||
                        to == domain::RoomStatus::Setup ||
                        to == domain::RoomStatus::OutOfService);

            case domain::RoomStatus::OutOfService:
                return (to == domain::RoomStatus::Available);

            case domain::RoomStatus::EmergencyHold:
                return (to == domain::RoomStatus::Available);
        }
        return false;
    }

    /**
     * @brief Commissions or registers a physical room, placing it in Available state.
     */
    FacilityTransitionResult commissionRoom(domain::Facility& facility) {
        domain::RoomStatus current = facility.getStatus();
        facility.setStatus(domain::RoomStatus::Available);
        facility.setDoorsLocked(false);

        return {true, "Facility successfully commissioned and available for reservations", current, domain::RoomStatus::Available};
    }

    /**
     * @brief Locks calendar slot upon reservation confirmation (Available -> Reserved).
     */
    FacilityTransitionResult reservationConfirmed(domain::Facility& facility) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Available) {
            return {false, "Cannot confirm reservation: facility is not Available (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        facility.setStatus(domain::RoomStatus::Reserved);
        return {true, "Facility reserved for confirmed booking", current, domain::RoomStatus::Reserved};
    }

    /**
     * @brief Opens staging window to configure AV and equipment (Reserved -> Setup).
     */
    FacilityTransitionResult setupWindowOpens(domain::Facility& facility) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Reserved) {
            return {false, "Setup window requires Reserved state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        facility.setStatus(domain::RoomStatus::Setup);
        return {true, "Setup window opened: staging equipment and configuring facility", current, domain::RoomStatus::Setup};
    }

    /**
     * @brief Event check-in unlocks smart doors and transitions room to Occupied (Setup -> Occupied).
     */
    FacilityTransitionResult eventCheckIn(domain::Facility& facility, bool custodialClearance = true) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Setup) {
            return {false, "Event check-in requires Setup state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        if (!custodialClearance) {
            return {false, "Cannot check in: custodial readiness clearance not granted", current, current};
        }

        facility.setStatus(domain::RoomStatus::Occupied);
        facility.setDoorsLocked(false); // Smart doors unlocked for attendees

        return {true, "Event checked in: smart doors unlocked, room Occupied", current, domain::RoomStatus::Occupied};
    }

    /**
     * @brief Event concludes: locks doors, transitions to Turnover, dispatches custodial notice.
     */
    FacilityTransitionResult checkoutConcluded(domain::Facility& facility) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Occupied) {
            return {false, "Checkout requires Occupied state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        facility.setStatus(domain::RoomStatus::Turnover);
        facility.setDoorsLocked(true); // Smart doors locked
        domain::NotificationService::getInstance().sendCustodialNotice(
            facility.getFacilityId(),
            "Event concluded: doors locked. Cleaning and turnover inspection required."
        );

        return {true, "Checkout concluded: room moved to Turnover, doors locked, custodial notice sent", current, domain::RoomStatus::Turnover};
    }

    /**
     * @brief Custodial turnover inspection passed: returns facility to Available.
     */
    FacilityTransitionResult inspectionPassed(domain::Facility& facility, bool readyForUse = true) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Turnover) {
            return {false, "Turnover inspection requires Turnover state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        if (!readyForUse) {
            return {false, "Cannot pass inspection: facility is not ready for use", current, current};
        }

        facility.setStatus(domain::RoomStatus::Available);
        facility.setDoorsLocked(false);

        return {true, "Custodial inspection passed: facility clean and available", current, domain::RoomStatus::Available};
    }

    /**
     * @brief Fast-tracks back-to-back booking from Turnover straight to next Setup.
     */
    FacilityTransitionResult backToBackBooking(domain::Facility& facility) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Turnover) {
            return {false, "Back-to-back fast-track requires Turnover state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        facility.setStatus(domain::RoomStatus::Setup);
        return {true, "Back-to-back booking fast-track: transitioned from Turnover directly to Setup", current, domain::RoomStatus::Setup};
    }

    /**
     * @brief Reports damage or maintenance fault, placing room OutOfService.
     */
    FacilityTransitionResult reportDamage(domain::Facility& facility, const std::string& issue) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::Turnover &&
            current != domain::RoomStatus::Available &&
            current != domain::RoomStatus::Reserved &&
            current != domain::RoomStatus::Setup) {
            return {false, "Damage report not permitted from status: " + domain::roomStatusToString(current), current, current};
        }

        facility.setStatus(domain::RoomStatus::OutOfService);
        facility.setDoorsLocked(true);
        domain::NotificationService::getInstance().sendCustodialNotice(
            facility.getFacilityId(),
            "FACILITY DAMAGE REPORTED: " + issue + ". Status changed to OutOfService; doors locked."
        );

        return {true, "Damage reported: facility placed OutOfService with doors locked", current, domain::RoomStatus::OutOfService};
    }

    /**
     * @brief Certifies maintenance repairs and safety, returning facility to Available.
     */
    FacilityTransitionResult repairCertified(domain::Facility& facility, bool safetyPass = true) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::OutOfService) {
            return {false, "Repair certification requires OutOfService state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        if (!safetyPass) {
            return {false, "Cannot reinstate facility: safety inspection did not pass", current, current};
        }

        facility.setStatus(domain::RoomStatus::Available);
        facility.setDoorsLocked(false);

        return {true, "Repairs certified and safety verified: facility returned to Available", current, domain::RoomStatus::Available};
    }

    /**
     * @brief Declares emergency lockdown from ANY state, locking doors and alerting security.
     */
    FacilityTransitionResult declareEmergency(domain::Facility& facility, const std::string& reason) {
        domain::RoomStatus current = facility.getStatus();
        facility.setStatus(domain::RoomStatus::EmergencyHold);
        facility.setDoorsLocked(true);
        domain::NotificationService::getInstance().sendSecurityAlert(
            facility.getFacilityId(),
            "CAMPUS EMERGENCY LOCKDOWN: " + reason + ". Doors secured."
        );

        return {true, "Emergency declared: facility locked down in EmergencyHold", current, domain::RoomStatus::EmergencyHold};
    }

    /**
     * @brief Security all-clear given, lifting emergency lockdown and returning room to Available.
     */
    FacilityTransitionResult allClearGiven(domain::Facility& facility, bool securityApproved = true) {
        domain::RoomStatus current = facility.getStatus();
        if (current != domain::RoomStatus::EmergencyHold) {
            return {false, "All-clear requires EmergencyHold state (current: " + domain::roomStatusToString(current) + ")", current, current};
        }

        if (!securityApproved) {
            return {false, "Cannot lift emergency hold: security clearance approval not given", current, current};
        }

        facility.setStatus(domain::RoomStatus::Available);
        facility.setDoorsLocked(false);

        return {true, "Security all-clear confirmed: facility restored to Available", current, domain::RoomStatus::Available};
    }
};

} // namespace state
