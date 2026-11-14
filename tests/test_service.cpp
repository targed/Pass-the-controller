#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <ctime>

#include "service/BookingSystem.hpp"
#include "service/ScenarioRunner.hpp"

using namespace service;
using namespace domain;
using namespace state;

void testDefaultSeeding() {
    std::cout << "[RUN] testDefaultSeeding\n";
    BookingSystem sys;

    // 4 Facilities
    auto facilities = sys.getFacilities();
    assert(facilities.size() == 4);

    auto f101 = sys.getFacility("FAC-101");
    assert(f101.has_value());
    assert(f101->getName() == "Centennial Auditorium");
    assert(f101->getCapacity() == 250);
    assert(f101->getStatus() == RoomStatus::Available);

    auto f102 = sys.getFacility("FAC-102");
    assert(f102.has_value());
    assert(f102->getName() == "STEM Lab");
    assert(f102->getCapacity() == 40);

    auto f103 = sys.getFacility("FAC-103");
    assert(f103.has_value());
    assert(f103->getName() == "Student Union Ballroom");
    assert(f103->getCapacity() == 400);

    auto f104 = sys.getFacility("FAC-104");
    assert(f104.has_value());
    assert(f104->getName() == "Seminar Room");
    assert(f104->getCapacity() == 25);

    // 6 Users
    auto users = sys.getUsers();
    assert(users.size() >= 6);

    auto u1 = sys.getUser("USR-001");
    assert(u1 != nullptr);
    assert(u1->getRole() == UserRole::StudentOrgRep);

    auto u2 = sys.getUser("USR-002");
    assert(u2 != nullptr);
    assert(u2->getRole() == UserRole::StudentOrgRep);

    auto u3 = sys.getUser("USR-003");
    assert(u3 != nullptr);
    assert(u3->getRole() == UserRole::FacultyStaff);

    auto u4 = sys.getUser("USR-004");
    assert(u4 != nullptr);
    assert(u4->getRole() == UserRole::FacilityCoord);

    auto u5 = sys.getUser("USR-005");
    assert(u5 != nullptr);
    assert(u5->getRole() == UserRole::EventAppr);

    auto u6 = sys.getUser("USR-006");
    assert(u6 != nullptr);
    assert(u6->getRole() == UserRole::SecurityOfficer);

    std::cout << "[PASS] testDefaultSeeding\n";
}

void testSimulatedClock() {
    std::cout << "[RUN] testSimulatedClock\n";
    BookingSystem sys;
    std::time_t t0 = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
    sys.setTime(t0);
    assert(sys.getCurrentTime() == t0);

    sys.advanceTime(3600); // 1 hour
    assert(sys.getCurrentTime() == t0 + 3600);

    std::string formatted = BookingSystem::formatTime(sys.getCurrentTime());
    assert(formatted == "2026-09-01 10:00:00");

    std::cout << "[PASS] testSimulatedClock\n";
}

void testBookingOperations() {
    std::cout << "[RUN] testBookingOperations\n";
    BookingSystem sys;
    std::time_t base = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
    std::time_t start = BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
    std::time_t end   = BookingSystem::makeTime(2026, 9, 5, 16, 0, 0);
    sys.setTime(base);

    // Create
    std::string id = sys.createReservation("USR-001", "FAC-104", "Robotics Meet", "Meeting", 20, start, end);
    assert(!id.empty());
    auto rOpt = sys.getReservation(id);
    assert(rOpt.has_value());
    assert(rOpt->getStatus() == ReservationStatus::Draft);

    // Submit
    auto rSub = sys.submitReservation(id);
    assert(rSub.success);
    assert(sys.getReservation(id)->getStatus() == ReservationStatus::Submitted);

    // Route to Review
    auto rRoute = sys.routeToReview(id);
    assert(rRoute.success);
    assert(sys.getReservation(id)->getStatus() == ReservationStatus::UnderReview);
    assert(sys.getPendingApprovals().size() == 1);

    // Approve
    auto rApp = sys.approveReservation(id, "USR-005", "Approved by Dean");
    assert(rApp.success);
    assert(sys.getReservation(id)->getStatus() == ReservationStatus::Approved);
    assert(sys.getPendingApprovals().empty());
    assert(sys.getFacility("FAC-104")->getStatus() == RoomStatus::Reserved);

    // CheckIn at start
    sys.setTime(start);
    auto rIn = sys.checkInReservation(id);
    assert(rIn.success);
    assert(sys.getReservation(id)->getStatus() == ReservationStatus::InProgress);
    assert(sys.getFacility("FAC-104")->getStatus() == RoomStatus::Occupied);
    assert(!sys.getFacility("FAC-104")->areDoorsLocked());

    // Conclude at end
    sys.setTime(end);
    auto rDone = sys.concludeReservation(id);
    assert(rDone.success);
    assert(sys.getReservation(id)->getStatus() == ReservationStatus::Completed);
    assert(sys.getFacility("FAC-104")->getStatus() == RoomStatus::Turnover);
    assert(sys.getFacility("FAC-104")->areDoorsLocked());

    // Custodial Inspection
    auto rInsp = sys.completeInspection("FAC-104");
    assert(rInsp.success);
    assert(sys.getFacility("FAC-104")->getStatus() == RoomStatus::Available);

    std::cout << "[PASS] testBookingOperations\n";
}

