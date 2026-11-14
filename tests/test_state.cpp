#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <ctime>

#include "domain/User.hpp"
#include "domain/Facility.hpp"
#include "domain/NotificationService.hpp"
#include "domain/Reservation.hpp"
#include "state/ReservationStateMachine.hpp"
#include "state/FacilityStateMachine.hpp"

using namespace domain;
using namespace state;

static std::time_t makeLocalTime(int year, int month, int day, int hour, int minute) {
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;
    return std::mktime(&tm);
}

void testReservationStandardHappyPath() {
    std::cout << "[RUN] testReservationStandardHappyPath\n";
    ReservationStateMachine sm;
    NotificationService::getInstance().clear();

    std::time_t now = makeLocalTime(2026, 9, 1, 9, 0);       // Sept 1, 09:00
    std::time_t eventStart = makeLocalTime(2026, 9, 5, 14, 0); // Sept 5, 14:00 (midday, >24h lead time)
    std::time_t eventEnd = makeLocalTime(2026, 9, 5, 16, 0);   // Sept 5, 16:00 (midday)

    Reservation res("res-101", "student-alice", "room-101", "ACM General Body Meeting", "Club Meeting", 50, eventStart, eventEnd, now);
    Facility fac("room-101", "Oak Hall 101", "Oak Hall", 100);

    assert(res.getStatus() == ReservationStatus::Draft);
    assert(res.getRequiresSecurity() == false);

    // 1. Submit
    TransitionResult rSubmit = sm.submit(res, true, now);
    assert(rSubmit.success == true);
    assert(res.getStatus() == ReservationStatus::Submitted);
    assert(res.getHoldExpiresAt() == now + 900);

    // 2. Route to Review within 15 min buffer
    TransitionResult rRoute = sm.routeToReview(res, now + 300);
    assert(rRoute.success == true);
    assert(res.getStatus() == ReservationStatus::UnderReview);

    // 3. Evaluate Security (<= 100, day time)
    TransitionResult rSecEval = sm.evaluateSecurity(res);
    assert(rSecEval.success == true);
    assert(res.getStatus() == ReservationStatus::UnderReview); // Remains in UnderReview

    // 4. Approve
    TransitionResult rApprove = sm.approve(res, "dean-dave", "Approved for ACM chapter", now + 600);
    assert(rApprove.success == true);
    assert(res.getStatus() == ReservationStatus::Approved);
    assert(res.getApprovals().size() == 1);
    assert(res.getApprovals()[0].getApproverId() == "dean-dave");
    assert(res.getApprovals()[0].getDecision() == "Approved");
    assert(NotificationService::getInstance().count() >= 1);

    // 5. Check in when start time arrives
    fac.setStatus(RoomStatus::Setup);
    TransitionResult rCheckInEarly = sm.checkIn(res, fac, eventStart - 60);
    assert(rCheckInEarly.success == false); // Too early!

    TransitionResult rCheckIn = sm.checkIn(res, fac, eventStart);
    assert(rCheckIn.success == true);
    assert(res.getStatus() == ReservationStatus::InProgress);
    assert(fac.getStatus() == RoomStatus::Occupied);
    assert(fac.areDoorsLocked() == false);

    // 6. Conclude when end time arrives
    TransitionResult rConcludeEarly = sm.conclude(res, fac, eventEnd - 60);
    assert(rConcludeEarly.success == false); // Too early!

    TransitionResult rConclude = sm.conclude(res, fac, eventEnd);
    assert(rConclude.success == true);
    assert(res.getStatus() == ReservationStatus::Completed);
    assert(fac.getStatus() == RoomStatus::Turnover);
    assert(fac.areDoorsLocked() == true);

    std::cout << "[PASS] testReservationStandardHappyPath\n";
}

