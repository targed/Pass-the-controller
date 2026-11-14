# C++ Development Guide: Smart Campus Booking System

This guide provides technical reference documentation for the Smart Campus Event and Facility Booking System codebase. It details the directory architecture, class implementations, state transition specifications, embedded REST API routes, test harness mechanics, and compilation workflows.

---

## 1. System Architecture and Directory Layout

The application is structured into clean modular namespaces under `src/`. All header files are header-only or paired with clean implementation units, designed for rapid compilation under C++20.

```
Pass-the-controller/
├── .devcontainer/
│   └── devcontainer.json               # Ubuntu 24.04 Dev Container configuration
├── CMakeLists.txt                      # CMake build script targeting C++20
├── README.md                           # Project overview, launch guides, and quickstart
├── Docs/
│   ├── cpp-development.md              # Developer reference (this document)
│   ├── AI ML Class and State Models.pdf
│   ├── Capstone Class Model and State Models.pdf
│   └── superpowers/                    # Design specifications and implementation plans
└── src/
    ├── main.cpp                        # Entry point, CLI argument parser, signal handlers
    ├── domain/                         # Core domain entities and aggregates
    │   ├── User.hpp                    # User abstract base and 4 role specializations
    │   ├── Facility.hpp                # Facility entity, RoomStatus enum, door lock states
    │   ├── Reservation.hpp             # Reservation aggregate root, child entities, enums
    │   └── NotificationService.hpp     # Dispatch queue, alert routing, and audit logs
    ├── state/                          # Formal state machine engines
    │   ├── ReservationStateMachine.hpp # 11-state reservation lifecycle coordinator
    │   └── FacilityStateMachine.hpp    # 7-state cyclic facility operational coordinator
    ├── service/                        # System orchestration and presentation scenarios
    │   ├── BookingSystem.hpp           # Central service repository, clock, and transactions
    │   └── ScenarioRunner.hpp          # Automated runners for 6 demonstration scenarios
    ├── test/                           # Automated verification
    │   └── TestSuite.hpp               # 14-suite test harness with 609 assertions
    └── web/                            # Presentation and network tier
        ├── HttpServer.hpp              # Cross-platform socket HTTP server and REST router
        └── WebDashboard.hpp            # Inlined single-page HTML/CSS/JavaScript dashboard
```

### Module Responsibilities

1. **`domain/`**: Pure domain models without direct dependencies on networking or presentation. Models entities, value objects, and domain invariants such as booking quotas and cancellation lead times.
2. **`state/`**: Encapsulates state transitions, guard conditions, and transition side effects. Uses explicit enum state representations and validates all transitions against business rules.
3. **`service/`**: Coordinates domain models and state machines. Manages system time simulation, seeds initial campus facilities and mock users, and exposes thread-safe operations.
4. **`web/`**: Provides HTTP transport using native OS sockets (Winsock on Windows, POSIX sockets on Linux/macOS) with zero external library dependencies. Routes REST API requests to `BookingSystem`.
5. **`test/`**: Custom, zero-dependency unit test framework providing assertion macros, timing instrumentation, and colorized terminal summaries.

---

## 2. Domain Model Specifications

### 2.1 User Hierarchy (`domain/User.hpp`)
All actors inherit from the abstract base class `domain::User`:

* **`User`**: Base class containing `userId`, `fullName`, `email`, `department`, and an authentication token string. Provides virtual methods `getRole()`, `getRoleName()`, and `isExpeditedEligible()`.
* **`StudentOrgRepresentative`**: Associated with an `organizationName`. Restricts concurrent active bookings (default maximum: 5) to prevent room hoarding.
* **`FacultyStaffMember`**: Associated with an `employeeId` and department. Sets `isExpeditedEligible()` to `true`, allowing curricular reservations to bypass initial triage queues.
* **`FacilityCoordinator`**: Associated with a list of `assignedBuildings`. Exposes methods to toggle maintenance blackouts, inspect rooms, and certify repairs.
* **`EventApprover`**: Holds an `approvalTier` integer (Tier 1 for departmental spaces, Tier 2 for campus auditoriums, Tier 3 for executive facilities). Evaluates policy compliance.

