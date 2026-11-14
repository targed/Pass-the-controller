#pragma once

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <ctime>
#include <cassert>
#include <iomanip>

#include "service/BookingSystem.hpp"

namespace service {

namespace style {
    inline constexpr const char* RESET      = "\033[0m";
    inline constexpr const char* BOLD       = "\033[1m";
    inline constexpr const char* DIM        = "\033[2m";
    inline constexpr const char* RED        = "\033[31m";
    inline constexpr const char* GREEN      = "\033[32m";
    inline constexpr const char* YELLOW     = "\033[33m";
    inline constexpr const char* BLUE       = "\033[34m";
    inline constexpr const char* MAGENTA    = "\033[35m";
    inline constexpr const char* CYAN       = "\033[36m";
    inline constexpr const char* WHITE      = "\033[37m";
    inline constexpr const char* BG_GREEN   = "\033[42m";
    inline constexpr const char* BG_RED     = "\033[41m";
}

/**
 * @brief Interactive and automated scenario runner executing full end-to-end walkthroughs
 * of the 6 core business flows for terminal presentation and verification.
 */
class ScenarioRunner {
public:
    explicit ScenarioRunner(std::ostream& out = std::cout, bool verbose = true)
        : out_(out), verbose_(verbose) {}

    /**
     * @brief Scenario 1: Standard Happy Path Booking Lifecycle.
     * Student Org booking -> Submitted -> UnderReview -> Approved -> InProgress -> Completed -> Turnover -> Available.
     */
    bool runHappyPath() {
        printHeader("SCENARIO 1: Standard Happy Path Booking Lifecycle",
                    "Student Org standard booking through event execution, conclusion, and custodial turnover.");

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 5, 16, 0, 0);
        sys.setTime(baseTime);

        // Step 1: Draft
        printStep(1, "Student Org Rep (USR-001) drafts reservation in FAC-104 (Seminar Room)...");
        std::string resId = sys.createReservation(
            "USR-001", "FAC-104", "Autonomous Drone Workshop", "Student Workshop",
            20, start, end, {}, {}, "RES-HAPPY-101"
        );
        if (resId.empty()) return fail("Failed to create reservation RES-HAPPY-101");
        auto r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::Draft) return fail("Status is not Draft");
        printDetail("Status: [Draft] | Attendance: 20 | Start: " + BookingSystem::formatTime(start));