void testReservationSecurityWorkflow() {
    std::cout << "[RUN] testReservationSecurityWorkflow\n";
    ReservationStateMachine sm;
    NotificationService::getInstance().clear();

    std::time_t now = makeLocalTime(2026, 9, 1, 9, 0);
    std::time_t eventStart = makeLocalTime(2026, 9, 5, 14, 0);
    std::time_t eventEnd = makeLocalTime(2026, 9, 5, 17, 0);

    // Headcount > 100 triggers security gate
    Reservation res("res-202", "student-bob", "auditorium-1", "Campus Hackathon Kickoff", "Conference", 250, eventStart, eventEnd, now);
    assert(res.getRequiresSecurity() == true);

    assert(sm.submit(res, true, now).success == true);
    assert(sm.routeToReview(res, now + 100).success == true);

    // Security evaluation moves to SecurityReview
    TransitionResult rSecEval = sm.evaluateSecurity(res);
    assert(rSecEval.success == true);
    assert(res.getStatus() == ReservationStatus::SecurityReview);

    // Direct standard approval must fail while in SecurityReview or if requires security
    TransitionResult rDirectApprove = sm.approve(res, "dean-dave", "Looks good", now + 200);
    assert(rDirectApprove.success == false);

    // Security Clear by Security Officer
    TransitionResult rClear = sm.securityClear(res, "officer-eve", "Officer Eve and unit assigned for perimeter patrol", now + 300);
    assert(rClear.success == true);
    assert(res.getStatus() == ReservationStatus::Approved);
    assert(res.getSupportRequests().size() >= 1);
    assert(res.getSupportRequests()[0].isAssigned() == true);
    assert(res.getSupportRequests()[0].getAssignedOfficerId() == "officer-eve");

    // Test Security Deny branch with another reservation
    Reservation resDeny("res-203", "student-charlie", "auditorium-1", "Late Night Rave", "Party", 300, eventStart, eventEnd, now);
    assert(sm.submit(resDeny, true, now).success == true);
    assert(sm.routeToReview(resDeny, now + 50).success == true);
    assert(sm.evaluateSecurity(resDeny).success == true);
    assert(resDeny.getStatus() == ReservationStatus::SecurityReview);

    TransitionResult rDeny = sm.securityDeny(resDeny, "officer-eve", "Over capacity for sound equipment and security risk", now + 150);
    assert(rDeny.success == true);
    assert(resDeny.getStatus() == ReservationStatus::Rejected);

    std::cout << "[PASS] testReservationSecurityWorkflow\n";
}

void testReservationRevisionAndExpiration() {
    std::cout << "[RUN] testReservationRevisionAndExpiration\n";
    ReservationStateMachine sm;
    NotificationService::getInstance().clear();

    std::time_t now = makeLocalTime(2026, 9, 1, 9, 0);
    std::time_t deadline = now + 86400; // 24 hours to revise
    std::time_t eventStart = makeLocalTime(2026, 9, 5, 14, 0);
    std::time_t eventEnd = makeLocalTime(2026, 9, 5, 16, 0);

    Reservation res("res-303", "prof-dan", "lab-200", "Robotics Workshop", "Workshop", 30, eventStart, eventEnd, now);
    assert(sm.submit(res, true, now).success == true);
    assert(sm.routeToReview(res, now + 100).success == true);

    // Approver requests revision
    TransitionResult rRev = sm.requestRevision(res, "approver-carol", "Please specify safety equipment required", deadline, now + 200);
    assert(rRev.success == true);
    assert(res.getStatus() == ReservationStatus::NeedsRevision);
    assert(res.getRevisionDeadline() == deadline);

    // Resubmit with conflict fails
    TransitionResult rConflict = sm.resubmit(res, false, now + 300);
    assert(rConflict.success == false);
    assert(res.getStatus() == ReservationStatus::NeedsRevision);

    // Resubmit successfully
    TransitionResult rResubmit = sm.resubmit(res, true, now + 400);
    assert(rResubmit.success == true);
    assert(res.getStatus() == ReservationStatus::Submitted);
    assert(res.getHoldExpiresAt() == (now + 400 + 900));

    // Route again and request revision again to test deadline expiration
    assert(sm.routeToReview(res, now + 500).success == true);
    assert(sm.requestRevision(res, "approver-carol", "Update headcount", now + 1000, now + 600).success == true);
    assert(res.getStatus() == ReservationStatus::NeedsRevision);

    // Try expire before deadline
    TransitionResult rExpireEarly = sm.expire(res, now + 999);
    assert(rExpireEarly.success == false);
    assert(res.getStatus() == ReservationStatus::NeedsRevision);

    // Expire after deadline passed
    TransitionResult rExpire = sm.expire(res, now + 1001);
    assert(rExpire.success == true);
    assert(res.getStatus() == ReservationStatus::Expired);

    std::cout << "[PASS] testReservationRevisionAndExpiration\n";
}