### 2.2 Facility Entity (`domain/Facility.hpp`)
Represents physical campus rooms.
* **Attributes:** `facilityId`, `name`, `building`, `capacity`, `hourlyRate`, and `supportedEquipment` (`std::vector<std::string>`).
* **Operational State (`RoomStatus`):**
  * `Available` (0)
  * `Reserved` (1)
  * `Setup` (2)
  * `Occupied` (3)
  * `Turnover` (4)
  * `OutOfService` (5)
  * `EmergencyHold` (6)
* **Access Control:** `areDoorsLocked()` tracks physical electronic door locks. Doors are unlocked automatically when transitioning to `Occupied` and locked in `Turnover`, `EmergencyHold`, or `OutOfService`.

### 2.3 Reservation Aggregate Root (`domain/Reservation.hpp`)
Coordinates booking metadata, child entities, and status progression:
* **Status Enum (`ReservationStatus`):** `Draft` (0), `Submitted` (1), `UnderReview` (2), `SecurityReview` (3), `NeedsRevision` (4), `Expired` (5), `Approved` (6), `InProgress` (7), `Completed` (8), `Rejected` (9), `Cancelled` (10).
* **Child Entities:**
  * **`EquipmentRequest`:** `requestId`, `equipmentType`, `quantity`, `isFulfilled`.
  * **`SupportRequest`:** `requestId`, `supportType` (Custodial, AV, Security), `notes`, `isAssigned`, `assignedOfficerId`.
  * **`Approval`:** `approvalId`, `approverId`, `decision` (Approved, Rejected, ChangesRequested), `comments`, `decidedAt`.
* **Guards & Helpers:**
  * `isHighRisk()`: Returns `true` if `expectedAttendance > 100` or if the start time is outside 08:00 to 20:00.
  * `canCancel(currentTime)`: Returns `true` if current status is prior to `Approved`, or if status is `Approved` and `(startTime - currentTime) >= 86400` (24-hour lead time).

---

## 3. State Transition Tables and Guard Conditions

### 3.1 Reservation Lifecycle State Machine

The reservation lifecycle transitions across 11 discrete states:

| Current State | Event / Trigger | Target State | Guard Condition | Actions Executed |
| :--- | :--- | :--- | :--- | :--- |
| `Draft` | `submitReservation` | `Submitted` | Facility available with no schedule overlap | Soft-lock placed on facility (`holdExpiresAt = now + 900s`); notification dispatched |
| `Submitted` | `routeToReview` | `UnderReview` | Requester or system routes to review queue | Request assigned to approver triage pool |
| `Submitted` | `advanceTime` | `Expired` | `currentTime > holdExpiresAt` | Soft-lock released; room marked available; requester notified |
| `UnderReview` | `evaluateSecurity` | `SecurityReview` | `headcount > 100` or after-hours schedule | High-risk alert routed to campus police; security officer required |
| `UnderReview` | `approveReservation` | `Approved` | `headcount <= 100` and normal operating hours | Calendar slot locked permanently; facility marked `Reserved`; approval record stored |
| `UnderReview` | `rejectReservation` | `Rejected` | Written justification provided | Hold released; rejection record stored; requester notified |
| `UnderReview` | `requestChanges` | `NeedsRevision` | Specific adjustment comments provided | Revision deadline set (`now + 48h`); notification dispatched |
| `SecurityReview` | `securityClear` | `Approved` | Police officer assigned and clearance notes entered | Calendar slot locked permanently; facility marked `Reserved`; approval record stored |
| `SecurityReview` | `securityDeny` | `Rejected` | Safety or staffing denial reason provided | Hold released; denial record stored; requester notified |
| `NeedsRevision` | `resubmitReservation` | `Submitted` | `currentTime <= revisionDeadline` | Soft-lock renewed; request moved back to `Submitted` |
| `NeedsRevision` | `advanceTime` | `Expired` | `currentTime > revisionDeadline` | Hold released; reservation marked expired; audit log updated |
| `NeedsRevision` | `cancelReservation` | `Cancelled` | Requester matches reservation owner | Hold released; cancellation confirmed |
| `Approved` | `checkInReservation` | `InProgress` | `currentTime >= startTime` and `currentTime < endTime` | Facility status set to `Occupied`; smart doors unlocked for attendees |
| `Approved` | `cancelReservation` | `Cancelled` | Requester matches owner and `(startTime - now) >= 86400` | Facility restored to `Available`; calendar hold released |
| `Approved` | `cancelReservation` | (Blocked) | `(startTime - now) < 86400` | Cancellation rejected by 24-hour lead time guard |
| `InProgress` | `concludeReservation` | `Completed` | Event end time reached or manual conclusion | Smart doors locked; facility status set to `Turnover`; custodial dispatch sent |