        // Step 2: Submit
        printStep(2, "Submitting reservation (triggers schedule conflict check)...");
        auto rSubmit = sys.submitReservation(resId);
        if (!rSubmit.success) return fail("Submit failed: " + rSubmit.message);
        r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::Submitted) return fail("Status is not Submitted");
        printDetail("Status: [Submitted] | 15-Minute Soft Lock active until " + BookingSystem::formatTime(r->getHoldExpiresAt()));

        // Step 3: Route to Review
        printStep(3, "Routing to Event Approver queue...");
        auto rRoute = sys.routeToReview(resId);
        if (!rRoute.success) return fail("Route to review failed: " + rRoute.message);
        r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::UnderReview) return fail("Status is not UnderReview");
        printDetail("Status: [UnderReview] | In Event Approver review queue");

        // Step 4: Approve
        printStep(4, "Event Approver (USR-005) evaluates and approves booking...");
        auto rApprove = sys.approveReservation(resId, "USR-005", "Approved: verified organization eligibility and room capacity.");
        if (!rApprove.success) return fail("Approve failed: " + rApprove.message);
        r = sys.getReservation(resId);
        auto fac = sys.getFacility("FAC-104");
        if (!r || r->getStatus() != domain::ReservationStatus::Approved) return fail("Status is not Approved");
        if (!fac || fac->getStatus() != domain::RoomStatus::Reserved) return fail("Facility status is not Reserved");
        printDetail("Status: [Approved] | Room FAC-104 Status: [Reserved] (Calendar slot locked permanently)");

        // Step 5: Advance clock to event start
        printStep(5, "Simulated clock advances to event start time (2026-09-05 14:00:00)...");
        sys.setTime(start);
        printDetail("Simulated Clock: " + BookingSystem::formatTime(sys.getCurrentTime()));

        // Step 6: Check-in
        printStep(6, "Event organizer checks in at venue entrance...");
        auto rCheckIn = sys.checkInReservation(resId);
        if (!rCheckIn.success) return fail("Check-in failed: " + rCheckIn.message);
        r = sys.getReservation(resId);
        fac = sys.getFacility("FAC-104");
        if (!r || r->getStatus() != domain::ReservationStatus::InProgress) return fail("Status is not InProgress");
        if (!fac || fac->getStatus() != domain::RoomStatus::Occupied || fac->areDoorsLocked()) return fail("Facility not Occupied or doors not unlocked");
        printDetail("Status: [InProgress] | Room FAC-104 Status: [Occupied] | Smart Doors: [UNLOCKED]");

        // Step 7: Advance clock to event end
        printStep(7, "Simulated clock advances to event end time (2026-09-05 16:00:00)...");
        sys.setTime(end);
        printDetail("Simulated Clock: " + BookingSystem::formatTime(sys.getCurrentTime()));

        // Step 8: Conclude
        printStep(8, "Event concludes and checkout is processed...");
        auto rConclude = sys.concludeReservation(resId);
        if (!rConclude.success) return fail("Conclude failed: " + rConclude.message);
        r = sys.getReservation(resId);
        fac = sys.getFacility("FAC-104");
        if (!r || r->getStatus() != domain::ReservationStatus::Completed) return fail("Status is not Completed");
        if (!fac || fac->getStatus() != domain::RoomStatus::Turnover || !fac->areDoorsLocked()) return fail("Facility not in Turnover or doors not locked");
        printDetail("Status: [Completed] | Room FAC-104 Status: [Turnover] | Smart Doors: [LOCKED] | Custodial notice dispatched");

        // Step 9: Custodial Turnover
        printStep(9, "Custodial crew cleans, sanitizes, and certifies room inspection...");
        auto rInspect = sys.completeInspection("FAC-104");
        if (!rInspect.success) return fail("Custodial inspection failed: " + rInspect.message);
        fac = sys.getFacility("FAC-104");
        if (!fac || fac->getStatus() != domain::RoomStatus::Available || fac->areDoorsLocked()) return fail("Facility not restored to Available");
        printDetail("Room FAC-104 Status: [Available] | Smart Doors: [NORMAL/UNLOCKED] | Ready for next booking");

        return pass("SCENARIO 1: Standard Happy Path executed cleanly end-to-end.");
    }

    /**
     * @brief Scenario 2: Large Event Conditional Security Review & Clearance.
     * Attendance > 100 triggers conditional security review gate; cleared by Security Officer.
     */
    bool runLargeEventSecurityScenario() {
        printHeader("SCENARIO 2: Large Event Security Review & Clearance",
                    "Headcount > 100 conditional security gate triggering officer assignment and clearance.");

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 10, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 10, 13, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 10, 17, 0, 0);
        sys.setTime(baseTime);

        // Step 1: Draft large event in Centennial Auditorium (FAC-101, cap 250)
        printStep(1, "Student Org Rep (USR-002) drafts large event (220 attendees) in FAC-101...");
        std::string resId = sys.createReservation(
            "USR-002", "FAC-101", "National Collegiate Debate Championship", "Competition",
            220, start, end, {}, {}, "RES-SEC-201"
        );
        if (resId.empty()) return fail("Failed to create reservation RES-SEC-201");
        auto r = sys.getReservation(resId);
        if (!r || !r->getRequiresSecurity()) return fail("RequiresSecurity flag not set for attendance 220");
        printDetail("Status: [Draft] | Attendance: 220 (> 100 threshold) | RequiresSecurity: TRUE");

        // Step 2: Submit
        printStep(2, "Submitting large event request...");
        auto rSubmit = sys.submitReservation(resId);
        if (!rSubmit.success) return fail("Submit failed: " + rSubmit.message);
        printDetail("Status: [Submitted] | 15-Minute Soft Lock placed");

        // Step 3: Route to UnderReview
        printStep(3, "Routing to initial policy evaluation...");
        auto rRoute = sys.routeToReview(resId);
        if (!rRoute.success) return fail("Route to review failed: " + rRoute.message);
        printDetail("Status: [UnderReview]");

        // Step 4: Evaluate Security (Conditional gate)
        printStep(4, "System evaluates safety risk profile against campus security policy...");
        auto rSecEval = sys.evaluateSecurity(resId);
        if (!rSecEval.success) return fail("Security evaluation failed: " + rSecEval.message);
        r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::SecurityReview) return fail("Status is not SecurityReview");
        printDetail("Risk evaluation: Headcount (220) exceeds safety threshold (100).");
        printDetail("Status: [SecurityReview] | High-risk alert sent to campus-police dispatch");

        // Step 5: Guard check - standard approval must be BLOCKED
        printStep(5, "Verifying guard: Standard approver attempts direct approval without security clearance...");
        auto rPrematureApprove = sys.approveReservation(resId, "USR-005", "Standard approval attempt");
        if (rPrematureApprove.success) return fail("Direct approval should have been blocked in SecurityReview!");
        printDetail("Blocked as expected: \"" + rPrematureApprove.message + "\"");

        // Step 6: Security Officer Review and Clearance
        printStep(6, "Campus Security Officer (USR-006) evaluates crowd management and egress plan...");
        auto rClear = sys.securityClear(resId, "USR-006", "Security detail approved: 2 officers assigned to venue entrances.");
        if (!rClear.success) return fail("Security clearance failed: " + rClear.message);
        r = sys.getReservation(resId);
        auto fac = sys.getFacility("FAC-101");
        if (!r || r->getStatus() != domain::ReservationStatus::Approved) return fail("Status is not Approved");
        if (!fac || fac->getStatus() != domain::RoomStatus::Reserved) return fail("Facility status is not Reserved");
        printDetail("Status: [Approved] | Officer USR-006 assigned | Room FAC-101 Status: [Reserved]");

        // Step 7: Check-in and Conclude
        printStep(7, "Advancing clock to start time -> Check-In...");
        sys.setTime(start);
        auto rCheckIn = sys.checkInReservation(resId);
        if (!rCheckIn.success) return fail("Check-in failed: " + rCheckIn.message);
        printDetail("Status: [InProgress] | Room FAC-101: [Occupied] | Smart Doors: [UNLOCKED]");

        printStep(8, "Advancing clock to end time -> Conclude event...");
        sys.setTime(end);
        auto rConclude = sys.concludeReservation(resId);
        if (!rConclude.success) return fail("Conclude failed: " + rConclude.message);
        printDetail("Status: [Completed] | Room FAC-101: [Turnover] | Doors: [LOCKED]");

        return pass("SCENARIO 2: Large event security evaluation and clearance verified.");
    }

    /**
     * @brief Scenario 3: Approver Revision Request & Resubmission Loop.
     * Approver requests adjustments -> NeedsRevision -> Requester updates -> Resubmit -> Approved.
     */
    bool runRevisionLoopScenario() {
        printHeader("SCENARIO 3: Approver Revision Request & Resubmission Loop",
                    "Approver requests changes; requester resubmits modified booking within deadline.");

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 11, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 12, 10, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 12, 15, 0, 0);
        sys.setTime(baseTime);

        // Step 1: Draft and Submit in Student Union Ballroom (FAC-103)
        printStep(1, "Student Org Rep (USR-001) drafts booking for Robotics Hackathon in FAC-103...");
        std::string resId = sys.createReservation(
            "USR-001", "FAC-103", "Robotics Hackathon & Showcase", "Hackathon",
            80, start, end, {}, {}, "RES-REV-301"
        );
        sys.submitReservation(resId);
        sys.routeToReview(resId);
        auto r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::UnderReview) return fail("Status is not UnderReview");
        printDetail("Status: [UnderReview] | Awaiting Event Approver policy check");

        // Step 2: Approver requests changes
        printStep(2, "Event Approver (USR-005) reviews submission and notes missing table layout...");
        std::time_t revisionDeadline = baseTime + (48 * 3600); // 48-hour deadline
        auto rReqChanges = sys.requestChanges(
            resId, "USR-005",
            "Please provide detailed table layout diagram and verify 36-inch fire aisle clearance.",
            revisionDeadline
        );
        if (!rReqChanges.success) return fail("Request changes failed: " + rReqChanges.message);
        r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::NeedsRevision) return fail("Status is not NeedsRevision");
        printDetail("Status: [NeedsRevision] | Revision Deadline: " + BookingSystem::formatTime(revisionDeadline));
        printDetail("Action notice dispatched to requester alice@studentorg.campus.edu");

        // Step 3: Guard check - Cannot approve while in NeedsRevision
        printStep(3, "Verifying guard: Attempting approval before changes are resubmitted...");
        auto rPremature = sys.approveReservation(resId, "USR-005", "Premature approval");
        if (rPremature.success) return fail("Approval should not succeed while in NeedsRevision!");
        printDetail("Blocked as expected: \"" + rPremature.message + "\"");

        // Step 4: Requester updates and resubmits 12 hours later (well before 48h deadline)
        printStep(4, "Requester attaches updated table layout and resubmits 12 hours later...");
        sys.advanceTime(12 * 3600);
        auto rResubmit = sys.resubmitReservation(resId);
        if (!rResubmit.success) return fail("Resubmit failed: " + rResubmit.message);
        r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::Submitted) return fail("Status is not Submitted");
        printDetail("Status: [Submitted] | Refreshed 15-minute soft lock active until " + BookingSystem::formatTime(r->getHoldExpiresAt()));

        // Step 5: Route to Review & Final Approval
        printStep(5, "Routing revised booking back to Approver review queue...");
        sys.routeToReview(resId);
        r = sys.getReservation(resId);
        if (!r || r->getStatus() != domain::ReservationStatus::UnderReview) return fail("Status is not UnderReview");
        printDetail("Status: [UnderReview]");

        printStep(6, "Event Approver reviews updated floor plan and grants formal approval...");
        auto rFinalApprove = sys.approveReservation(resId, "USR-005", "Layout diagram verified. Fire aisles confirmed compliant.");
        if (!rFinalApprove.success) return fail("Final approval failed: " + rFinalApprove.message);
        r = sys.getReservation(resId);
        auto fac = sys.getFacility("FAC-103");
        if (!r || r->getStatus() != domain::ReservationStatus::Approved) return fail("Status is not Approved");
        if (!fac || fac->getStatus() != domain::RoomStatus::Reserved) return fail("Facility status is not Reserved");
        printDetail("Status: [Approved] | Room FAC-103 Status: [Reserved] | Confirmation notice dispatched");

        return pass("SCENARIO 3: Revision request and resubmission loop verified.");
    }

    /**
     * @brief Scenario 4: Facility Logistics, Staging & Custodial Turnover Cycle.
     * Focuses on physical room operational transitions: Available -> Reserved -> Setup -> Occupied -> Turnover -> Available.
     */
    bool runFacilityLogisticsScenario() {
        printHeader("SCENARIO 4: Facility Staging & Custodial Turnover Cycle",
                    "Complete 5-phase physical room operational cycle with equipment staging and cleaning.");

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 3, 10, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 3, 14, 0, 0);
        sys.setTime(baseTime);

        // Step 1: Initial Available state
        printStep(1, "Initial State: Facility FAC-103 (Student Union Ballroom) is Available in directory...");
        auto fac = sys.getFacility("FAC-103");
        if (!fac || fac->getStatus() != domain::RoomStatus::Available) return fail("FAC-103 not Available");
        printDetail("Room Status: [Available] | Smart Doors: [NORMAL/UNLOCKED for inspections]");

        // Step 2: Confirmed booking places room in Reserved
        printStep(2, "Faculty Member (USR-003) books and confirms symposium in FAC-103...");
        std::string resId = sys.createReservation(
            "USR-003", "FAC-103", "Annual Faculty Research Symposium", "Symposium",
            50, start, end, {}, {}, "RES-FAC-401"
        );
        sys.submitReservation(resId);
        sys.routeToReview(resId);
        sys.approveReservation(resId, "USR-005", "Approved");
        fac = sys.getFacility("FAC-103");
        if (!fac || fac->getStatus() != domain::RoomStatus::Reserved) return fail("FAC-103 not Reserved");
        printDetail("Transition: [Available] ──(reservationConfirmed)──> [Reserved]");

        // Step 3: Setup staging window
        printStep(3, "Facility Coordinator (USR-004) opens pre-event staging window 2 hours prior...");
        auto rSetup = sys.openSetupWindow("FAC-103");
        if (!rSetup.success) return fail("Open setup window failed: " + rSetup.message);
        fac = sys.getFacility("FAC-103");
        if (!fac || fac->getStatus() != domain::RoomStatus::Setup) return fail("FAC-103 not in Setup");
        printDetail("Transition: [Reserved] ──(setupWindowOpens)──> [Setup]");
        printDetail("Staff actively staging: Sound System, Banquet Tables, Wireless Microphones");

        // Step 4: Event start -> Check-In -> Occupied
        printStep(4, "Event start time reached: Organizer checks in, unlocking smart doors...");
        sys.setTime(start);
        auto rCheckIn = sys.checkInReservation(resId);
        if (!rCheckIn.success) return fail("Check-in failed: " + rCheckIn.message);
        fac = sys.getFacility("FAC-103");
        if (!fac || fac->getStatus() != domain::RoomStatus::Occupied || fac->areDoorsLocked()) return fail("FAC-103 not Occupied or doors not unlocked");
        printDetail("Transition: [Setup] ──(eventCheckIn)──> [Occupied]");
        printDetail("Smart Doors: [UNLOCKED] | Attendees in session");

        // Step 5: Event ends -> Checkout -> Turnover
        printStep(5, "Event concludes: Checkout triggers door lock and custodial dispatch...");
        sys.setTime(end);
        auto rConclude = sys.concludeReservation(resId);
        if (!rConclude.success) return fail("Conclude failed: " + rConclude.message);
        fac = sys.getFacility("FAC-103");
        if (!fac || fac->getStatus() != domain::RoomStatus::Turnover || !fac->areDoorsLocked()) return fail("FAC-103 not in Turnover or doors not locked");
        printDetail("Transition: [Occupied] ──(checkoutConcluded)──> [Turnover]");
        printDetail("Smart Doors: [LOCKED] | Custodial alert: Cleaning and resetting furniture required");

        // Step 6: Turnover inspection passed -> Available
        printStep(6, "Custodial staff completes cleaning, resets room layout, and logs inspection pass...");
        auto rPass = sys.completeInspection("FAC-103", true);
        if (!rPass.success) return fail("Inspection pass failed: " + rPass.message);
        fac = sys.getFacility("FAC-103");
        if (!fac || fac->getStatus() != domain::RoomStatus::Available || fac->areDoorsLocked()) return fail("FAC-103 not restored to Available");
        printDetail("Transition: [Turnover] ──(inspectionPassed)──> [Available]");
        printDetail("Room Status: [Available] | Smart Doors: [NORMAL/UNLOCKED] | Cycle complete");

        return pass("SCENARIO 4: 5-Phase Facility Operational Cycle verified.");
    }

    /**
     * @brief Scenario 5: Campus Emergency Lockdown & All-Clear Restoration.
     * Emergency lockdown preempts all active venues; lifting lockdown restores Available state.
     */
    bool runEmergencyLockdownScenario() {
        printHeader("SCENARIO 5: Campus Emergency Lockdown & All-Clear Restoration",
                    "Simulated campus security alert locking down all venues, followed by security all-clear.");

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 14, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 1, 13, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 1, 16, 0, 0);
        sys.setTime(baseTime);

        // Step 1: Active event in Centennial Auditorium (FAC-101)
        printStep(1, "Active assembly underway in FAC-101 (Occupied, doors unlocked)...");
        std::string resId = sys.createReservation(
            "USR-001", "FAC-101", "Campus Assembly", "Assembly",
            90, start, end, {}, {}, "RES-EMERG-501"
        );
        sys.submitReservation(resId);
        sys.routeToReview(resId);
        sys.approveReservation(resId, "USR-005", "Approved");
        sys.setTime(start);
        sys.checkInReservation(resId);
        auto fac101 = sys.getFacility("FAC-101");
        auto fac102 = sys.getFacility("FAC-102");
        if (!fac101 || fac101->getStatus() != domain::RoomStatus::Occupied || fac101->areDoorsLocked()) return fail("FAC-101 not Occupied");
        if (!fac102 || fac102->getStatus() != domain::RoomStatus::Available) return fail("FAC-102 not Available");
        printDetail("FAC-101: [Occupied] (Doors UNLOCKED) | FAC-102, 103, 104: [Available]");

        // Step 2: Emergency Lockdown
        printStep(2, "Campus Safety declares emergency: \"Active security and severe weather threat\"...");
        auto rEmerg = sys.declareCampusEmergency("Active perimeter security alert and severe storm");
        if (!rEmerg.success) return fail("Declare emergency failed: " + rEmerg.message);
        fac101 = sys.getFacility("FAC-101");
        fac102 = sys.getFacility("FAC-102");
        auto fac103 = sys.getFacility("FAC-103");
        auto fac104 = sys.getFacility("FAC-104");
        if (!fac101 || fac101->getStatus() != domain::RoomStatus::EmergencyHold || !fac101->areDoorsLocked()) return fail("FAC-101 not in EmergencyHold or not locked");
        if (!fac102 || fac102->getStatus() != domain::RoomStatus::EmergencyHold || !fac102->areDoorsLocked()) return fail("FAC-102 not in EmergencyHold or not locked");
        if (!fac103 || fac103->getStatus() != domain::RoomStatus::EmergencyHold || !fac103->areDoorsLocked()) return fail("FAC-103 not in EmergencyHold or not locked");
        if (!fac104 || fac104->getStatus() != domain::RoomStatus::EmergencyHold || !fac104->areDoorsLocked()) return fail("FAC-104 not in EmergencyHold or not locked");
        printDetail("ALL facilities transitioned to [EmergencyHold]! All smart doors securely [LOCKED].");

        // Step 3: Guard verification - Booking attempts during lockdown must be blocked
        printStep(3, "Verifying guard: Requester attempts to submit booking during lockdown...");
        std::string blockedId = sys.createReservation(
            "USR-002", "FAC-102", "Study Group", "Meeting",
            10, start + 3600, end + 3600, {}, {}, "RES-BLOCKED"
        );
        auto rSubmitBlocked = sys.submitReservation(blockedId);
        if (rSubmitBlocked.success) return fail("Booking during lockdown should have failed conflict/availability check!");
        printDetail("Blocked as expected: Schedule conflict/blackout detected because venue is in EmergencyHold.");

        // Step 4: Security all-clear given
        printStep(4, "Campus Police completes venue sweep and issues all-clear...");
        auto rClear = sys.clearCampusEmergency();
        if (!rClear.success) return fail("Clear emergency failed: " + rClear.message);
        fac101 = sys.getFacility("FAC-101");
        fac102 = sys.getFacility("FAC-102");
        fac103 = sys.getFacility("FAC-103");
        fac104 = sys.getFacility("FAC-104");
        if (!fac101 || fac101->getStatus() != domain::RoomStatus::Available || fac101->areDoorsLocked()) return fail("FAC-101 not restored to Available");
        if (!fac102 || fac102->getStatus() != domain::RoomStatus::Available || fac102->areDoorsLocked()) return fail("FAC-102 not restored to Available");
        if (!fac103 || fac103->getStatus() != domain::RoomStatus::Available || fac103->areDoorsLocked()) return fail("FAC-103 not restored to Available");
        if (!fac104 || fac104->getStatus() != domain::RoomStatus::Available || fac104->areDoorsLocked()) return fail("FAC-104 not restored to Available");
        printDetail("All facilities restored to [Available]. Smart doors unlocked for normal operations.");

        return pass("SCENARIO 5: Campus emergency preemption and restoration verified.");
    }

    /**
     * @brief Scenario 6: Cancellation Policy & 24-Hour Lead Time Constraint.
     * Validates that approved bookings can only be cancelled with >= 24h notice.
     */
    bool runCancellationRulesScenario() {
        printHeader("SCENARIO 6: Cancellation Policy & 24-Hour Lead Time Guard",
                    "Enforcing cancellation rules: permitted > 24 hours prior; strictly blocked < 24 hours.");

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 5, 17, 0, 0);
        sys.setTime(baseTime);

        // --- Part A: Valid cancellation with > 24 hours lead time ---
        printSubHeader("Part A: Valid Cancellation with > 24 Hours Lead Time (~76h before event)");
        printStep(1, "Booking created and approved for event on 2026-09-05 14:00:00...");
        std::string resId1 = sys.createReservation(
            "USR-001", "FAC-102", "STEM Robotics Workshop", "Workshop",
            30, start, end, {}, {}, "RES-CANCEL-601"
        );
        sys.submitReservation(resId1);
        sys.routeToReview(resId1);
        sys.approveReservation(resId1, "USR-005", "Approved");
        auto fac = sys.getFacility("FAC-102");
        if (!fac || fac->getStatus() != domain::RoomStatus::Reserved) return fail("FAC-102 not Reserved");
        printDetail("Status: [Approved] | Room FAC-102: [Reserved]");

        printStep(2, "Clock advances to 2026-09-02 10:00:00 (~76 hours prior to event)...");
        sys.setTime(BookingSystem::makeTime(2026, 9, 2, 10, 0, 0));
        auto rCancel1 = sys.cancelReservation(resId1, "USR-001");
        if (!rCancel1.success) return fail("Valid cancellation failed: " + rCancel1.message);
        auto r1 = sys.getReservation(resId1);
        fac = sys.getFacility("FAC-102");
        if (!r1 || r1->getStatus() != domain::ReservationStatus::Cancelled) return fail("Status is not Cancelled");
        if (!fac || fac->getStatus() != domain::RoomStatus::Available) return fail("Facility not released to Available");
        printDetail("Cancellation SUCCESS: Status -> [Cancelled] | Facility lock released -> [Available]");

        // --- Part B: Late cancellation with < 24 hours lead time ---
        printSubHeader("Part B: Late Cancellation Blocked (< 24 Hours Prior to Event Start)");
        printStep(3, "Second booking created and approved for the same venue...");
        std::string resId2 = sys.createReservation(
            "USR-002", "FAC-102", "Urgent Debate Practice", "Meeting",
            30, start, end, {}, {}, "RES-CANCEL-602"
        );
        sys.submitReservation(resId2);
        sys.routeToReview(resId2);
        sys.approveReservation(resId2, "USR-005", "Approved");
        printDetail("Status: [Approved] | Room FAC-102: [Reserved]");

        printStep(4, "Clock advances to 2026-09-05 08:00:00 (only 6 hours before event!)...");
        sys.setTime(BookingSystem::makeTime(2026, 9, 5, 8, 0, 0));
        printDetail("Simulated Time: 2026-09-05 08:00:00 | Time to Event: 6 hours (< 24h rule)");

        printStep(5, "Requester attempts late cancellation...");
        auto rCancel2 = sys.cancelReservation(resId2, "USR-002");
        if (rCancel2.success) return fail("Late cancellation should have been REJECTED by 24h lead time guard!");
        auto r2 = sys.getReservation(resId2);
        fac = sys.getFacility("FAC-102");
        if (!r2 || r2->getStatus() != domain::ReservationStatus::Approved) return fail("Status changed from Approved unexpectedly");
        if (!fac || fac->getStatus() != domain::RoomStatus::Reserved) return fail("Facility lock released unexpectedly");
        printDetail("Cancellation REJECTED: \"" + rCancel2.message + "\"");
        printDetail("Reservation remains [Approved] | Room FAC-102 remains [Reserved]");

        return pass("SCENARIO 6: 24-Hour lead time cancellation guard strictly enforced.");
    }

    /**
     * @brief Scenario 7: Executes all 6 scenarios in sequence and displays summary table.
     */
    bool runAll() {
        out_ << "\n" << style::BOLD << style::CYAN
             << "╔══════════════════════════════════════════════════════════════════════════════╗\n"
             << "║  SMART CAMPUS EVENT & FACILITY BOOKING SYSTEM - SCENARIO DEMONSTRATIONS     ║\n"
             << "╚══════════════════════════════════════════════════════════════════════════════╝\n"
             << style::RESET << "\n";

        struct Result {
            std::string name;
            bool passed;
        };

        std::vector<Result> results;
        results.push_back({"Standard Happy Path Booking", runHappyPath()});
        results.push_back({"Large Event Security Review", runLargeEventSecurityScenario()});
        results.push_back({"Revision Request & Resubmission", runRevisionLoopScenario()});
        results.push_back({"Facility Logistics & Staging Cycle", runFacilityLogisticsScenario()});
        results.push_back({"Emergency Lockdown & Restoration", runEmergencyLockdownScenario()});
        results.push_back({"Cancellation Rules (24h Guard)", runCancellationRulesScenario()});

        // Summary Table
        out_ << "\n" << style::BOLD
             << "╔══════════════════════════════════════════════════════════════════════════════╗\n"
             << "║                       SCENARIO EXECUTION SUMMARY                             ║\n"
             << "╠══════════════════════════════════════════════════════════════════════════════╣\n";

        bool allPassed = true;
        for (size_t i = 0; i < results.size(); ++i) {
            const auto& r = results[i];
            std::string indexStr = std::to_string(i + 1) + ". " + r.name;
            while (indexStr.length() < 60) indexStr += " ";

            std::string badge = r.passed
                ? std::string(style::GREEN) + "[PASS]" + style::RESET
                : std::string(style::RED) + "[FAIL]" + style::RESET;

            out_ << "║ " << indexStr << " " << badge << "    ║\n";
            if (!r.passed) allPassed = false;
        }

        out_ << "╠══════════════════════════════════════════════════════════════════════════════╣\n";
        if (allPassed) {
            out_ << "║ " << style::GREEN << "Result: ALL 6 SCENARIOS COMPLETED SUCCESSFULLY (6/6 PASS)" << style::RESET;
            std::string fill = "Result: ALL 6 SCENARIOS COMPLETED SUCCESSFULLY (6/6 PASS)";
            for (size_t i = fill.length(); i < 76; ++i) out_ << " ";
            out_ << "║\n";
        } else {
            out_ << "║ " << style::RED << "Result: SOME SCENARIOS FAILED - CHECK OUTPUT ABOVE" << style::RESET;
            std::string fill = "Result: SOME SCENARIOS FAILED - CHECK OUTPUT ABOVE";
            for (size_t i = fill.length(); i < 76; ++i) out_ << " ";
            out_ << "║\n";
        }
        out_ << "╚══════════════════════════════════════════════════════════════════════════════╝\n\n";

        return allPassed;
    }