void testReservationCancellationGuards() {
    std::cout << "[RUN] testReservationCancellationGuards\n";
    ReservationStateMachine sm;
    NotificationService::getInstance().clear();

    std::time_t now = makeLocalTime(2026, 9, 1, 9, 0);
    std::time_t eventStart = makeLocalTime(2026, 9, 5, 14, 0); // 48+ hours away
    std::time_t eventEnd = makeLocalTime(2026, 9, 5, 16, 0);

    // Cancellation from Submitted
    Reservation res1("res-401", "user-1", "room-1", "Test 1", "Meeting", 20, eventStart, eventEnd, now);
    assert(sm.submit(res1, true, now).success == true);
    assert(sm.cancel(res1, "user-1", now + 60).success == true);
    assert(res1.getStatus() == ReservationStatus::Cancelled);

    // Cancellation from Approved with > 24 hours lead time
    Reservation res2("res-402", "user-2", "room-2", "Test 2", "Meeting", 20, eventStart, eventEnd, now);
    assert(sm.submit(res2, true, now).success == true);
    assert(sm.routeToReview(res2, now + 60).success == true);
    assert(sm.approve(res2, "approver-1", "OK", now + 120).success == true);
    assert(res2.getStatus() == ReservationStatus::Approved);

    // At 25 hours before start: can cancel
    std::time_t t25HoursBefore = eventStart - 25 * 3600;
    assert(sm.cancel(res2, "user-2", t25HoursBefore).success == true);
    assert(res2.getStatus() == ReservationStatus::Cancelled);

    // Cancellation from Approved with < 24 hours lead time: MUST FAIL!
    Reservation res3("res-403", "user-3", "room-3", "Test 3", "Meeting", 20, eventStart, eventEnd, now);
    assert(sm.submit(res3, true, now).success == true);
    assert(sm.routeToReview(res3, now + 60).success == true);
    assert(sm.approve(res3, "approver-1", "OK", now + 120).success == true);

    std::time_t t23HoursBefore = eventStart - 23 * 3600;
    TransitionResult rLateCancel = sm.cancel(res3, "user-3", t23HoursBefore);
    assert(rLateCancel.success == false);
    assert(res3.getStatus() == ReservationStatus::Approved); // Kept in Approved

    std::cout << "[PASS] testReservationCancellationGuards\n";
}

void testReservationHoldBufferExpiry() {
    std::cout << "[RUN] testReservationHoldBufferExpiry\n";
    ReservationStateMachine sm;

    std::time_t now = makeLocalTime(2026, 9, 1, 9, 0);
    std::time_t eventStart = makeLocalTime(2026, 9, 5, 14, 0);
    std::time_t eventEnd = makeLocalTime(2026, 9, 5, 16, 0);
    Reservation res("res-501", "user-1", "room-1", "Hold Test", "Meeting", 20, eventStart, eventEnd, now);
    assert(sm.submit(res, true, now).success == true);
    assert(res.getHoldExpiresAt() == now + 900);

    // Attempt routing after 15 minutes (901 seconds)
    TransitionResult rExpiredRoute = sm.routeToReview(res, now + 901);
    assert(rExpiredRoute.success == false);
    assert(res.getStatus() == ReservationStatus::Submitted);

    // Expire soft hold
    TransitionResult rExpire = sm.expire(res, now + 901);
    assert(rExpire.success == true);
    assert(res.getStatus() == ReservationStatus::Expired);

    std::cout << "[PASS] testReservationHoldBufferExpiry\n";
}

void testReservationCanTransition() {
    std::cout << "[RUN] testReservationCanTransition\n";
    ReservationStateMachine sm;

    // Self transitions are false
    assert(sm.canTransition(ReservationStatus::Draft, ReservationStatus::Draft) == false);

    // Valid forward transitions
    assert(sm.canTransition(ReservationStatus::Draft, ReservationStatus::Submitted) == true);
    assert(sm.canTransition(ReservationStatus::Submitted, ReservationStatus::UnderReview) == true);
    assert(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::SecurityReview) == true);
    assert(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::Approved) == true);
    assert(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::NeedsRevision) == true);
    assert(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::Rejected) == true);
    assert(sm.canTransition(ReservationStatus::SecurityReview, ReservationStatus::Approved) == true);
    assert(sm.canTransition(ReservationStatus::SecurityReview, ReservationStatus::Rejected) == true);
    assert(sm.canTransition(ReservationStatus::NeedsRevision, ReservationStatus::Submitted) == true);
    assert(sm.canTransition(ReservationStatus::NeedsRevision, ReservationStatus::Expired) == true);
    assert(sm.canTransition(ReservationStatus::Approved, ReservationStatus::InProgress) == true);
    assert(sm.canTransition(ReservationStatus::Approved, ReservationStatus::Cancelled) == true);
    assert(sm.canTransition(ReservationStatus::InProgress, ReservationStatus::Completed) == true);

    // Invalid backwards or jump transitions
    assert(sm.canTransition(ReservationStatus::Draft, ReservationStatus::Approved) == false);
    assert(sm.canTransition(ReservationStatus::Completed, ReservationStatus::InProgress) == false);
    assert(sm.canTransition(ReservationStatus::Cancelled, ReservationStatus::Draft) == false);
    assert(sm.canTransition(ReservationStatus::Rejected, ReservationStatus::UnderReview) == false);
    assert(sm.canTransition(ReservationStatus::Expired, ReservationStatus::Submitted) == false);

    std::cout << "[PASS] testReservationCanTransition\n";
}