### 3.2 Facility Operational Cycle State Machine

The physical room operational cycle manages room readiness and access control across 7 continuous states:

| Current State | Event / Trigger | Target State | Guard Condition | Actions Executed |
| :--- | :--- | :--- | :--- | :--- |
| `Available` | `reservationConfirmed` | `Reserved` | Valid approved reservation exists on calendar | Calendar slot locked; smart doors remain secured |
| `Reserved` | `openSetupWindow` | `Setup` | Within staging window prior to event start | Staging crew alerted; doors unlocked for setup staff |
| `Reserved` / `Setup` | `checkIn` | `Occupied` | Event start time reached and organizer checked in | Smart doors unlocked for attendees; venue occupancy logged |
| `Occupied` | `conclude` | `Turnover` | Event conclusion processed | Smart doors locked; custodial turnover dispatch generated |
| `Turnover` | `cleanedAndInspected` | `Available` | Custodial inspection passes without reported damage | Smart doors reset to normal; room unlocked for inspection; bookable |
| `Turnover` | `reportDamage` | `OutOfService` | Equipment breakdown or physical damage reported | Maintenance ticket opened; all upcoming reservations flagged |
| `OutOfService` | `certifyRepair` | `Available` | Facilities team certifies repairs complete | Maintenance ticket resolved; room returned to bookable inventory |
| `[Any State]` | `declareEmergency` | `EmergencyHold` | Campus-wide security alert or severe weather lockdown | All smart doors locked immediately; all reservations suspended |
| `EmergencyHold` | `clearEmergency` | `Available` | Campus safety issues official all-clear | Smart doors restored to standard operation; rooms re-opened |

---

## 4. Embedded REST API Reference

The embedded HTTP server listens on port 8080 by default. It responds to standard JSON and form payloads, supporting both application integration and the single-page dashboard.

### 4.1 Endpoints Summary Table

| Method | Path | Description | Parameters |
| :--- | :--- | :--- | :--- |
| `GET` | `/` or `/index.html` | Serves single-page web dashboard | None |
| `GET` | `/api/state` | Returns full system state JSON | None |
| `POST` | `/api/book` | Drafts and submits a new reservation | `facilityId`, `requesterId`, `title`, `attendance`, `startTime`, `endTime` |
| `POST` | `/api/advance-time` | Advances simulation clock | `seconds` (default: 900) |
| `POST` | `/api/route-review` | Moves submission into review queue | `id` |
| `POST` | `/api/approve` | Approves reservation | `id`, `approverId`, `comments` |
| `POST` | `/api/reject` | Rejects reservation | `id`, `approverId`, `reason` |
| `POST` | `/api/request-changes` | Requests adjustments from requester | `id`, `approverId`, `notes`, `deadline` |
| `POST` | `/api/resubmit` | Resubmits revised booking | `id` |
| `POST` | `/api/security-clear` | Grants security clearance | `id`, `officerId`, `notes` |
| `POST` | `/api/security-deny` | Denies security clearance | `id`, `officerId`, `reason` |
| `POST` | `/api/check-in` | Checks in and unlocks smart doors | `id` |
| `POST` | `/api/conclude` | Concludes event and locks doors | `id` |
| `POST` | `/api/cancel` | Cancels booking (subject to 24h rule) | `id`, `userId` |
| `POST` | `/api/emergency` | Declares campus-wide lockdown | `reason` |
| `POST` | `/api/all-clear` | Clears lockdown and restores rooms | None |
| `POST` | `/api/report-damage` | Reports damage during turnover | `facilityId`, `issue` |
| `POST` | `/api/certify-repair` | Restores out-of-service facility | `facilityId` |
| `POST` | `/api/scenario` | Executes demonstration scenario | `scenario` ("1" - "6", or "all") |
| `POST` | `/api/reset` | Resets state to default seed data | None |