void testEmergencyLockdownOperations() {
    std::cout << "[RUN] testEmergencyLockdownOperations\n";
    BookingSystem sys;

    auto rEmerg = sys.declareCampusEmergency("Drill lockdown");
    assert(rEmerg.success);
    for (const auto& fac : sys.getFacilities()) {
        assert(fac.getStatus() == RoomStatus::EmergencyHold);
        assert(fac.areDoorsLocked());
    }

    auto rClear = sys.clearCampusEmergency();
    assert(rClear.success);
    for (const auto& fac : sys.getFacilities()) {
        assert(fac.getStatus() == RoomStatus::Available);
        assert(!fac.areDoorsLocked());
    }

    std::cout << "[PASS] testEmergencyLockdownOperations\n";
}

void testJsonSerialization() {
    std::cout << "[RUN] testJsonSerialization\n";
    BookingSystem sys;
    std::string json = sys.toJson();
    assert(json.find("\"facilities\":") != std::string::npos);
    assert(json.find("\"Centennial Auditorium\"") != std::string::npos);
    assert(json.find("\"users\":") != std::string::npos);
    assert(json.find("\"USR-001\"") != std::string::npos);
    assert(json.find("\"auditLog\":") != std::string::npos);

    std::cout << "[PASS] testJsonSerialization\n";
}

void testScenarioRunnerExecutions() {
    std::cout << "[RUN] testScenarioRunnerExecutions\n";
    ScenarioRunner runner(std::cout, false); // Quiet mode for unit test

    assert(runner.runHappyPath());
    assert(runner.runLargeEventSecurityScenario());
    assert(runner.runRevisionLoopScenario());
    assert(runner.runFacilityLogisticsScenario());
    assert(runner.runEmergencyLockdownScenario());
    assert(runner.runCancellationRulesScenario());
    assert(runner.runAll());

    std::cout << "[PASS] testScenarioRunnerExecutions\n";
}

void testNeedsRevisionConflictAndDeduplication() {
    std::cout << "[RUN] testNeedsRevisionConflictAndDeduplication\n";
    BookingSystem sys;
    std::time_t base = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
    std::time_t start = BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
    std::time_t end   = BookingSystem::makeTime(2026, 9, 5, 16, 0, 0);
    sys.setTime(base);

    // 1. Test deduplication on createReservation (overload 1)
    std::string id1 = sys.createReservation("USR-001", "FAC-104", "Robotics Meet", "Meeting", 20, start, end, {}, {}, "CUSTOM-1");
    assert(id1 == "CUSTOM-1");
    size_t count1 = sys.getReservations().size();

    // Re-create / overwrite with same custom ID
    std::string id2 = sys.createReservation("USR-001", "FAC-104", "Robotics Meet Updated", "Meeting", 20, start, end, {}, {}, "CUSTOM-1");
    assert(id2 == "CUSTOM-1");
    assert(sys.getReservations().size() == count1); // Order vector not duplicated

    // Overload 2 deduplication test
    domain::Reservation resManual("CUSTOM-2", "USR-001", "FAC-104", "Study", "Meeting", 5, start, end);
    sys.createReservation(resManual);
    size_t count2 = sys.getReservations().size();
    sys.createReservation(resManual);
    assert(sys.getReservations().size() == count2);

    // 2. Test NeedsRevision conflict blocking
    // Move CUSTOM-1 to NeedsRevision
    assert(sys.submitReservation(id1).success);
    assert(sys.routeToReview(id1).success);
    assert(sys.requestChanges(id1, "USR-005", "Please add a description", base + 86400).success);
    assert(sys.getReservation(id1)->getStatus() == ReservationStatus::NeedsRevision);

    // Try submitting an overlapping reservation for FAC-104
    std::string overlapId = sys.createReservation("USR-002", "FAC-104", "Overlapping Event", "Meeting", 15, start, end);
    auto submitOverlap = sys.submitReservation(overlapId);
    assert(!submitOverlap.success); // Must fail due to conflict with NeedsRevision
    assert(sys.getReservation(overlapId)->getStatus() == ReservationStatus::Draft);

    std::cout << "[PASS] testNeedsRevisionConflictAndDeduplication\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Starting Service & Scenario Unit Tests\n";
    std::cout << "========================================\n";

    testDefaultSeeding();
    testSimulatedClock();
    testBookingOperations();
    testEmergencyLockdownOperations();
    testNeedsRevisionConflictAndDeduplication();
    testJsonSerialization();
    testScenarioRunnerExecutions();

    std::cout << "========================================\n";
    std::cout << "All Service Tests Passed (7/7)!\n";
    std::cout << "========================================\n";

    // Now execute verbose scenario output to verify aesthetic terminal formatting
    std::cout << "\nExecuting Verbose Scenario Suite Demonstration:\n";
    ScenarioRunner verboseRunner(std::cout, true);
    bool allPass = verboseRunner.runAll();
    assert(allPass);

    return 0;
}
