#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <chrono>
#include <atomic>
#include <thread>
#include <type_traits>
#include <stdexcept>
#include <iomanip>
#include <ctime>
#include <cstdint>

#include "domain/User.hpp"
#include "domain/Facility.hpp"
#include "domain/Reservation.hpp"
#include "domain/NotificationService.hpp"
#include "state/ReservationStateMachine.hpp"
#include "state/FacilityStateMachine.hpp"
#include "service/BookingSystem.hpp"
#include "web/HttpServer.hpp"

namespace smartcampus::test {

// =============================================================================
// ANSI Terminal Styling
// =============================================================================
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

// =============================================================================
// Test Assertion Exception & Tracking Harness
// =============================================================================

class TestAssertionException : public std::runtime_error {
public:
    TestAssertionException(std::string msg, const char* file, int line)
        : std::runtime_error(std::move(msg)), file_(file), line_(line) {}

    const char* getFile() const noexcept { return file_; }
    int getLine() const noexcept { return line_; }

private:
    const char* file_;
    int line_;
};

struct AssertionTracker {
    static inline std::atomic<uint64_t> assertionCount{0};
    static inline std::atomic<uint64_t> assertionPassCount{0};

    static void reset() noexcept {
        assertionCount.store(0, std::memory_order_relaxed);
        assertionPassCount.store(0, std::memory_order_relaxed);
    }
};

// Stream concept for formatting values safely
template <typename T>
concept Streamable = requires(std::ostream& os, const T& t) {
    { os << t } -> std::same_as<std::ostream&>;
};

template <typename T>
std::string formatVal(const T& val) {
    using DecayedT = std::decay_t<T>;
    if constexpr (std::is_enum_v<DecayedT>) {
        if constexpr (std::is_same_v<DecayedT, ::domain::UserRole>) {
            return ::domain::userRoleToString(val);
        } else if constexpr (std::is_same_v<DecayedT, ::domain::RoomStatus>) {
            return ::domain::roomStatusToString(val);
        } else if constexpr (std::is_same_v<DecayedT, ::domain::ReservationStatus>) {
            return ::domain::reservationStatusToString(val);
        } else {
            return std::to_string(static_cast<std::underlying_type_t<DecayedT>>(val));
        }
    } else if constexpr (std::is_same_v<DecayedT, bool>) {
        return val ? "true" : "false";
    } else if constexpr (std::is_same_v<DecayedT, std::string>) {
        return "\"" + val + "\"";
    } else if constexpr (std::is_same_v<DecayedT, std::string_view>) {
        return "\"" + std::string(val) + "\"";
    } else if constexpr (std::is_same_v<DecayedT, const char*>) {
        return val ? (std::string("\"") + val + "\"") : "nullptr";
    } else if constexpr (Streamable<DecayedT>) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    } else {
        return "<unprintable>";
    }
}

// =============================================================================
// Assertion Macros
// =============================================================================