### 4.2 Endpoint Details and Examples

#### `GET /api/state`
Returns the complete JSON snapshot of all registered facilities, active and historical reservations, current simulation timestamp, emergency status, and audit log.

Example response:
```json
{
  "currentTime": 1788253200,
  "formattedTime": "2026-09-01 09:00:00",
  "emergencyMode": false,
  "emergencyReason": "",
  "facilities": [
    {
      "facilityId": "FAC-101",
      "name": "Main Campus Auditorium",
      "building": "Campus Center",
      "capacity": 300,
      "status": "Available",
      "statusCode": 0,
      "doorsLocked": false,
      "hourlyRate": 150.00
    }
  ],
  "reservations": [],
  "auditLog": [
    {
      "timestamp": "2026-09-01 09:00:00",
      "message": "BookingSystem initialized with default campus facilities."
    }
  ]
}
```

#### `POST /api/book`
Submits a new reservation request. If the facility is available, places a 15-minute soft lock.

Example request:
```bash
curl -X POST http://localhost:8080/api/book \
  -H "Content-Type: application/json" \
  -d '{
    "facilityId": "FAC-104",
    "requesterId": "USR-001",
    "title": "Robotics Club Workshop",
    "eventType": "Student Workshop",
    "attendance": 25
  }'
```

Example response:
```json
{
  "success": true,
  "reservationId": "RES-0001",
  "message": "Reservation created and submitted (15-min soft lock active)"
}
```

#### `POST /api/advance-time`
Advances the simulated clock by the specified number of seconds. This triggers evaluation of soft-lock expirations and revision deadlines.

Example request:
```bash
curl -X POST http://localhost:8080/api/advance-time -d "seconds=3600"
```

Example response:
```json
{
  "success": true,
  "currentTime": 1788256800,
  "formattedTime": "2026-09-01 10:00:00"
}
```

#### `POST /api/approve`
Approver evaluates and approves a pending booking in `UnderReview`.

Example request:
```bash
curl -X POST http://localhost:8080/api/approve \
  -d "id=RES-0001&approverId=USR-005&comments=Approved+for+curricular+use"
```

Example response:
```json
{
  "success": true,
  "message": "Reservation RES-0001 approved and venue locked"
}
```

#### `POST /api/cancel`
Cancels an existing reservation. If the reservation is already approved, the system enforces the 24-hour lead time requirement.

Example request:
```bash
curl -X POST http://localhost:8080/api/cancel \
  -d "id=RES-0001&userId=USR-001"
```

Response when cancelled with sufficient notice:
```json
{
  "success": true,
  "message": "Reservation cancelled successfully"
}
```

Response when blocked by the 24-hour lead time guard:
```json
{
  "success": false,
  "message": "Cancellation rejected: approved bookings must be cancelled at least 24 hours prior to start time"
}
```

---

## 5. Verification Test Suite and Scenario Engine

### 5.1 Test Harness (`test/TestSuite.hpp`)
The test harness runs without third-party frameworks. It provides:
* Custom assertion macros: `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_NE`, `ASSERT_THROWS`.
* Global assertion counting via `std::atomic<uint64_t>`.
* Microsecond execution timing per test case.
* ANSI colorized pass/fail reports and detailed exception location reporting (`__FILE__`, `__LINE__`).