private:
    void printHeader(std::string_view title, std::string_view description) {
        if (!verbose_) return;
        out_ << "\n" << style::BOLD << style::BLUE
             << "╔══════════════════════════════════════════════════════════════════════════════╗\n";
        std::string t = "║ " + std::string(title);
        while (t.length() < 79) t += " ";
        t += "║\n";
        out_ << t;
        out_ << "╠══════════════════════════════════════════════════════════════════════════════╣\n";
        std::string d = "║ " + std::string(description);
        while (d.length() < 79) d += " ";
        d += "║\n";
        out_ << style::DIM << d << style::RESET;
        out_ << style::BOLD << style::BLUE
             << "╚══════════════════════════════════════════════════════════════════════════════╝\n"
             << style::RESET;
    }

    void printSubHeader(std::string_view subtitle) {
        if (!verbose_) return;
        out_ << "\n  " << style::BOLD << style::CYAN << "── " << subtitle << " ──" << style::RESET << "\n";
    }

    void printStep(int stepNum, std::string_view desc) {
        if (!verbose_) return;
        out_ << "  " << style::BOLD << style::YELLOW << "[Step " << stepNum << "] " << style::RESET
             << desc << "\n";
    }

    void printDetail(std::string_view detail) {
        if (!verbose_) return;
        out_ << "         " << style::DIM << "└─ " << detail << style::RESET << "\n";
    }

    bool pass(std::string_view msg) {
        if (verbose_) {
            out_ << "  " << style::BOLD << style::GREEN << "✓ " << msg << style::RESET << "\n";
        }
        return true;
    }

    bool fail(std::string_view msg) {
        if (verbose_) {
            out_ << "  " << style::BOLD << style::RED << "✗ ERROR: " << msg << style::RESET << "\n";
        }
        return false;
    }

    std::ostream& out_;
    bool verbose_{true};
};

} // namespace service