void testFacilityCyclicOperationalCycle() {
    std::cout << "[RUN] testFacilityCyclicOperationalCycle\n";
    FacilityStateMachine fsm;
    NotificationService::getInstance().clear();

    Facility fac("aud-100", "Main Auditorium", "Campus Center", 500);

    // 1. Commission room -> Available
    FacilityTransitionResult rComm = fsm.commissionRoom(fac);
    assert(rComm.success == true);
    assert(fac.getStatus() == RoomStatus::Available);
    assert(fac.areDoorsLocked() == false);

    // 2. Reservation Confirmed -> Reserved
    FacilityTransitionResult rConf = fsm.reservationConfirmed(fac);
    assert(rConf.success == true);
    assert(fac.getStatus() == RoomStatus::Reserved);

    // 3. Setup Window Opens -> Setup
    FacilityTransitionResult rSetup = fsm.setupWindowOpens(fac);
    assert(rSetup.success == true);
    assert(fac.getStatus() == RoomStatus::Setup);

    // 4. Event Check-in -> Occupied (smart doors unlocked)
    FacilityTransitionResult rCheckInFail = fsm.eventCheckIn(fac, false);
    assert(rCheckInFail.success == false); // Guard failure without custodial clearance

    FacilityTransitionResult rCheckIn = fsm.eventCheckIn(fac, true);
    assert(rCheckIn.success == true);
    assert(fac.getStatus() == RoomStatus::Occupied);
    assert(fac.areDoorsLocked() == false);

    // 5. Checkout Concluded -> Turnover (doors locked, custodial notice sent)
    FacilityTransitionResult rCheckout = fsm.checkoutConcluded(fac);
    assert(rCheckout.success == true);
    assert(fac.getStatus() == RoomStatus::Turnover);
    assert(fac.areDoorsLocked() == true);
    assert(NotificationService::getInstance().count() >= 1);

    // 6. Inspection Passed -> Available (doors unlocked)
    FacilityTransitionResult rInspFail = fsm.inspectionPassed(fac, false);
    assert(rInspFail.success == false); // Guard failure

    FacilityTransitionResult rInsp = fsm.inspectionPassed(fac, true);
    assert(rInsp.success == true);
    assert(fac.getStatus() == RoomStatus::Available);
    assert(fac.areDoorsLocked() == false);

    // 7. Back-to-back fast-track: Reserved -> Setup -> Occupied -> Turnover -> Setup
    assert(fsm.reservationConfirmed(fac).success == true);
    assert(fsm.setupWindowOpens(fac).success == true);
    assert(fsm.eventCheckIn(fac).success == true);
    assert(fsm.checkoutConcluded(fac).success == true);

    FacilityTransitionResult rB2B = fsm.backToBackBooking(fac);
    assert(rB2B.success == true);
    assert(fac.getStatus() == RoomStatus::Setup);

    std::cout << "[PASS] testFacilityCyclicOperationalCycle\n";
}

void testFacilityDamageAndMaintenance() {
    std::cout << "[RUN] testFacilityDamageAndMaintenance\n";
    FacilityStateMachine fsm;
    NotificationService::getInstance().clear();

    Facility fac("lab-101", "Robotics Lab", "Engineering Hall", 40);

    // Report damage from Available -> OutOfService
    assert(fac.getStatus() == RoomStatus::Available);
    FacilityTransitionResult rDamage = fsm.reportDamage(fac, "Broken hydraulic pipe");
    assert(rDamage.success == true);
    assert(fac.getStatus() == RoomStatus::OutOfService);
    assert(fac.areDoorsLocked() == true);

    // Repair certified with failure
    FacilityTransitionResult rRepairFail = fsm.repairCertified(fac, false);
    assert(rRepairFail.success == false);
    assert(fac.getStatus() == RoomStatus::OutOfService);

    // Repair certified with success -> Available
    FacilityTransitionResult rRepairPass = fsm.repairCertified(fac, true);
    assert(rRepairPass.success == true);
    assert(fac.getStatus() == RoomStatus::Available);
    assert(fac.areDoorsLocked() == false);

    // Report damage from Setup -> OutOfService
    assert(fsm.reservationConfirmed(fac).success == true);
    assert(fsm.setupWindowOpens(fac).success == true);
    assert(fsm.reportDamage(fac, "Projector power surge").success == true);
    assert(fac.getStatus() == RoomStatus::OutOfService);
    assert(fsm.repairCertified(fac, true).success == true);
    assert(fac.getStatus() == RoomStatus::Available);

    std::cout << "[PASS] testFacilityDamageAndMaintenance\n";
}

