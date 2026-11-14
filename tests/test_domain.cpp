#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include "domain/User.hpp"
#include "domain/Facility.hpp"
#include "domain/NotificationService.hpp"
#include "domain/Reservation.hpp"

using namespace domain;

void testUserHierarchy() {
    StudentOrgRepresentative student("u-01", "Alice Student", "alice@campus.edu", "Computer Science", "ACM Chapter");
    assert(student.getRole() == UserRole::StudentOrgRep);
    assert(student.getRoleName() == "Student Organization Representative");
    assert(student.canSubmit() == true);
    assert(student.canApprove() == false);
    assert(student.canCoordinate() == false);
    assert(student.isExpeditedEligible() == false);
    assert(student.getOrganizationName() == "ACM Chapter");

    FacultyStaffMember faculty("u-02", "Prof. Bob Smith", "bsmith@campus.edu", "Physics", "EMP-9876");
    assert(faculty.getRole() == UserRole::FacultyStaff);
    assert(faculty.canSubmit() == true);
    assert(faculty.isExpeditedEligible() == true);
    assert(faculty.getEmployeeId() == "EMP-9876");

    FacilityCoordinator coord("u-03", "Carol Coordinator", "carol@campus.edu", "Facilities", {"Student Union", "Engineering Hall"});
    assert(coord.getRole() == UserRole::FacilityCoord);
    assert(coord.canCoordinate() == true);
    assert(coord.managesBuilding("Student Union") == true);
    assert(coord.managesBuilding("Library") == false);

    EventApprover approver("u-04", "Dean Dave", "dean@campus.edu", "Student Affairs", 2);
    assert(approver.getRole() == UserRole::EventAppr);
    assert(approver.canApprove() == true);
    assert(approver.getApprovalTier() == 2);

    SecurityOfficer security("u-05", "Officer Eve", "officer@campus.edu", "Police", "BADGE-42");
    assert(security.getRole() == UserRole::SecurityOfficer);
    assert(security.isSecurityOfficer() == true);
    assert(security.getBadgeNumber() == "BADGE-42");

    Administrator admin("u-06", "Super Admin", "admin@campus.edu");
    assert(admin.getRole() == UserRole::Admin);
    assert(admin.isAdmin() == true);
    assert(admin.canSubmit() == true);
    assert(admin.canApprove() == true);
    assert(admin.canCoordinate() == true);

    // Test polymorphism
    std::vector<std::unique_ptr<User>> users;
    users.push_back(std::make_unique<StudentOrgRepresentative>(student));
    users.push_back(std::make_unique<FacultyStaffMember>(faculty));
    users.push_back(std::make_unique<FacilityCoordinator>(coord));
    users.push_back(std::make_unique<EventApprover>(approver));
    assert(users[0]->getRole() == UserRole::StudentOrgRep);
    assert(users[1]->getRole() == UserRole::FacultyStaff);
    assert(users[2]->getRole() == UserRole::FacilityCoord);
    assert(users[3]->getRole() == UserRole::EventAppr);

    // Enums
    assert(userRoleToString(UserRole::StudentOrgRep) == "StudentOrgRep");
    assert(stringToUserRole("FacultyStaff") == UserRole::FacultyStaff);
    assert(stringToUserRole("EventApprover") == UserRole::EventAppr);

    std::cout << "[PASS] testUserHierarchy\n";
}

void testFacility() {
    Facility room("fac-101", "Auditorium A", "Student Union", 250, 45.0, RoomStatus::Available, {"Projector", "Microphones"});
    assert(room.getFacilityId() == "fac-101");
    assert(room.getCapacity() == 250);
    assert(room.getStatus() == RoomStatus::Available);
    assert(room.hasEquipment("Projector") == true);
    assert(room.hasEquipment("Stage Lighting") == false);

    room.addSupportedEquipment("Stage Lighting");
    assert(room.hasEquipment("Stage Lighting") == true);

    // Availability checks
    std::time_t t1 = 1700000000;
    std::time_t t2 = 1700003600;
    assert(room.checkAvailability(t1, t2) == true);

    // Set blackout
    room.setBlackout(t1 - 100, t1 + 500, "Floor Maintenance");
    assert(room.checkAvailability(t1, t2) == false);
    assert(room.checkAvailability(t1 + 600, t2 + 600) == true);

    // Status impact
    room.clearBlackouts();
    assert(room.checkAvailability(t1, t2) == true);
    room.setStatus(RoomStatus::OutOfService);
    assert(room.checkAvailability(t1, t2) == false);
    room.setStatus(RoomStatus::EmergencyHold);
    assert(room.checkAvailability(t1, t2) == false);
    room.setStatus(RoomStatus::Available);
    assert(room.checkAvailability(t1, t2) == true);

    // Enum strings
    assert(roomStatusToString(RoomStatus::Turnover) == "Turnover");
    assert(stringToRoomStatus("Occupied") == RoomStatus::Occupied);

    std::cout << "[PASS] testFacility\n";
}