The 14 suites cover:
1. `testUserHierarchyAndPolymorphism`: Base class dispatch, role names, and quota limits.
2. `testFacilityOperations`: Availability checking, overlapping time intervals, and door lock states.
3. `testReservationComposition`: Aggregate root assembly with child equipment and support requests.
4. `testNotificationService`: Message dispatching, queue sizes, and audit trail generation.
5. `testReservationHappyPath`: Full lifecycle progression from Draft to Completed.
6. `testSoftLockExpiration`: 15-minute timeout causing release of calendar holds.
7. `testSecurityReviewEscalation`: Automatic trigger when headcount > 100, blocking standard approval until officer sign-off.
8. `testRevisionAndResubmissionLoop`: Change requests, deadline checks, and resubmissions.
9. `testCancellationRulesAndLeadTime`: Verification of lead time guards for approved reservations.
10. `testInvalidReservationTransitions`: Attempting illegal state jumps (e.g. Draft directly to Completed).
11. `testFacilityOperationalCycle`: Staging windows, check-in unlock, and turnover cycles.
12. `testFacilityMaintenanceAndEmergency`: Damage reporting, repairs certification, and emergency holds.
13. `testBookingSystemIntegration`: JSON serialization, multi-venue scheduling, and reset logic.
14. `testWebDashboardAndEndpoints`: Embedded HTTP request routing, static dashboard serving, API state serialization, time advancement, Content-Length bounds validation, and 404 responses.

### 5.2 Scenario Runner (`service/ScenarioRunner.hpp`)
The scenario runner executes 6 automated walkthroughs demonstrating core operational workflows:
* **Scenario 1:** Standard Happy Path Booking Lifecycle.
* **Scenario 2:** Large Event Security Review and Clearance.
* **Scenario 3:** Approver Revision Request and Resubmission Loop.
* **Scenario 4:** Facility Logistics, Staging, and Turnover.
* **Scenario 5:** Campus Emergency Lockdown and Restoration.
* **Scenario 6:** Cancellation Policy and 24-Hour Lead Time Guard.

Run all scenarios from the terminal:
```bash
./build/app --scenario all
```

---

## 6. Compilation and Debugging Workflows

### 6.1 Using the VS Code Dev Container
The Dev Container provides an Ubuntu 24.04 environment pre-configured with GCC 13+, GDB, CMake 3.28+, and Ninja.

1. Open the repository root in VS Code.
2. Run `Dev Containers: Reopen in Container` from the Command Palette (`F1`).
3. Build and test:
```bash
cmake -B build -G Ninja
cmake --build build
./build/app --test
```

### 6.2 Host Compilation on Windows (MSVC)
From an environment with Visual Studio 2022 C++ build tools:

```powershell
# Configure CMake for Visual Studio 2022
cmake -B build -S .

# Build Debug configuration
cmake --build build --config Debug

# Run the automated tests
.\build\Debug\app.exe --test

# Run the scenario demonstrations
.\build\Debug\app.exe --scenario all

# Start the web dashboard
.\build\Debug\app.exe --web 8080
```

### 6.3 Host Compilation on Linux or macOS (GCC / Clang)
Ensure CMake 3.20+ and a C++20 compatible compiler are installed:

```bash
# Configure with Ninja or Makefiles
cmake -B build -S . -G Ninja

# Compile the target
cmake --build build

# Execute verification suite
./build/app --test
```

### 6.4 Debugging with GDB / VS Code
When running inside the Dev Container:
* Press `F5` in VS Code to launch GDB with the active executable.
* To debug from the command line:
```bash
gdb ./build/app
(gdb) break smartcampus::service::BookingSystem::createReservation
(gdb) run --test
(gdb) continue
```

To clean compilation artifacts:
```bash
# On Linux / Dev Container
rm -rf build

# On Windows PowerShell
Remove-Item -Recurse -Force build
```