void testFacilityEmergencyPreemption() {
    std::cout << "[RUN] testFacilityEmergencyPreemption\n";
    FacilityStateMachine fsm;
    NotificationService::getInstance().clear();

    Facility fac("hall-200", "Great Hall", "Union", 300);

    // Walk into Occupied state
    assert(fsm.reservationConfirmed(fac).success == true);
    assert(fsm.setupWindowOpens(fac).success == true);
    assert(fsm.eventCheckIn(fac).success == true);
    assert(fac.getStatus() == RoomStatus::Occupied);
    assert(fac.areDoorsLocked() == false);

    // Emergency declared from Occupied state -> EmergencyHold (doors locked, security alert dispatched)
    FacilityTransitionResult rEmerg = fsm.declareEmergency(fac, "Severe weather tornado siren activated");
    assert(rEmerg.success == true);
    assert(fac.getStatus() == RoomStatus::EmergencyHold);
    assert(fac.areDoorsLocked() == true);
    assert(NotificationService::getInstance().count() >= 1);

    // All clear without security approval fails
    FacilityTransitionResult rClearFail = fsm.allClearGiven(fac, false);
    assert(rClearFail.success == false);
    assert(fac.getStatus() == RoomStatus::EmergencyHold);

    // All clear with security approval -> Available
    FacilityTransitionResult rClearPass = fsm.allClearGiven(fac, true);
    assert(rClearPass.success == true);
    assert(fac.getStatus() == RoomStatus::Available);
    assert(fac.areDoorsLocked() == false);

    std::cout << "[PASS] testFacilityEmergencyPreemption\n";
}

void testFacilityCanTransition() {
    std::cout << "[RUN] testFacilityCanTransition\n";
    FacilityStateMachine fsm;

    assert(fsm.canTransition(RoomStatus::Available, RoomStatus::Reserved) == true);
    assert(fsm.canTransition(RoomStatus::Reserved, RoomStatus::Setup) == true);
    assert(fsm.canTransition(RoomStatus::Setup, RoomStatus::Occupied) == true);
    assert(fsm.canTransition(RoomStatus::Occupied, RoomStatus::Turnover) == true);
    assert(fsm.canTransition(RoomStatus::Turnover, RoomStatus::Available) == true);
    assert(fsm.canTransition(RoomStatus::Turnover, RoomStatus::Setup) == true); // B2B
    assert(fsm.canTransition(RoomStatus::Turnover, RoomStatus::OutOfService) == true);
    assert(fsm.canTransition(RoomStatus::OutOfService, RoomStatus::Available) == true);

    // Emergency preemption from ANY state is valid
    assert(fsm.canTransition(RoomStatus::Available, RoomStatus::EmergencyHold) == true);
    assert(fsm.canTransition(RoomStatus::Reserved, RoomStatus::EmergencyHold) == true);
    assert(fsm.canTransition(RoomStatus::Setup, RoomStatus::EmergencyHold) == true);
    assert(fsm.canTransition(RoomStatus::Occupied, RoomStatus::EmergencyHold) == true);
    assert(fsm.canTransition(RoomStatus::Turnover, RoomStatus::EmergencyHold) == true);
    assert(fsm.canTransition(RoomStatus::OutOfService, RoomStatus::EmergencyHold) == true);
    assert(fsm.canTransition(RoomStatus::EmergencyHold, RoomStatus::Available) == true);

    // Invalid transitions
    assert(fsm.canTransition(RoomStatus::Available, RoomStatus::Occupied) == false);
    assert(fsm.canTransition(RoomStatus::Occupied, RoomStatus::Available) == false);
    assert(fsm.canTransition(RoomStatus::Available, RoomStatus::Available) == false);

    std::cout << "[PASS] testFacilityCanTransition\n";
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Starting State Machine Engine Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testReservationStandardHappyPath();
    testReservationSecurityWorkflow();
    testReservationRevisionAndExpiration();
    testReservationCancellationGuards();
    testReservationHoldBufferExpiry();
    testReservationCanTransition();

    testFacilityCyclicOperationalCycle();
    testFacilityDamageAndMaintenance();
    testFacilityEmergencyPreemption();
    testFacilityCanTransition();

    std::cout << "========================================" << std::endl;
    std::cout << "All State Machine Tests Passed (10/10)!" << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