void testNotificationService() {
    auto& svc = NotificationService::getInstance();
    svc.clear();
    assert(svc.count() == 0);

    svc.sendConfirmation("user@campus.edu", "res-123", "Room confirmed");
    svc.sendSecurityAlert("res-124", "High attendance alert");
    svc.sendCustodialNotice("fac-101", "Event completed, turnover needed");

    assert(svc.count() == 3);
    auto recent = svc.getRecent(2);
    assert(recent.size() == 2);
    assert(recent[1].recipientEmail == "custodial-facilities@campus.edu");

    std::cout << "[PASS] testNotificationService\n";
}

void testReservation() {
    // Normal daytime timestamp setup
    // 2026-11-14 10:00:00 UTC (or local)
    std::tm tmStart{};
    tmStart.tm_year = 2026 - 1900;
    tmStart.tm_mon = 10; // November
    tmStart.tm_mday = 14;
    tmStart.tm_hour = 10;
    tmStart.tm_min = 0;
    tmStart.tm_sec = 0;
    std::time_t start = std::mktime(&tmStart);
    std::time_t end = start + 7200; // 2 hours later (12:00)

    Reservation res("res-500", "u-01", "fac-101", "Hackathon Opening", "Workshop", 80, start, end);
    assert(res.getReservationId() == "res-500");
    assert(res.getStatus() == ReservationStatus::Draft);
    assert(res.getRequiresSecurity() == false); // headcount <= 100 and daytime

    // Large attendance triggering security
    res.setExpectedAttendance(150);
    assert(res.getRequiresSecurity() == true);

    // After hours triggering security even with headcount < 100
    std::tm tmNight{};
    tmNight.tm_year = 2026 - 1900;
    tmNight.tm_mon = 10;
    tmNight.tm_mday = 14;
    tmNight.tm_hour = 21; // 9 PM
    tmNight.tm_min = 0;
    std::time_t nightStart = std::mktime(&tmNight);
    std::time_t nightEnd = nightStart + 3600;

    Reservation nightRes("res-501", "u-01", "fac-101", "Late Night LAN", "Social", 30, nightStart, nightEnd);
    assert(nightRes.getRequiresSecurity() == true);

    // Composition items
    res.addEquipment("PA System", 2);
    assert(res.getEquipmentRequests().size() == 1);
    assert(res.getEquipmentRequests()[0].getEquipmentType() == "PA System");
    assert(res.getEquipmentRequests()[0].getQuantity() == 2);

    res.addSupportRequest("AV Support", "Need technician for stage setup");
    assert(res.getSupportRequests().size() == 1);
    assert(res.getSupportRequests()[0].getSupportType() == "AV Support");

    res.recordApproval("dean-01", "Approved", "Standard review passed");
    assert(res.getApprovals().size() == 1);
    assert(res.getApprovals()[0].getDecision() == "Approved");

    // Status transitions
    res.setStatus(ReservationStatus::Submitted);
    assert(res.getStatus() == ReservationStatus::Submitted);
    assert(reservationStatusToString(res.getStatus()) == "Submitted");
    assert(stringToReservationStatus("Approved") == ReservationStatus::Approved);

    std::cout << "[PASS] testReservation\n";
}

int main() {
    std::cout << "Starting Domain Unit Tests...\n";
    testUserHierarchy();
    testFacility();
    testNotificationService();
    testReservation();
    std::cout << "All Domain Unit Tests Passed Successfully!\n";
    return 0;
}