#ifndef ASSERT_TRUE
#define ASSERT_TRUE(expr) \
    do { \
        ::smartcampus::test::AssertionTracker::assertionCount.fetch_add(1, std::memory_order_relaxed); \
        if (!(expr)) { \
            throw ::smartcampus::test::TestAssertionException( \
                std::string("ASSERT_TRUE failed: (") + #expr + ") is false", __FILE__, __LINE__); \
        } \
        ::smartcampus::test::AssertionTracker::assertionPassCount.fetch_add(1, std::memory_order_relaxed); \
    } while (false)
#endif

#ifndef ASSERT_FALSE
#define ASSERT_FALSE(expr) \
    do { \
        ::smartcampus::test::AssertionTracker::assertionCount.fetch_add(1, std::memory_order_relaxed); \
        if (expr) { \
            throw ::smartcampus::test::TestAssertionException( \
                std::string("ASSERT_FALSE failed: (") + #expr + ") is true", __FILE__, __LINE__); \
        } \
        ::smartcampus::test::AssertionTracker::assertionPassCount.fetch_add(1, std::memory_order_relaxed); \
    } while (false)
#endif

#ifndef ASSERT_EQ
#define ASSERT_EQ(val1, val2) \
    do { \
        ::smartcampus::test::AssertionTracker::assertionCount.fetch_add(1, std::memory_order_relaxed); \
        auto&& _v1 = (val1); \
        auto&& _v2 = (val2); \
        if (!(_v1 == _v2)) { \
            std::string _msg = std::string("ASSERT_EQ failed: (") + #val1 + " == " + #val2 + ") [" + \
                               ::smartcampus::test::formatVal(_v1) + " != " + \
                               ::smartcampus::test::formatVal(_v2) + "]"; \
            throw ::smartcampus::test::TestAssertionException(_msg, __FILE__, __LINE__); \
        } \
        ::smartcampus::test::AssertionTracker::assertionPassCount.fetch_add(1, std::memory_order_relaxed); \
    } while (false)
#endif

#ifndef ASSERT_NE
#define ASSERT_NE(val1, val2) \
    do { \
        ::smartcampus::test::AssertionTracker::assertionCount.fetch_add(1, std::memory_order_relaxed); \
        auto&& _v1 = (val1); \
        auto&& _v2 = (val2); \
        if (_v1 == _v2) { \
            std::string _msg = std::string("ASSERT_NE failed: (") + #val1 + " != " + #val2 + ") [both equal " + \
                               ::smartcampus::test::formatVal(_v1) + "]"; \
            throw ::smartcampus::test::TestAssertionException(_msg, __FILE__, __LINE__); \
        } \
        ::smartcampus::test::AssertionTracker::assertionPassCount.fetch_add(1, std::memory_order_relaxed); \
    } while (false)
#endif

#ifndef ASSERT_THROWS
#define ASSERT_THROWS(expr, ExceptionType) \
    do { \
        ::smartcampus::test::AssertionTracker::assertionCount.fetch_add(1, std::memory_order_relaxed); \
        bool _threwExpected = false; \
        try { \
            (expr); \
        } catch (const ExceptionType&) { \
            _threwExpected = true; \
        } catch (const std::exception& _e) { \
            throw ::smartcampus::test::TestAssertionException( \
                std::string("ASSERT_THROWS failed: (") + #expr + ") threw an unexpected exception type (expected " + #ExceptionType + ", caught: " + _e.what() + ")", __FILE__, __LINE__); \
        } catch (...) { \
            throw ::smartcampus::test::TestAssertionException( \
                std::string("ASSERT_THROWS failed: (") + #expr + ") threw an unknown exception type (expected " + #ExceptionType + ")", __FILE__, __LINE__); \
        } \
        if (!_threwExpected) { \
            throw ::smartcampus::test::TestAssertionException( \
                std::string("ASSERT_THROWS failed: (") + #expr + ") did not throw expected exception " + #ExceptionType, __FILE__, __LINE__); \
        } \
        ::smartcampus::test::AssertionTracker::assertionPassCount.fetch_add(1, std::memory_order_relaxed); \
    } while (false)
#endif

// =============================================================================
// TestSuite Class
// =============================================================================

class TestSuite {
public:
    TestSuite() = default;

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

    // -------------------------------------------------------------------------
    // 1. User Hierarchy, Polymorphism, and Access Control
    // -------------------------------------------------------------------------
    void testUserHierarchyAndPolymorphism() {
        testAssertionMacros();
        using namespace ::domain;

        // Student Organization Representative
        StudentOrgRepresentative student("u-s01", "Alice Chen", "alice@campus.edu", "Computer Science", "ACM Chapter", "pass123");
        ASSERT_EQ(student.getRole(), UserRole::StudentOrgRep);
        ASSERT_EQ(student.getRoleName(), "Student Organization Representative");
        ASSERT_TRUE(student.canSubmit());
        ASSERT_FALSE(student.canApprove());
        ASSERT_FALSE(student.canCoordinate());
        ASSERT_FALSE(student.isExpeditedEligible());
        ASSERT_FALSE(student.isSecurityOfficer());
        ASSERT_FALSE(student.isAdmin());
        ASSERT_EQ(student.getOrganizationName(), "ACM Chapter");
        student.setOrganizationName("IEEE Branch");
        ASSERT_EQ(student.getOrganizationName(), "IEEE Branch");
        ASSERT_TRUE(student.verifyPassword("pass123"));
        ASSERT_FALSE(student.verifyPassword("wrongpass"));

        // Faculty / Staff Member
        FacultyStaffMember faculty("u-f01", "Dr. Robert Smith", "rsmith@campus.edu", "Physics", "EMP-10293");
        ASSERT_EQ(faculty.getRole(), UserRole::FacultyStaff);
        ASSERT_EQ(faculty.getRoleName(), "Faculty/Staff Member");
        ASSERT_TRUE(faculty.canSubmit());
        ASSERT_FALSE(faculty.canApprove());
        ASSERT_FALSE(faculty.canCoordinate());
        ASSERT_TRUE(faculty.isExpeditedEligible());
        ASSERT_EQ(faculty.getEmployeeId(), "EMP-10293");
        faculty.setEmployeeId("EMP-99999");
        ASSERT_EQ(faculty.getEmployeeId(), "EMP-99999");

        // Facility Coordinator
        FacilityCoordinator coord("u-c01", "Carol Davis", "carol@campus.edu", "Facilities", {"Student Union", "Engineering Hall"});
        ASSERT_EQ(coord.getRole(), UserRole::FacilityCoord);
        ASSERT_EQ(coord.getRoleName(), "Facility Coordinator");
        ASSERT_FALSE(coord.canSubmit());
        ASSERT_FALSE(coord.canApprove());
        ASSERT_TRUE(coord.canCoordinate());
        ASSERT_TRUE(coord.managesBuilding("Student Union"));
        ASSERT_TRUE(coord.managesBuilding("Engineering Hall"));
        ASSERT_FALSE(coord.managesBuilding("Athletic Center"));
        coord.addAssignedBuilding("Athletic Center");
        ASSERT_TRUE(coord.managesBuilding("Athletic Center"));
        ASSERT_EQ(coord.getAssignedBuildings().size(), static_cast<size_t>(3));

        // Unrestricted Coordinator (empty building assignment list)
        FacilityCoordinator superCoord("u-c02", "Alex All", "alex@campus.edu", "Campus Operations", {});
        ASSERT_TRUE(superCoord.managesBuilding("Any Building"));

        // Event Approver
        EventApprover approver("u-a01", "Dean Gregory House", "ghouse@campus.edu", "Student Affairs", 2);
        ASSERT_EQ(approver.getRole(), UserRole::EventAppr);
        ASSERT_EQ(approver.getRoleName(), "Event Approver");
        ASSERT_FALSE(approver.canSubmit());
        ASSERT_TRUE(approver.canApprove());
        ASSERT_FALSE(approver.canCoordinate());
        ASSERT_EQ(approver.getApprovalTier(), 2);
        approver.setApprovalTier(3);
        ASSERT_EQ(approver.getApprovalTier(), 3);

        // Security Officer
        SecurityOfficer officer("u-sec01", "Officer Marcus Vance", "mvance@campus.edu", "Campus Police", "BADGE-404");
        ASSERT_EQ(officer.getRole(), UserRole::SecurityOfficer);
        ASSERT_EQ(officer.getRoleName(), "Campus Security Officer");
        ASSERT_TRUE(officer.isSecurityOfficer());
        ASSERT_FALSE(officer.canApprove());
        ASSERT_EQ(officer.getBadgeNumber(), "BADGE-404");
        officer.setBadgeNumber("BADGE-999");
        ASSERT_EQ(officer.getBadgeNumber(), "BADGE-999");

        // Administrator
        Administrator admin("u-admin01", "Root Administrator", "root@campus.edu", "IT");
        ASSERT_EQ(admin.getRole(), UserRole::Admin);
        ASSERT_EQ(admin.getRoleName(), "System Administrator");
        ASSERT_TRUE(admin.isAdmin());
        ASSERT_TRUE(admin.canSubmit());
        ASSERT_TRUE(admin.canApprove());
        ASSERT_TRUE(admin.canCoordinate());

        // Polymorphic Dispatch via vector of base pointers
        std::vector<std::unique_ptr<User>> polyUsers;
        polyUsers.push_back(std::make_unique<StudentOrgRepresentative>(student));
        polyUsers.push_back(std::make_unique<FacultyStaffMember>(faculty));
        polyUsers.push_back(std::make_unique<FacilityCoordinator>(coord));
        polyUsers.push_back(std::make_unique<EventApprover>(approver));
        polyUsers.push_back(std::make_unique<SecurityOfficer>(officer));
        polyUsers.push_back(std::make_unique<Administrator>(admin));

        ASSERT_EQ(polyUsers[0]->getRole(), UserRole::StudentOrgRep);
        ASSERT_EQ(polyUsers[1]->getRole(), UserRole::FacultyStaff);
        ASSERT_EQ(polyUsers[2]->getRole(), UserRole::FacilityCoord);
        ASSERT_EQ(polyUsers[3]->getRole(), UserRole::EventAppr);
        ASSERT_EQ(polyUsers[4]->getRole(), UserRole::SecurityOfficer);
        ASSERT_EQ(polyUsers[5]->getRole(), UserRole::Admin);

        ASSERT_TRUE(polyUsers[0]->canSubmit());
        ASSERT_TRUE(polyUsers[1]->isExpeditedEligible());
        ASSERT_TRUE(polyUsers[2]->canCoordinate());
        ASSERT_TRUE(polyUsers[3]->canApprove());
        ASSERT_TRUE(polyUsers[4]->isSecurityOfficer());
        ASSERT_TRUE(polyUsers[5]->isAdmin());

        // Mutators and Profile Updates
        student.setFullName("Alice Chen-Smith");
        student.setEmail("alice.cs@campus.edu");
        student.setDepartment("Data Science");
        student.setPasswordHash("newhash");
        ASSERT_EQ(student.getFullName(), "Alice Chen-Smith");
        ASSERT_EQ(student.getEmail(), "alice.cs@campus.edu");
        ASSERT_EQ(student.getDepartment(), "Data Science");
        ASSERT_TRUE(student.verifyPassword("newhash"));

        // Enum Serialization Helpers
        ASSERT_EQ(userRoleToString(UserRole::StudentOrgRep), "StudentOrgRep");
        ASSERT_EQ(userRoleToString(UserRole::FacultyStaff), "FacultyStaff");
        ASSERT_EQ(userRoleToString(UserRole::FacilityCoord), "FacilityCoord");
        ASSERT_EQ(userRoleToString(UserRole::EventAppr), "EventAppr");
        ASSERT_EQ(userRoleToString(UserRole::SecurityOfficer), "SecurityOfficer");
        ASSERT_EQ(userRoleToString(UserRole::Admin), "Admin");

        ASSERT_EQ(stringToUserRole("StudentOrgRep"), UserRole::StudentOrgRep);
        ASSERT_EQ(stringToUserRole("StudentOrgRepresentative"), UserRole::StudentOrgRep);
        ASSERT_EQ(stringToUserRole("FacultyStaff"), UserRole::FacultyStaff);
        ASSERT_EQ(stringToUserRole("FacultyStaffMember"), UserRole::FacultyStaff);
        ASSERT_EQ(stringToUserRole("FacilityCoord"), UserRole::FacilityCoord);
        ASSERT_EQ(stringToUserRole("FacilityCoordinator"), UserRole::FacilityCoord);
        ASSERT_EQ(stringToUserRole("EventAppr"), UserRole::EventAppr);
        ASSERT_EQ(stringToUserRole("EventApprover"), UserRole::EventAppr);
        ASSERT_EQ(stringToUserRole("SecurityOfficer"), UserRole::SecurityOfficer);
        ASSERT_EQ(stringToUserRole("Admin"), UserRole::Admin);
        ASSERT_EQ(stringToUserRole("Administrator"), UserRole::Admin);
        ASSERT_EQ(stringToUserRole("NonExistentRole"), UserRole::StudentOrgRep);
    }

    // -------------------------------------------------------------------------
    // 2. Facility Operations, Capacity, Equipment, and Blackouts
    // -------------------------------------------------------------------------
    void testFacilityOperations() {
        using namespace ::domain;

        Facility fac("FAC-101", "Centennial Auditorium", "Centennial Hall", 250, 45.0, RoomStatus::Available, {"Projector", "Sound System"});
        ASSERT_EQ(fac.getFacilityId(), "FAC-101");
        ASSERT_EQ(fac.getName(), "Centennial Auditorium");
        ASSERT_EQ(fac.getBuilding(), "Centennial Hall");
        ASSERT_EQ(fac.getCapacity(), 250);
        ASSERT_EQ(fac.getHourlyRate(), 45.0);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Available);
        ASSERT_FALSE(fac.areDoorsLocked());

        // Equipment checks
        ASSERT_TRUE(fac.hasEquipment("Projector"));
        ASSERT_TRUE(fac.hasEquipment("Sound System"));
        ASSERT_FALSE(fac.hasEquipment("Stage Lighting"));
        fac.addSupportedEquipment("Stage Lighting");
        ASSERT_TRUE(fac.hasEquipment("Stage Lighting"));
        // Duplicate equipment addition is ignored
        size_t eqCount = fac.getSupportedEquipment().size();
        fac.addSupportedEquipment("Projector");
        ASSERT_EQ(fac.getSupportedEquipment().size(), eqCount);

        // Mutators
        fac.setName("Centennial Great Hall");
        fac.setBuilding("West Wing");
        fac.setCapacity(300);
        fac.setHourlyRate(60.0);
        ASSERT_EQ(fac.getName(), "Centennial Great Hall");
        ASSERT_EQ(fac.getBuilding(), "West Wing");
        ASSERT_EQ(fac.getCapacity(), 300);
        ASSERT_EQ(fac.getHourlyRate(), 60.0);

        // Schedule Availability and Blackouts
        std::time_t t1 = makeTime(2026, 9, 10, 10, 0);
        std::time_t t2 = makeTime(2026, 9, 10, 12, 0);
        ASSERT_TRUE(fac.checkAvailability(t1, t2));

        // Blackout overlapping query window
        fac.setBlackout(t1 - 1800, t1 + 1800, "Audio Console Maintenance");
        ASSERT_EQ(fac.getBlackoutWindows().size(), static_cast<size_t>(1));
        ASSERT_FALSE(fac.checkAvailability(t1, t2));

        // Non-overlapping query outside blackout
        std::time_t t3 = makeTime(2026, 9, 10, 14, 0);
        std::time_t t4 = makeTime(2026, 9, 10, 16, 0);
        ASSERT_TRUE(fac.checkAvailability(t3, t4));

        fac.clearBlackouts();
        ASSERT_TRUE(fac.getBlackoutWindows().empty());
        ASSERT_TRUE(fac.checkAvailability(t1, t2));

        // Facility Status impacts on availability
        fac.setStatus(RoomStatus::OutOfService);
        ASSERT_FALSE(fac.checkAvailability(t1, t2));

        fac.setStatus(RoomStatus::EmergencyHold);
        ASSERT_FALSE(fac.checkAvailability(t1, t2));

        fac.setStatus(RoomStatus::Reserved);
        ASSERT_TRUE(fac.checkAvailability(t1, t2)); // Reserved status does not block facility checkAvailability directly

        fac.setStatus(RoomStatus::Available);
        ASSERT_TRUE(fac.checkAvailability(t1, t2));

        // Door Lock management
        fac.setDoorsLocked(true);
        ASSERT_TRUE(fac.areDoorsLocked());
        fac.setDoorsLocked(false);
        ASSERT_FALSE(fac.areDoorsLocked());

        // RoomStatus Enums
        ASSERT_EQ(roomStatusToString(RoomStatus::Available), "Available");
        ASSERT_EQ(roomStatusToString(RoomStatus::Reserved), "Reserved");
        ASSERT_EQ(roomStatusToString(RoomStatus::Setup), "Setup");
        ASSERT_EQ(roomStatusToString(RoomStatus::Occupied), "Occupied");
        ASSERT_EQ(roomStatusToString(RoomStatus::Turnover), "Turnover");
        ASSERT_EQ(roomStatusToString(RoomStatus::OutOfService), "OutOfService");
        ASSERT_EQ(roomStatusToString(RoomStatus::EmergencyHold), "EmergencyHold");

        ASSERT_EQ(stringToRoomStatus("Available"), RoomStatus::Available);
        ASSERT_EQ(stringToRoomStatus("Reserved"), RoomStatus::Reserved);
        ASSERT_EQ(stringToRoomStatus("Setup"), RoomStatus::Setup);
        ASSERT_EQ(stringToRoomStatus("Occupied"), RoomStatus::Occupied);
        ASSERT_EQ(stringToRoomStatus("Turnover"), RoomStatus::Turnover);
        ASSERT_EQ(stringToRoomStatus("OutOfService"), RoomStatus::OutOfService);
        ASSERT_EQ(stringToRoomStatus("EmergencyHold"), RoomStatus::EmergencyHold);
        ASSERT_EQ(stringToRoomStatus("InvalidStatus"), RoomStatus::Available);
    }

    // -------------------------------------------------------------------------
    // 3. Reservation Composition, Aggregate Root, and Security Detection
    // -------------------------------------------------------------------------
    void testReservationComposition() {
        using namespace ::domain;

        std::time_t startDay = makeTime(2026, 9, 15, 10, 0); // 10:00 AM daytime
        std::time_t endDay   = makeTime(2026, 9, 15, 12, 0); // 12:00 PM daytime

        Reservation res("RES-901", "u-s01", "FAC-101", "Robotics Exhibition", "Exhibition", 75, startDay, endDay);
        ASSERT_EQ(res.getReservationId(), "RES-901");
        ASSERT_EQ(res.getRequesterId(), "u-s01");
        ASSERT_EQ(res.getFacilityId(), "FAC-101");
        ASSERT_EQ(res.getEventTitle(), "Robotics Exhibition");
        ASSERT_EQ(res.getEventType(), "Exhibition");
        ASSERT_EQ(res.getExpectedAttendance(), 75);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Draft);
        ASSERT_FALSE(res.isAfterHours());
        ASSERT_FALSE(res.getRequiresSecurity());

        // Child Equipment Requests
        res.addEquipment(EquipmentRequest("EQ-01", "Dual Wireless Mics", 2, false));
        ASSERT_EQ(res.getEquipmentRequests().size(), static_cast<size_t>(1));
        ASSERT_EQ(res.getEquipmentRequests()[0].getRequestId(), "EQ-01");
        ASSERT_EQ(res.getEquipmentRequests()[0].getEquipmentType(), "Dual Wireless Mics");
        ASSERT_EQ(res.getEquipmentRequests()[0].getQuantity(), 2);
        ASSERT_FALSE(res.getEquipmentRequests()[0].isFulfilled());
        res.getEquipmentRequests()[0].setFulfilled(true);
        res.getEquipmentRequests()[0].setQuantity(3);
        ASSERT_TRUE(res.getEquipmentRequests()[0].isFulfilled());
        ASSERT_EQ(res.getEquipmentRequests()[0].getQuantity(), 3);

        // Add equipment via helper overload
        res.addEquipment("HD Projector", 1);
        ASSERT_EQ(res.getEquipmentRequests().size(), static_cast<size_t>(2));

        // Child Support Requests
        res.addSupportRequest(SupportRequest("SUP-01", "Custodial", "Pre-event cleaning", false, ""));
        ASSERT_EQ(res.getSupportRequests().size(), static_cast<size_t>(1));
        ASSERT_EQ(res.getSupportRequests()[0].getSupportType(), "Custodial");
        ASSERT_FALSE(res.getSupportRequests()[0].isAssigned());
        res.getSupportRequests()[0].assignOfficer("USR-006");
        ASSERT_TRUE(res.getSupportRequests()[0].isAssigned());
        ASSERT_EQ(res.getSupportRequests()[0].getAssignedOfficerId(), "USR-006");
        res.getSupportRequests()[0].setNotes("Updated turnover instructions");
        ASSERT_EQ(res.getSupportRequests()[0].getNotes(), "Updated turnover instructions");

        res.addSupportRequest("AV Support", "Stage lighting control operator needed");
        ASSERT_EQ(res.getSupportRequests().size(), static_cast<size_t>(2));

        // Child Approval Records
        std::time_t appTime = makeTime(2026, 9, 12, 14, 0);
        res.recordApproval(Approval("APP-01", "u-a01", "Approved", "Standard review passed", appTime));
        ASSERT_EQ(res.getApprovals().size(), static_cast<size_t>(1));
        ASSERT_EQ(res.getApprovals()[0].getApprovalId(), "APP-01");
        ASSERT_EQ(res.getApprovals()[0].getApproverId(), "u-a01");
        ASSERT_EQ(res.getApprovals()[0].getDecision(), "Approved");
        ASSERT_EQ(res.getApprovals()[0].getComments(), "Standard review passed");
        ASSERT_EQ(res.getApprovals()[0].getDecidedAt(), appTime);

        res.recordApproval("u-a02", "Revision Requested", "Provide safety waiver");
        ASSERT_EQ(res.getApprovals().size(), static_cast<size_t>(2));

        // Headcount > 100 triggers security gate
        res.setExpectedAttendance(150);
        ASSERT_TRUE(res.getRequiresSecurity());

        res.setExpectedAttendance(50);
        ASSERT_FALSE(res.getRequiresSecurity());

        // After-hours detection triggers security gate even with low headcount
        std::time_t startNight = makeTime(2026, 9, 15, 20, 30); // 8:30 PM
        std::time_t endNight   = makeTime(2026, 9, 15, 22, 30); // 10:30 PM
        Reservation nightRes("RES-902", "u-s01", "FAC-101", "Late Night Hack", "Social", 30, startNight, endNight);
        ASSERT_TRUE(nightRes.isAfterHours());
        ASSERT_TRUE(nightRes.getRequiresSecurity());

        // Early morning after-hours detection
        std::time_t startEarly = makeTime(2026, 9, 15, 6, 0);  // 6:00 AM (< 8 AM)
        std::time_t endEarly   = makeTime(2026, 9, 15, 7, 30); // 7:30 AM
        Reservation earlyRes("RES-903", "u-s01", "FAC-101", "Dawn Run", "Athletic", 15, startEarly, endEarly);
        ASSERT_TRUE(earlyRes.isAfterHours());
        ASSERT_TRUE(earlyRes.getRequiresSecurity());

        // Multi-day spanning after-hours detection
        std::time_t multiStart = makeTime(2026, 9, 15, 14, 0);
        std::time_t multiEnd   = makeTime(2026, 9, 16, 10, 0);
        Reservation multiRes("RES-904", "u-s01", "FAC-101", "Lock-in", "Social", 20, multiStart, multiEnd);
        ASSERT_TRUE(multiRes.isAfterHours());
        ASSERT_TRUE(multiRes.getRequiresSecurity());

        // Zero start/end times
        Reservation emptyRes;
        ASSERT_FALSE(emptyRes.isAfterHours());

        // Reservation Mutators
        res.setReservationId("RES-901-MOD");
        res.setRequesterId("u-s02");
        res.setFacilityId("FAC-102");
        res.setEventTitle("Updated Title");
        res.setEventType("Conference");
        res.setStatus(ReservationStatus::Submitted);
        res.setHoldExpiresAt(startDay + 900);
        res.setRevisionDeadline(startDay + 86400);

        ASSERT_EQ(res.getReservationId(), "RES-901-MOD");
        ASSERT_EQ(res.getRequesterId(), "u-s02");
        ASSERT_EQ(res.getFacilityId(), "FAC-102");
        ASSERT_EQ(res.getEventTitle(), "Updated Title");
        ASSERT_EQ(res.getEventType(), "Conference");
        ASSERT_EQ(res.getStatus(), ReservationStatus::Submitted);
        ASSERT_EQ(res.getHoldExpiresAt(), startDay + 900);
        ASSERT_EQ(res.getRevisionDeadline(), startDay + 86400);

        // ReservationStatus Enum strings
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Draft), "Draft");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Submitted), "Submitted");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::UnderReview), "UnderReview");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::SecurityReview), "SecurityReview");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Approved), "Approved");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::NeedsRevision), "NeedsRevision");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Expired), "Expired");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Rejected), "Rejected");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::InProgress), "InProgress");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Completed), "Completed");
        ASSERT_EQ(reservationStatusToString(ReservationStatus::Cancelled), "Cancelled");

        ASSERT_EQ(stringToReservationStatus("Draft"), ReservationStatus::Draft);
        ASSERT_EQ(stringToReservationStatus("Submitted"), ReservationStatus::Submitted);
        ASSERT_EQ(stringToReservationStatus("UnderReview"), ReservationStatus::UnderReview);
        ASSERT_EQ(stringToReservationStatus("SecurityReview"), ReservationStatus::SecurityReview);
        ASSERT_EQ(stringToReservationStatus("Approved"), ReservationStatus::Approved);
        ASSERT_EQ(stringToReservationStatus("NeedsRevision"), ReservationStatus::NeedsRevision);
        ASSERT_EQ(stringToReservationStatus("Expired"), ReservationStatus::Expired);
        ASSERT_EQ(stringToReservationStatus("Rejected"), ReservationStatus::Rejected);
        ASSERT_EQ(stringToReservationStatus("InProgress"), ReservationStatus::InProgress);
        ASSERT_EQ(stringToReservationStatus("Completed"), ReservationStatus::Completed);
        ASSERT_EQ(stringToReservationStatus("Cancelled"), ReservationStatus::Cancelled);
        ASSERT_EQ(stringToReservationStatus("NonExistent"), ReservationStatus::Draft);
    }

    // -------------------------------------------------------------------------
    // 4. Thread-Safe Notification Service
    // -------------------------------------------------------------------------
    void testNotificationService() {
        using namespace ::domain;

        auto& svc = NotificationService::getInstance();
        svc.clear();
        ASSERT_EQ(svc.count(), static_cast<size_t>(0));

        // Confirmation notice
        svc.sendConfirmation("student@campus.edu", "RES-101", "Audio/Visual tech requested.");
        ASSERT_EQ(svc.count(), static_cast<size_t>(1));

        // Security alert
        svc.sendSecurityAlert("RES-102", "Large crowd expected (250+ attendees).");
        ASSERT_EQ(svc.count(), static_cast<size_t>(2));

        // Custodial notice
        svc.sendCustodialNotice("FAC-101", "Turnover cleaning and trash clearing required.");
        ASSERT_EQ(svc.count(), static_cast<size_t>(3));

        auto allNotifs = svc.getAllNotifications();
        ASSERT_EQ(allNotifs.size(), static_cast<size_t>(3));
        ASSERT_EQ(allNotifs[0].recipientEmail, "student@campus.edu");
        ASSERT_EQ(allNotifs[1].recipientEmail, "security-ops@campus.edu");
        ASSERT_EQ(allNotifs[2].recipientEmail, "custodial-facilities@campus.edu");

        // getRecent
        auto recent = svc.getRecent(2);
        ASSERT_EQ(recent.size(), static_cast<size_t>(2));
        ASSERT_EQ(recent[0].recipientEmail, "security-ops@campus.edu");
        ASSERT_EQ(recent[1].recipientEmail, "custodial-facilities@campus.edu");

        // Multithreaded Concurrent Dispatch Verification
        std::vector<std::thread> workers;
        const int numThreads = 4;
        const int notifsPerThread = 25;
        for (int t = 0; t < numThreads; ++t) {
            workers.emplace_back([t]() {
                auto& s = NotificationService::getInstance();
                for (int i = 0; i < notifsPerThread; ++i) {
                    s.sendNotification("worker" + std::to_string(t) + "@campus.edu",
                                       "Notice " + std::to_string(i),
                                       "Thread payload message");
                }
            });
        }
        for (auto& w : workers) {
            w.join();
        }

        size_t expectedTotal = 3 + (numThreads * notifsPerThread);
        ASSERT_EQ(svc.count(), expectedTotal);

        svc.clear();
        ASSERT_EQ(svc.count(), static_cast<size_t>(0));
    }

    // -------------------------------------------------------------------------
    // 5. Reservation Happy Path (6-Step Lifecycle)
    // -------------------------------------------------------------------------
    void testReservationHappyPath() {
        using namespace ::domain;
        using namespace ::state;

        ReservationStateMachine sm;
        NotificationService::getInstance().clear();

        std::time_t now = makeTime(2026, 9, 1, 9, 0);
        std::time_t eventStart = makeTime(2026, 9, 5, 14, 0); // 4 days later (midday)
        std::time_t eventEnd   = makeTime(2026, 9, 5, 16, 0);

        Reservation res("RES-HAPPY-01", "u-s01", "FAC-104", "Robotics Club Workshop", "Workshop", 20, eventStart, eventEnd, now);
        Facility fac("FAC-104", "Seminar Room", "East Hall", 25);

        ASSERT_EQ(res.getStatus(), ReservationStatus::Draft);

        // Step 1: Draft -> Submitted (15-min soft lock)
        TransitionResult rSub = sm.submit(res, true, now);
        ASSERT_TRUE(rSub.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Submitted);
        ASSERT_EQ(res.getHoldExpiresAt(), now + 900);

        // Step 2: Submitted -> UnderReview (within 15 min buffer)
        TransitionResult rRoute = sm.routeToReview(res, now + 300);
        ASSERT_TRUE(rRoute.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::UnderReview);

        // Step 3: Security Evaluation (standard risk -> remains UnderReview)
        TransitionResult rSec = sm.evaluateSecurity(res);
        ASSERT_TRUE(rSec.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::UnderReview);

        // Step 4: UnderReview -> Approved
        TransitionResult rApp = sm.approve(res, "u-a01", "Approved for campus student club", now + 600);
        ASSERT_TRUE(rApp.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Approved);
        ASSERT_EQ(res.getApprovals().size(), static_cast<size_t>(1));
        ASSERT_EQ(res.getApprovals()[0].getApproverId(), "u-a01");
        ASSERT_EQ(res.getApprovals()[0].getDecision(), "Approved");

        // Staging window opens
        fac.setStatus(RoomStatus::Setup);

        // Step 5: Approved -> InProgress (event start time reached)
        TransitionResult rIn = sm.checkIn(res, fac, eventStart);
        ASSERT_TRUE(rIn.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::InProgress);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Occupied);
        ASSERT_FALSE(fac.areDoorsLocked());

        // Step 6: InProgress -> Completed (event end time reached)
        TransitionResult rDone = sm.conclude(res, fac, eventEnd);
        ASSERT_TRUE(rDone.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Completed);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Turnover);
        ASSERT_TRUE(fac.areDoorsLocked());
    }

    // -------------------------------------------------------------------------
    // 6. Soft Lock Expiration Guard (15-Minute Expiry)
    // -------------------------------------------------------------------------
    void testSoftLockExpiration() {
        using namespace ::domain;
        using namespace ::state;

        ReservationStateMachine sm;

        std::time_t now = makeTime(2026, 9, 1, 9, 0);
        std::time_t eventStart = makeTime(2026, 9, 5, 14, 0);
        std::time_t eventEnd   = makeTime(2026, 9, 5, 16, 0);

        // 1. Submit sets 900s hold
        Reservation res1("RES-HOLD-01", "u-s01", "FAC-104", "Meeting 1", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(res1, true, now).success);
        ASSERT_EQ(res1.getHoldExpiresAt(), now + 900);

        // Routing at exact boundary (now + 900) succeeds
        ASSERT_TRUE(sm.routeToReview(res1, now + 900).success);
        ASSERT_EQ(res1.getStatus(), ReservationStatus::UnderReview);

        // 2. Second reservation: route attempt after 15 minutes fails
        Reservation res2("RES-HOLD-02", "u-s01", "FAC-104", "Meeting 2", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(res2, true, now).success);

        TransitionResult rExpiredRoute = sm.routeToReview(res2, now + 901);
        ASSERT_FALSE(rExpiredRoute.success);
        ASSERT_EQ(res2.getStatus(), ReservationStatus::Submitted);

        // Expiration attempt before 15 minutes passes fails
        TransitionResult rEarlyExpire = sm.expire(res2, now + 899);
        ASSERT_FALSE(rEarlyExpire.success);
        ASSERT_EQ(res2.getStatus(), ReservationStatus::Submitted);

        // Expiration after 15 minutes succeeds -> Expired
        TransitionResult rExpire = sm.expire(res2, now + 901);
        ASSERT_TRUE(rExpire.success);
        ASSERT_EQ(res2.getStatus(), ReservationStatus::Expired);

        // Expired is a terminal state
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Expired, ReservationStatus::Draft));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Expired, ReservationStatus::Submitted));
    }

    // -------------------------------------------------------------------------
    // 7. Security Review Escalation (Headcount > 100 & After-Hours)
    // -------------------------------------------------------------------------
    void testSecurityReviewEscalation() {
        using namespace ::domain;
        using namespace ::state;

        ReservationStateMachine sm;
        NotificationService::getInstance().clear();

        std::time_t now = makeTime(2026, 9, 1, 9, 0);
        std::time_t eventStart = makeTime(2026, 9, 5, 14, 0);
        std::time_t eventEnd   = makeTime(2026, 9, 5, 18, 0);

        // Case A: Headcount > 100 escalation
        Reservation resLarge("RES-SEC-01", "u-s01", "FAC-101", "Annual Hackathon", "Conference", 250, eventStart, eventEnd, now);
        ASSERT_TRUE(resLarge.getRequiresSecurity());

        ASSERT_TRUE(sm.submit(resLarge, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resLarge, now + 60).success);

        // evaluateSecurity routes to SecurityReview
        TransitionResult rEval = sm.evaluateSecurity(resLarge);
        ASSERT_TRUE(rEval.success);
        ASSERT_EQ(resLarge.getStatus(), ReservationStatus::SecurityReview);

        // Direct standard approval MUST FAIL
        TransitionResult rDirectApp = sm.approve(resLarge, "u-a01", "Direct approval attempt", now + 120);
        ASSERT_FALSE(rDirectApp.success);
        ASSERT_EQ(resLarge.getStatus(), ReservationStatus::SecurityReview);

        // Security Clear by Security Officer succeeds
        TransitionResult rClear = sm.securityClear(resLarge, "u-sec01", "Detail assigned: 2 officers", now + 180);
        ASSERT_TRUE(rClear.success);
        ASSERT_EQ(resLarge.getStatus(), ReservationStatus::Approved);
        ASSERT_FALSE(resLarge.getSupportRequests().empty());
        ASSERT_TRUE(resLarge.getSupportRequests()[0].isAssigned());
        ASSERT_EQ(resLarge.getSupportRequests()[0].getAssignedOfficerId(), "u-sec01");

        // Case B: After-hours booking escalation and Security Denial
        std::time_t lateStart = makeTime(2026, 9, 5, 21, 0); // 9:00 PM
        std::time_t lateEnd   = makeTime(2026, 9, 5, 23, 0); // 11:00 PM
        Reservation resLate("RES-SEC-02", "u-s01", "FAC-101", "Late Night Rave", "Party", 40, lateStart, lateEnd, now);
        ASSERT_TRUE(resLate.isAfterHours());
        ASSERT_TRUE(resLate.getRequiresSecurity());

        ASSERT_TRUE(sm.submit(resLate, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resLate, now + 60).success);
        ASSERT_TRUE(sm.evaluateSecurity(resLate).success);
        ASSERT_EQ(resLate.getStatus(), ReservationStatus::SecurityReview);

        // Direct standard approval fails
        ASSERT_FALSE(sm.approve(resLate, "u-a01", "Standard direct", now + 120).success);

        // Security Denial transitions to Rejected
        TransitionResult rDeny = sm.securityDeny(resLate, "u-sec01", "Noise ordinance and inadequate staffing", now + 180);
        ASSERT_TRUE(rDeny.success);
        ASSERT_EQ(resLate.getStatus(), ReservationStatus::Rejected);

        // Unauthorized direct actions in SecurityReview
        Reservation resIllegal("RES-SEC-03", "u-s01", "FAC-101", "Illegal checkin test", "Test", 200, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resIllegal, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resIllegal, now + 60).success);
        ASSERT_TRUE(sm.evaluateSecurity(resIllegal).success);

        Facility fac("FAC-101", "Centennial Auditorium", "Centennial Hall", 250);
        fac.setStatus(RoomStatus::Setup);
        ASSERT_FALSE(sm.checkIn(resIllegal, fac, eventStart).success);
        ASSERT_FALSE(sm.conclude(resIllegal, fac, eventEnd).success);
        ASSERT_FALSE(sm.resubmit(resIllegal, true, now + 120).success);
    }

    // -------------------------------------------------------------------------
    // 8. Revision and Resubmission Loop
    // -------------------------------------------------------------------------
    void testRevisionAndResubmissionLoop() {
        using namespace ::domain;
        using namespace ::state;

        ReservationStateMachine sm;
        NotificationService::getInstance().clear();

        std::time_t now = makeTime(2026, 9, 1, 9, 0);
        std::time_t deadline = now + 86400; // 24 hours
        std::time_t eventStart = makeTime(2026, 9, 6, 14, 0);
        std::time_t eventEnd   = makeTime(2026, 9, 6, 16, 0);

        Reservation res("RES-REV-01", "u-s01", "FAC-102", "Autonomous Drone Flight", "Demo", 30, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(res, true, now).success);
        ASSERT_TRUE(sm.routeToReview(res, now + 60).success);

        // Past deadline revision request fails
        TransitionResult rPastDead = sm.requestRevision(res, "u-a01", "Fix details", now - 10, now);
        ASSERT_FALSE(rPastDead.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::UnderReview);

        // Future deadline revision request succeeds -> NeedsRevision
        TransitionResult rRev = sm.requestRevision(res, "u-a01", "Attach drone safety certification", deadline, now);
        ASSERT_TRUE(rRev.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::NeedsRevision);
        ASSERT_EQ(res.getRevisionDeadline(), deadline);

        // Resubmit with conflicts fails
        TransitionResult rConf = sm.resubmit(res, false, now + 3600);
        ASSERT_FALSE(rConf.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::NeedsRevision);

        // Resubmit past deadline fails
        TransitionResult rLate = sm.resubmit(res, true, deadline + 10);
        ASSERT_FALSE(rLate.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::NeedsRevision);

        // Valid resubmission within deadline succeeds -> Submitted (refreshed hold)
        TransitionResult rResub = sm.resubmit(res, true, now + 3600);
        ASSERT_TRUE(rResub.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Submitted);
        ASSERT_EQ(res.getHoldExpiresAt(), now + 3600 + 900);

        // Test Revision Deadline Expiry branch
        ASSERT_TRUE(sm.routeToReview(res, now + 3700).success);
        std::time_t deadline2 = now + 5000;
        ASSERT_TRUE(sm.requestRevision(res, "u-a01", "Provide pilot license", deadline2, now + 3800).success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::NeedsRevision);

        // Premature expire attempt fails
        TransitionResult rEarly = sm.expire(res, deadline2 - 10);
        ASSERT_FALSE(rEarly.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::NeedsRevision);

        // Expire after deadline succeeds -> Expired
        TransitionResult rExp = sm.expire(res, deadline2 + 1);
        ASSERT_TRUE(rExp.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Expired);
    }

    // -------------------------------------------------------------------------
    // 9. Cancellation Rules and Lead-Time Guard Enforcement
    // -------------------------------------------------------------------------
    void testCancellationRulesAndLeadTime() {
        using namespace ::domain;
        using namespace ::state;

        ReservationStateMachine sm;
        NotificationService::getInstance().clear();

        std::time_t now = makeTime(2026, 9, 1, 9, 0);
        std::time_t eventStart = makeTime(2026, 9, 5, 14, 0);
        std::time_t eventEnd   = makeTime(2026, 9, 5, 16, 0);

        // 1. Cancellation from Draft
        Reservation resDraft("RES-CAN-01", "u-s01", "FAC-104", "Draft Cancel", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.cancel(resDraft, "u-s01", now).success);
        ASSERT_EQ(resDraft.getStatus(), ReservationStatus::Cancelled);

        // 2. Cancellation from Submitted
        Reservation resSub("RES-CAN-02", "u-s01", "FAC-104", "Sub Cancel", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resSub, true, now).success);
        ASSERT_TRUE(sm.cancel(resSub, "u-s01", now + 60).success);
        ASSERT_EQ(resSub.getStatus(), ReservationStatus::Cancelled);

        // 3. Cancellation from UnderReview
        Reservation resRev("RES-CAN-03", "u-s01", "FAC-104", "Review Cancel", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resRev, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resRev, now + 60).success);
        ASSERT_TRUE(sm.cancel(resRev, "u-s01", now + 120).success);
        ASSERT_EQ(resRev.getStatus(), ReservationStatus::Cancelled);

        // 4. Cancellation from SecurityReview
        Reservation resSec("RES-CAN-04", "u-s01", "FAC-101", "Sec Cancel", "Event", 250, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resSec, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resSec, now + 60).success);
        ASSERT_TRUE(sm.evaluateSecurity(resSec).success);
        ASSERT_EQ(resSec.getStatus(), ReservationStatus::SecurityReview);
        ASSERT_TRUE(sm.cancel(resSec, "u-s01", now + 120).success);
        ASSERT_EQ(resSec.getStatus(), ReservationStatus::Cancelled);

        // 5. Cancellation from NeedsRevision
        Reservation resNeeds("RES-CAN-05", "u-s01", "FAC-104", "NeedsRev Cancel", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resNeeds, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resNeeds, now + 60).success);
        ASSERT_TRUE(sm.requestRevision(resNeeds, "u-a01", "Fix", now + 86400, now + 120).success);
        ASSERT_TRUE(sm.cancel(resNeeds, "u-s01", now + 180).success);
        ASSERT_EQ(resNeeds.getStatus(), ReservationStatus::Cancelled);

        // 6. Cancellation from Approved with 24-Hour Lead-Time Guard
        Reservation resApp("RES-CAN-06", "u-s01", "FAC-104", "Approved Cancel Test", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resApp, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resApp, now + 60).success);
        ASSERT_TRUE(sm.approve(resApp, "u-a01", "Approved", now + 120).success);
        ASSERT_EQ(resApp.getStatus(), ReservationStatus::Approved);

        // Cancel with > 24 hours lead time (25 hours prior) succeeds
        std::time_t t25HoursBefore = eventStart - 25 * 3600;
        ASSERT_TRUE(sm.cancel(resApp, "u-s01", t25HoursBefore).success);
        ASSERT_EQ(resApp.getStatus(), ReservationStatus::Cancelled);

        // Exact 24 hours boundary succeeds
        Reservation resAppExact("RES-CAN-07", "u-s01", "FAC-104", "Exact 24h Cancel", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resAppExact, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resAppExact, now + 60).success);
        ASSERT_TRUE(sm.approve(resAppExact, "u-a01", "Approved", now + 120).success);
        std::time_t t24HoursBefore = eventStart - 24 * 3600;
        ASSERT_TRUE(sm.cancel(resAppExact, "u-s01", t24HoursBefore).success);
        ASSERT_EQ(resAppExact.getStatus(), ReservationStatus::Cancelled);

        // Cancel with < 24 hours lead time (23 hours prior) MUST BE REJECTED
        Reservation resAppLate("RES-CAN-08", "u-s01", "FAC-104", "Late Cancel Test", "Meeting", 10, eventStart, eventEnd, now);
        ASSERT_TRUE(sm.submit(resAppLate, true, now).success);
        ASSERT_TRUE(sm.routeToReview(resAppLate, now + 60).success);
        ASSERT_TRUE(sm.approve(resAppLate, "u-a01", "Approved", now + 120).success);

        std::time_t t23HoursBefore = eventStart - 23 * 3600;
        TransitionResult rLateCancel = sm.cancel(resAppLate, "u-s01", t23HoursBefore);
        ASSERT_FALSE(rLateCancel.success);
        ASSERT_EQ(resAppLate.getStatus(), ReservationStatus::Approved);

        // Cancel 1 hour before start also fails
        ASSERT_FALSE(sm.cancel(resAppLate, "u-s01", eventStart - 3600).success);
        ASSERT_EQ(resAppLate.getStatus(), ReservationStatus::Approved);

        // Forbidden cancellations from terminal/active states
        Facility fac("FAC-104", "Seminar Room", "East Hall", 25);
        fac.setStatus(RoomStatus::Setup);
        ASSERT_TRUE(sm.checkIn(resAppLate, fac, eventStart).success);
        ASSERT_EQ(resAppLate.getStatus(), ReservationStatus::InProgress);
        ASSERT_FALSE(sm.cancel(resAppLate, "u-s01", eventStart + 60).success);

        ASSERT_TRUE(sm.conclude(resAppLate, fac, eventEnd).success);
        ASSERT_EQ(resAppLate.getStatus(), ReservationStatus::Completed);
        ASSERT_FALSE(sm.cancel(resAppLate, "u-s01", eventEnd + 60).success);
    }

    // -------------------------------------------------------------------------
    // 10. Negative Testing of Invalid Reservation Transitions
    // -------------------------------------------------------------------------
    void testInvalidReservationTransitions() {
        using namespace ::domain;
        using namespace ::state;

        ReservationStateMachine sm;

        // Structural transitions check: canTransition matrix
        // Self transitions are false
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Draft, ReservationStatus::Draft));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Submitted, ReservationStatus::Submitted));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::UnderReview));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::SecurityReview, ReservationStatus::SecurityReview));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Approved, ReservationStatus::Approved));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::InProgress, ReservationStatus::InProgress));

        // Illegal backward transitions
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Approved, ReservationStatus::Draft));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Approved, ReservationStatus::Submitted));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::InProgress, ReservationStatus::Approved));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Completed, ReservationStatus::InProgress));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Rejected, ReservationStatus::UnderReview));

        // Illegal jump transitions
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Draft, ReservationStatus::Approved));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Draft, ReservationStatus::InProgress));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::Draft, ReservationStatus::Completed));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::InProgress));
        ASSERT_FALSE(sm.canTransition(ReservationStatus::UnderReview, ReservationStatus::Completed));

        // Transitions out of terminal states are all false
        for (auto term : {ReservationStatus::Completed, ReservationStatus::Cancelled, ReservationStatus::Rejected, ReservationStatus::Expired}) {
            ASSERT_FALSE(sm.canTransition(term, ReservationStatus::Draft));
            ASSERT_FALSE(sm.canTransition(term, ReservationStatus::Submitted));
            ASSERT_FALSE(sm.canTransition(term, ReservationStatus::UnderReview));
            ASSERT_FALSE(sm.canTransition(term, ReservationStatus::Approved));
        }

        // Operational Negative Tests
        std::time_t now = makeTime(2026, 9, 1, 9, 0);
        std::time_t eventStart = makeTime(2026, 9, 5, 14, 0);
        std::time_t eventEnd   = makeTime(2026, 9, 5, 16, 0);

        Reservation res("RES-NEG-01", "u-s01", "FAC-104", "Negative Test", "Meeting", 15, eventStart, eventEnd, now);

        // Submit with schedule conflict fails
        TransitionResult rSubConflict = sm.submit(res, false, now);
        ASSERT_FALSE(rSubConflict.success);
        ASSERT_EQ(res.getStatus(), ReservationStatus::Draft);

        // Submit from non-Draft state fails
        res.setStatus(ReservationStatus::Submitted);
        ASSERT_FALSE(sm.submit(res, true, now).success);

        // Route to review from non-Submitted state fails
        res.setStatus(ReservationStatus::Draft);
        ASSERT_FALSE(sm.routeToReview(res, now).success);

        // Direct approve from non-UnderReview state fails
        ASSERT_FALSE(sm.approve(res, "u-a01", "LGTM", now).success);

        // Check-in on non-Approved reservation fails
        Facility fac("FAC-104", "Seminar Room", "East Hall", 25);
        ASSERT_FALSE(sm.checkIn(res, fac, eventStart).success);

        // Check-in before event start time fails
        res.setStatus(ReservationStatus::Approved);
        ASSERT_FALSE(sm.checkIn(res, fac, eventStart - 60).success);

        // Check-in with facility ID mismatch fails
        Facility wrongFac("FAC-999", "Wrong Room", "East Hall", 25);
        TransitionResult rMismatch = sm.checkIn(res, wrongFac, eventStart);
        ASSERT_FALSE(rMismatch.success);
        ASSERT_EQ(rMismatch.message, "Facility ID mismatch");

        // Check-in when facility is OutOfService or EmergencyHold fails
        fac.setStatus(RoomStatus::OutOfService);
        TransitionResult rOos = sm.checkIn(res, fac, eventStart);
        ASSERT_FALSE(rOos.success);
        ASSERT_EQ(rOos.message, "Cannot check into facility under maintenance or emergency lockdown");

        fac.setStatus(RoomStatus::EmergencyHold);
        TransitionResult rEmerg = sm.checkIn(res, fac, eventStart);
        ASSERT_FALSE(rEmerg.success);
        ASSERT_EQ(rEmerg.message, "Cannot check into facility under maintenance or emergency lockdown");

        // Conclude before end time fails
        fac.setStatus(RoomStatus::Occupied);
        res.setStatus(ReservationStatus::InProgress);
        ASSERT_FALSE(sm.conclude(res, fac, eventEnd - 60).success);

        // Rejection from terminal states fails
        res.setStatus(ReservationStatus::Completed);
        ASSERT_FALSE(sm.reject(res, "u-a01", "Cannot reject completed", now).success);
        res.setStatus(ReservationStatus::Cancelled);
        ASSERT_FALSE(sm.reject(res, "u-a01", "Cannot reject cancelled", now).success);
        res.setStatus(ReservationStatus::Rejected);
        ASSERT_FALSE(sm.reject(res, "u-a01", "Already rejected", now).success);
    }

    // -------------------------------------------------------------------------
    // 11. Facility Cyclic Operational Cycle & Back-to-Back Fast-Track
    // -------------------------------------------------------------------------
    void testFacilityOperationalCycle() {
        using namespace ::domain;
        using namespace ::state;

        FacilityStateMachine fsm;
        NotificationService::getInstance().clear();

        Facility fac("FAC-201", "Multipurpose Room", "Student Union", 100);

        // 1. Commission Room -> Available
        FacilityTransitionResult rComm = fsm.commissionRoom(fac);
        ASSERT_TRUE(rComm.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Available);
        ASSERT_FALSE(fac.areDoorsLocked());

        // Setup from Available directly is invalid
        ASSERT_FALSE(fsm.setupWindowOpens(fac).success);

        // 2. Reservation Confirmed -> Reserved
        FacilityTransitionResult rConf = fsm.reservationConfirmed(fac);
        ASSERT_TRUE(rConf.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Reserved);

        // Check-in directly from Reserved is invalid
        ASSERT_FALSE(fsm.eventCheckIn(fac, true).success);

        // 3. Setup Window Opens -> Setup
        FacilityTransitionResult rSetup = fsm.setupWindowOpens(fac);
        ASSERT_TRUE(rSetup.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Setup);

        // 4. Event Check-In: failure without custodial clearance
        FacilityTransitionResult rFailCheckIn = fsm.eventCheckIn(fac, false);
        ASSERT_FALSE(rFailCheckIn.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Setup);

        // Check-in with custodial clearance -> Occupied (smart doors unlocked)
        FacilityTransitionResult rCheckIn = fsm.eventCheckIn(fac, true);
        ASSERT_TRUE(rCheckIn.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Occupied);
        ASSERT_FALSE(fac.areDoorsLocked());

        // 5. Checkout Concluded -> Turnover (doors locked, custodial notice dispatched)
        FacilityTransitionResult rCheckout = fsm.checkoutConcluded(fac);
        ASSERT_TRUE(rCheckout.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Turnover);
        ASSERT_TRUE(fac.areDoorsLocked());
        ASSERT_TRUE(NotificationService::getInstance().count() >= 1);

        // 6. Custodial Inspection: failure when not ready for use
        FacilityTransitionResult rFailInsp = fsm.inspectionPassed(fac, false);
        ASSERT_FALSE(rFailInsp.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Turnover);

        // Inspection passed -> Available (doors unlocked)
        FacilityTransitionResult rInsp = fsm.inspectionPassed(fac, true);
        ASSERT_TRUE(rInsp.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Available);
        ASSERT_FALSE(fac.areDoorsLocked());

        // 7. Back-to-Back Fast-Track Booking (Turnover -> Setup)
        ASSERT_TRUE(fsm.reservationConfirmed(fac).success);
        ASSERT_TRUE(fsm.setupWindowOpens(fac).success);
        ASSERT_TRUE(fsm.eventCheckIn(fac, true).success);
        ASSERT_TRUE(fsm.checkoutConcluded(fac).success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Turnover);

        // Fast-track transition directly to Setup for consecutive event
        FacilityTransitionResult rB2B = fsm.backToBackBooking(fac);
        ASSERT_TRUE(rB2B.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Setup);

        // Back-to-back from non-Turnover status fails
        ASSERT_FALSE(fsm.backToBackBooking(fac).success);
    }

    // -------------------------------------------------------------------------
    // 12. Facility Maintenance Blackouts & Emergency Preemption
    // -------------------------------------------------------------------------
    void testFacilityMaintenanceAndEmergency() {
        using namespace ::domain;
        using namespace ::state;

        FacilityStateMachine fsm;
        NotificationService::getInstance().clear();

        Facility fac("FAC-301", "Robotics Laboratory", "Engineering Hall", 40);

        // 1. Report Damage from Available -> OutOfService (doors locked, notice sent)
        ASSERT_EQ(fac.getStatus(), RoomStatus::Available);
        FacilityTransitionResult rDam = fsm.reportDamage(fac, "Hydraulic fluid leak on test bench");
        ASSERT_TRUE(rDam.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::OutOfService);
        ASSERT_TRUE(fac.areDoorsLocked());
        ASSERT_TRUE(NotificationService::getInstance().count() >= 1);

        // Booking confirmation while OutOfService fails
        ASSERT_FALSE(fsm.reservationConfirmed(fac).success);

        // Repair certification failed
        FacilityTransitionResult rRepFail = fsm.repairCertified(fac, false);
        ASSERT_FALSE(rRepFail.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::OutOfService);

        // Repair certification succeeded -> Available (doors unlocked)
        FacilityTransitionResult rRepPass = fsm.repairCertified(fac, true);
        ASSERT_TRUE(rRepPass.success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Available);
        ASSERT_FALSE(fac.areDoorsLocked());

        // Report damage from Setup and Turnover
        ASSERT_TRUE(fsm.reservationConfirmed(fac).success);
        ASSERT_TRUE(fsm.setupWindowOpens(fac).success);
        ASSERT_TRUE(fsm.reportDamage(fac, "Projector blown bulb").success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::OutOfService);
        ASSERT_TRUE(fsm.repairCertified(fac, true).success);
        ASSERT_EQ(fac.getStatus(), RoomStatus::Available);

        // 2. Emergency Preemption: Can preempt from ANY active state
        for (auto st : {RoomStatus::Available, RoomStatus::Reserved, RoomStatus::Setup, RoomStatus::Occupied, RoomStatus::Turnover, RoomStatus::OutOfService}) {
            fac.setStatus(st);
            FacilityTransitionResult rEmerg = fsm.declareEmergency(fac, "Campus Weather / Severe Storm Alert");
            ASSERT_TRUE(rEmerg.success);
            ASSERT_EQ(fac.getStatus(), RoomStatus::EmergencyHold);
            ASSERT_TRUE(fac.areDoorsLocked());

            // All-clear without security approval fails
            ASSERT_FALSE(fsm.allClearGiven(fac, false).success);
            ASSERT_EQ(fac.getStatus(), RoomStatus::EmergencyHold);

            // All-clear with security approval succeeds -> Available (doors unlocked)
            ASSERT_TRUE(fsm.allClearGiven(fac, true).success);
            ASSERT_EQ(fac.getStatus(), RoomStatus::Available);
            ASSERT_FALSE(fac.areDoorsLocked());
        }

        // Structural transitions check: canTransition matrix for Facility
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Available, RoomStatus::Reserved));
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Reserved, RoomStatus::Setup));
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Setup, RoomStatus::Occupied));
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Occupied, RoomStatus::Turnover));
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Turnover, RoomStatus::Available));
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Turnover, RoomStatus::Setup)); // Back-to-back
        ASSERT_TRUE(fsm.canTransition(RoomStatus::Turnover, RoomStatus::OutOfService));
        ASSERT_TRUE(fsm.canTransition(RoomStatus::OutOfService, RoomStatus::Available));

        // EmergencyHold can be entered from ANY status
        for (auto s : {RoomStatus::Available, RoomStatus::Reserved, RoomStatus::Setup, RoomStatus::Occupied, RoomStatus::Turnover, RoomStatus::OutOfService, RoomStatus::EmergencyHold}) {
            if (s != RoomStatus::EmergencyHold) {
                ASSERT_TRUE(fsm.canTransition(s, RoomStatus::EmergencyHold));
            }
        }

        // Invalid transitions
        ASSERT_FALSE(fsm.canTransition(RoomStatus::Available, RoomStatus::Occupied));
        ASSERT_FALSE(fsm.canTransition(RoomStatus::Occupied, RoomStatus::Available));
        ASSERT_FALSE(fsm.canTransition(RoomStatus::Reserved, RoomStatus::Occupied));
        ASSERT_FALSE(fsm.canTransition(RoomStatus::Available, RoomStatus::Available));
    }

    // -------------------------------------------------------------------------
    // 13. Central BookingSystem End-to-End Coordinator Integration
    // -------------------------------------------------------------------------
    void testBookingSystemIntegration() {
        using namespace ::service;
        using namespace ::domain;

        BookingSystem sys;
        std::time_t baseTime = BookingSystem::makeTime(2026, 9, 1, 9, 0, 0);
        std::time_t start    = BookingSystem::makeTime(2026, 9, 5, 14, 0, 0);
        std::time_t end      = BookingSystem::makeTime(2026, 9, 5, 16, 0, 0);
        sys.setTime(baseTime);

        // 1. Verify Default Repository Seeding
        auto facilities = sys.getFacilities();
        ASSERT_EQ(facilities.size(), static_cast<size_t>(4));
        ASSERT_TRUE(sys.getFacility("FAC-101").has_value());
        ASSERT_TRUE(sys.getFacility("FAC-102").has_value());
        ASSERT_TRUE(sys.getFacility("FAC-103").has_value());
        ASSERT_TRUE(sys.getFacility("FAC-104").has_value());

        auto users = sys.getUsers();
        ASSERT_TRUE(users.size() >= static_cast<size_t>(6));
        ASSERT_NE(sys.getUser("USR-001"), nullptr);
        ASSERT_NE(sys.getUser("USR-005"), nullptr);
        ASSERT_NE(sys.getUser("USR-006"), nullptr);

        // 2. Simulated Clock Operations
        ASSERT_EQ(sys.getCurrentTime(), baseTime);
        sys.advanceTime(1800); // 30 minutes
        ASSERT_EQ(sys.getCurrentTime(), baseTime + 1800);
        ASSERT_EQ(BookingSystem::formatTime(baseTime), "2026-09-01 09:00:00");
        sys.setTime(baseTime);

        // 3. End-to-End Coordinated Booking Lifecycle
        std::string resId = sys.createReservation(
            "USR-001", "FAC-104", "Autonomous Robotics Lab", "Workshop", 18, start, end,
            {EquipmentRequest("EQ-01", "Projector", 1, false)},
            {SupportRequest("SUP-01", "AV Support", "Need tech", false, "")},
            "RES-INT-101"
        );
        ASSERT_EQ(resId, "RES-INT-101");
        auto rOpt = sys.getReservation(resId);
        ASSERT_TRUE(rOpt.has_value());
        ASSERT_EQ(rOpt->getStatus(), ReservationStatus::Draft);

        // Submit
        ASSERT_TRUE(sys.submitReservation(resId).success);
        ASSERT_EQ(sys.getReservation(resId)->getStatus(), ReservationStatus::Submitted);

        // Route
        ASSERT_TRUE(sys.routeToReview(resId).success);
        ASSERT_EQ(sys.getReservation(resId)->getStatus(), ReservationStatus::UnderReview);
        ASSERT_EQ(sys.getPendingApprovals().size(), static_cast<size_t>(1));

        // Approve
        ASSERT_TRUE(sys.approveReservation(resId, "USR-005", "Approved by Event Approver").success);
        ASSERT_EQ(sys.getReservation(resId)->getStatus(), ReservationStatus::Approved);
        ASSERT_EQ(sys.getPendingApprovals().size(), static_cast<size_t>(0));
        ASSERT_EQ(sys.getFacility("FAC-104")->getStatus(), RoomStatus::Reserved);

        // Staging window
        ASSERT_TRUE(sys.openSetupWindow("FAC-104").success);
        ASSERT_EQ(sys.getFacility("FAC-104")->getStatus(), RoomStatus::Setup);

        // Advance to event start -> check in
        sys.setTime(start);
        ASSERT_TRUE(sys.checkInReservation(resId).success);
        ASSERT_EQ(sys.getReservation(resId)->getStatus(), ReservationStatus::InProgress);
        ASSERT_EQ(sys.getFacility("FAC-104")->getStatus(), RoomStatus::Occupied);
        ASSERT_FALSE(sys.getFacility("FAC-104")->areDoorsLocked());

        // Advance to event end -> conclude
        sys.setTime(end);
        ASSERT_TRUE(sys.concludeReservation(resId).success);
        ASSERT_EQ(sys.getReservation(resId)->getStatus(), ReservationStatus::Completed);
        ASSERT_EQ(sys.getFacility("FAC-104")->getStatus(), RoomStatus::Turnover);
        ASSERT_TRUE(sys.getFacility("FAC-104")->areDoorsLocked());

        // Custodial inspection completion
        ASSERT_TRUE(sys.completeInspection("FAC-104").success);
        ASSERT_EQ(sys.getFacility("FAC-104")->getStatus(), RoomStatus::Available);
        ASSERT_FALSE(sys.getFacility("FAC-104")->areDoorsLocked());

        // 4. High-Risk Security Event through BookingSystem
        std::string secId = sys.createReservation(
            "USR-001", "FAC-101", "Campus Mega Hackathon", "Conference", 220, start, end, {}, {}, "RES-INT-SEC"
        );
        ASSERT_TRUE(sys.submitReservation(secId).success);
        ASSERT_TRUE(sys.routeToReview(secId).success);
        ASSERT_TRUE(sys.evaluateSecurity(secId).success);
        ASSERT_EQ(sys.getReservation(secId)->getStatus(), ReservationStatus::SecurityReview);
        ASSERT_EQ(sys.getPendingSecurity().size(), static_cast<size_t>(1));

        // Direct standard approval rejected
        ASSERT_FALSE(sys.approveReservation(secId, "USR-005", "Direct attempt").success);

        // Security clearance by Security Officer USR-006 succeeds
        ASSERT_TRUE(sys.securityClear(secId, "USR-006", "Stationing 2 campus security officers").success);
        ASSERT_EQ(sys.getReservation(secId)->getStatus(), ReservationStatus::Approved);
        ASSERT_EQ(sys.getPendingSecurity().size(), static_cast<size_t>(0));

        // 5. Revision Loop and Slot Hold Protection
        sys.setTime(baseTime);
        std::string revId = sys.createReservation(
            "USR-001", "FAC-102", "STEM Workshop", "Workshop", 30, start, end, {}, {}, "RES-INT-REV"
        );
        ASSERT_TRUE(sys.submitReservation(revId).success);
        ASSERT_TRUE(sys.routeToReview(revId).success);
        ASSERT_TRUE(sys.requestChanges(revId, "USR-005", "Provide safety equipment list", baseTime + 86400).success);
        ASSERT_EQ(sys.getReservation(revId)->getStatus(), ReservationStatus::NeedsRevision);

        // Other booking attempting to book overlapping slot during NeedsRevision window MUST FAIL (conflict guard)
        std::string overlapId = sys.createReservation(
            "USR-002", "FAC-102", "Overlapping Session", "Workshop", 20, start, end, {}, {}, "RES-INT-OVERLAP"
        );
        ASSERT_FALSE(sys.submitReservation(overlapId).success);

        // Resubmit succeeds
        ASSERT_TRUE(sys.resubmitReservation(revId).success);
        ASSERT_EQ(sys.getReservation(revId)->getStatus(), ReservationStatus::Submitted);

        // 6. 24-Hour Cancellation Lead-Time Enforcement in BookingSystem
        std::time_t canStart = BookingSystem::makeTime(2026, 9, 10, 14, 0, 0);
        std::time_t canEnd   = BookingSystem::makeTime(2026, 9, 10, 16, 0, 0);
        std::string canId = sys.createReservation("USR-001", "FAC-103", "Concert", "Music", 50, canStart, canEnd, {}, {}, "RES-INT-CAN");
        ASSERT_TRUE(sys.submitReservation(canId).success);
        ASSERT_TRUE(sys.routeToReview(canId).success);
        ASSERT_TRUE(sys.approveReservation(canId, "USR-005", "Approved").success);
        ASSERT_EQ(sys.getReservation(canId)->getStatus(), ReservationStatus::Approved);

        // Attempt cancellation < 24 hours prior (23 hours prior) fails
        sys.setTime(canStart - 23 * 3600);
        ASSERT_FALSE(sys.cancelReservation(canId, "USR-001").success);
        ASSERT_EQ(sys.getReservation(canId)->getStatus(), ReservationStatus::Approved);

        // Attempt cancellation > 24 hours prior (25 hours prior) on new reservation succeeds
        std::string canId2 = sys.createReservation("USR-001", "FAC-103", "Concert 2", "Music", 50, canStart + 86400, canEnd + 86400, {}, {}, "RES-INT-CAN2");
        ASSERT_TRUE(sys.submitReservation(canId2).success);
        ASSERT_TRUE(sys.routeToReview(canId2).success);
        ASSERT_TRUE(sys.approveReservation(canId2, "USR-005", "Approved").success);
        sys.setTime((canStart + 86400) - 25 * 3600);
        ASSERT_TRUE(sys.cancelReservation(canId2, "USR-001").success);
        ASSERT_EQ(sys.getReservation(canId2)->getStatus(), ReservationStatus::Cancelled);

        // 7. Campus Emergency Controls
        ASSERT_TRUE(sys.declareCampusEmergency("Drill lockdown").success);
        for (const auto& f : sys.getFacilities()) {
            ASSERT_EQ(f.getStatus(), RoomStatus::EmergencyHold);
            ASSERT_TRUE(f.areDoorsLocked());
        }

        ASSERT_TRUE(sys.clearCampusEmergency().success);
        for (const auto& f : sys.getFacilities()) {
            ASSERT_EQ(f.getStatus(), RoomStatus::Available);
            ASSERT_FALSE(f.areDoorsLocked());
        }

        // 7. JSON Serialization
        std::string json = sys.toJson();
        ASSERT_FALSE(json.empty());
        ASSERT_TRUE(json.find("\"facilities\":") != std::string::npos);
        ASSERT_TRUE(json.find("\"reservations\":") != std::string::npos);
        ASSERT_TRUE(json.find("\"users\":") != std::string::npos);
        ASSERT_TRUE(json.find("\"auditLog\":") != std::string::npos);
    }

    // -------------------------------------------------------------------------
    // 14. Embedded Web Dashboard & REST API Endpoints
    // -------------------------------------------------------------------------
    void testWebDashboardAndEndpoints() {
        auto sys = std::make_shared<service::BookingSystem>();
        smartcampus::web::HttpServer server(sys, 18080);

        // 1. GET / returns 200 with HTML and Content-Type: text/html
        std::string resHome = server.handleHttpRequest("GET / HTTP/1.1\r\nHost: localhost:18080\r\n\r\n");
        ASSERT_TRUE(resHome.find("HTTP/1.1 200 OK") != std::string::npos);
        ASSERT_TRUE(resHome.find("Content-Type: text/html") != std::string::npos);
        ASSERT_TRUE(resHome.find("<!DOCTYPE html>") != std::string::npos);

        // 2. GET /api/state returns 200 with JSON and Content-Type: application/json
        std::string resState = server.handleHttpRequest("GET /api/state HTTP/1.1\r\nHost: localhost:18080\r\n\r\n");
        ASSERT_TRUE(resState.find("HTTP/1.1 200 OK") != std::string::npos);
        ASSERT_TRUE(resState.find("Content-Type: application/json") != std::string::npos);
        ASSERT_TRUE(resState.find("\"facilities\"") != std::string::npos);
        ASSERT_TRUE(resState.find("\"reservations\"") != std::string::npos);
        ASSERT_TRUE(resState.find("\"currentTime\"") != std::string::npos);

        // 3. POST /api/advance-time advances time
        std::time_t tBefore = sys->getCurrentTime();
        std::string resAdv = server.handleHttpRequest(
            "POST /api/advance-time HTTP/1.1\r\nHost: localhost:18080\r\nContent-Type: application/json\r\nContent-Length: 17\r\n\r\n{\"seconds\": 1800}"
        );
        ASSERT_TRUE(resAdv.find("HTTP/1.1 200 OK") != std::string::npos);
        ASSERT_TRUE(resAdv.find("\"success\": true") != std::string::npos);
        std::time_t tAfter = sys->getCurrentTime();
        ASSERT_EQ(tAfter, tBefore + 1800);

        // 4. Negative Content-Length returns 400
        std::string resNegCl = server.handleHttpRequest(
            "POST /api/advance-time HTTP/1.1\r\nHost: localhost:18080\r\nContent-Length: -50\r\n\r\n"
        );
        ASSERT_TRUE(resNegCl.find("HTTP/1.1 400 Bad Request") != std::string::npos);

        // 5. Content-Length > 10MB returns 413
        std::string resBigCl = server.handleHttpRequest(
            "POST /api/advance-time HTTP/1.1\r\nHost: localhost:18080\r\nContent-Length: 15000000\r\n\r\n"
        );
        ASSERT_TRUE(resBigCl.find("HTTP/1.1 413 Payload Too Large") != std::string::npos);

        // 6. 404 for unknown route
        std::string resUnknown = server.handleHttpRequest(
            "GET /api/unknown-endpoint HTTP/1.1\r\nHost: localhost:18080\r\n\r\n"
        );
        ASSERT_TRUE(resUnknown.find("HTTP/1.1 404 Not Found") != std::string::npos);
    }

    // -------------------------------------------------------------------------
    // Self-Test for Assertion Macros
    // -------------------------------------------------------------------------
    void testAssertionMacros() {
        ASSERT_TRUE(true);
        ASSERT_FALSE(false);
        ASSERT_EQ(42, 42);
        ASSERT_NE(42, 99);
        ASSERT_EQ(std::string("hello"), "hello");
        ASSERT_NE(std::string("hello"), "world");

        auto thrower = []() { throw std::runtime_error("intentional error"); };
        ASSERT_THROWS(thrower(), std::runtime_error);

        auto throwCustom = []() { throw std::invalid_argument("invalid param"); };
        ASSERT_THROWS(throwCustom(), std::invalid_argument);
    }

    // -------------------------------------------------------------------------
    // Execution Runner
    // -------------------------------------------------------------------------
    int runAll(bool verbose = true) {
        using namespace style;

        AssertionTracker::reset();
        auto startSuiteTime = std::chrono::high_resolution_clock::now();

        struct TestCase {
            std::string name;
            void (TestSuite::*func)();
        };

        std::vector<TestCase> cases = {
            {"testUserHierarchyAndPolymorphism",      &TestSuite::testUserHierarchyAndPolymorphism},
            {"testFacilityOperations",                &TestSuite::testFacilityOperations},
            {"testReservationComposition",            &TestSuite::testReservationComposition},
            {"testNotificationService",               &TestSuite::testNotificationService},
            {"testReservationHappyPath",              &TestSuite::testReservationHappyPath},
            {"testSoftLockExpiration",                &TestSuite::testSoftLockExpiration},
            {"testSecurityReviewEscalation",          &TestSuite::testSecurityReviewEscalation},
            {"testRevisionAndResubmissionLoop",       &TestSuite::testRevisionAndResubmissionLoop},
            {"testCancellationRulesAndLeadTime",      &TestSuite::testCancellationRulesAndLeadTime},
            {"testInvalidReservationTransitions",     &TestSuite::testInvalidReservationTransitions},
            {"testFacilityOperationalCycle",          &TestSuite::testFacilityOperationalCycle},
            {"testFacilityMaintenanceAndEmergency",   &TestSuite::testFacilityMaintenanceAndEmergency},
            {"testBookingSystemIntegration",          &TestSuite::testBookingSystemIntegration},
            {"testWebDashboardAndEndpoints",          &TestSuite::testWebDashboardAndEndpoints}
        };

        if (verbose) {
            std::cout << "\n" << BOLD << CYAN << "================================================================================" << RESET << "\n";
            std::cout << BOLD << WHITE << "      SMART CAMPUS EVENT & FACILITY BOOKING SYSTEM - VERIFICATION SUITE       " << RESET << "\n";
            std::cout << BOLD << CYAN << "================================================================================" << RESET << "\n\n";
        }

        size_t passedTests = 0;
        size_t failedTests = 0;
        std::vector<std::string> failures;

        for (const auto& tc : cases) {
            auto t0 = std::chrono::high_resolution_clock::now();
            bool pass = false;
            std::string errMsg;

            try {
                (this->*(tc.func))();
                pass = true;
            } catch (const TestAssertionException& e) {
                errMsg = std::string(e.what()) + " at " + e.getFile() + ":" + std::to_string(e.getLine());
            } catch (const std::exception& e) {
                errMsg = std::string("Unexpected std::exception: ") + e.what();
            } catch (...) {
                errMsg = "Unexpected unknown exception caught";
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

            if (pass) {
                passedTests++;
                if (verbose) {
                    std::cout << "  " << GREEN << "[PASS]" << RESET << " "
                              << std::left << std::setw(42) << tc.name
                              << DIM << " (" << std::fixed << std::setprecision(2) << elapsedMs << " ms)" << RESET << "\n";
                }
            } else {
                failedTests++;
                failures.push_back(tc.name + " -> " + errMsg);
                if (verbose) {
                    std::cout << "  " << RED << "[FAIL]" << RESET << " "
                              << std::left << std::setw(42) << tc.name
                              << RED << " (" << errMsg << ")" << RESET << "\n";
                }
            }
        }

        auto endSuiteTime = std::chrono::high_resolution_clock::now();
        double totalElapsedMs = std::chrono::duration<double, std::milli>(endSuiteTime - startSuiteTime).count();
        uint64_t totalAssertions = AssertionTracker::assertionCount.load(std::memory_order_relaxed);
        uint64_t passedAssertions = AssertionTracker::assertionPassCount.load(std::memory_order_relaxed);

        if (verbose) {
            std::cout << "\n" << BOLD << CYAN << "================================================================================" << RESET << "\n";
            std::cout << BOLD << WHITE << "                            TEST EXECUTION SUMMARY                              " << RESET << "\n";
            std::cout << BOLD << CYAN << "================================================================================" << RESET << "\n";
            std::cout << "  Total Test Suites:     " << BOLD << cases.size() << RESET << "\n";
            std::cout << "  Passed Test Suites:    " << GREEN << BOLD << passedTests << RESET << "\n";
            std::cout << "  Failed Test Suites:    " << (failedTests == 0 ? GREEN : RED) << BOLD << failedTests << RESET << "\n";
            std::cout << "  Total Assertions Run:  " << BOLD << totalAssertions << RESET << "\n";
            std::cout << "  Assertions Passed:     " << GREEN << BOLD << passedAssertions << RESET << "\n";
            std::cout << "  Assertions Failed:     " << (totalAssertions == passedAssertions ? GREEN : RED) << BOLD << (totalAssertions - passedAssertions) << RESET << "\n";
            std::cout << "  Total Elapsed Time:    " << YELLOW << std::fixed << std::setprecision(2) << totalElapsedMs << " ms" << RESET << "\n";
            std::cout << "--------------------------------------------------------------------------------\n";

            if (failedTests == 0) {
                std::cout << "  " << BG_GREEN << BOLD << WHITE << " VERIFICATION RESULT: 100% ASSERTIONS PASSED (ALL TESTS SUCCESSFUL) " << RESET << "\n";
            } else {
                std::cout << "  " << BG_RED << BOLD << WHITE << " VERIFICATION RESULT: FAILED WITH " << failedTests << " FAILING SUITES " << RESET << "\n";
                for (const auto& f : failures) {
                    std::cout << "    * " << RED << f << RESET << "\n";
                }
            }
            std::cout << BOLD << CYAN << "================================================================================" << RESET << "\n\n";
        } else {
            // Terse one-line summary mode
            if (failedTests == 0) {
                std::cout << "[TEST SUITE] PASS: " << passedTests << "/" << cases.size()
                          << " suites (" << passedAssertions << " assertions, 0 errors, "
                          << std::fixed << std::setprecision(2) << totalElapsedMs << " ms)\n";
            } else {
                std::cerr << "[TEST SUITE] FAIL: " << failedTests << " failures, "
                          << passedTests << " passed ("
                          << std::fixed << std::setprecision(2) << totalElapsedMs << " ms)\n";
            }
        }

        return (failedTests == 0) ? 0 : 1;
    }

    // Static convenience method allowing TestSuite::runAll(verbose)
    static int run(bool verbose = true) {
        TestSuite suite;
        return suite.runAll(verbose);
    }
};

} // namespace smartcampus::test
