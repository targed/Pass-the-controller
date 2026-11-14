# Smart Campus Event & Facility Booking System

A C++20 campus event coordination and venue booking platform built to demonstrate formal object-oriented class modeling and dual synchronized state engines. The system coordinates room scheduling, multi-tier administrative approvals, campus security reviews, automated calendar soft-locks, smart door access control, custodial turnover workflows, and emergency preemption.

The application compiles into a single executable binary (`app`) that includes an embedded HTTP web dashboard, an interactive terminal console, automated scenario demonstrations, and a 13-suite unit test harness.

---

## Architecture and Class Model

The system architecture implements the synthesized domain model formulated in `Docs/Capstone Class Model and State Models.pdf` and `Docs/AI ML Class and State Models.pdf`.

```
                    ┌─────────────────────────┐
                    │      User (Base)        │
                    ├─────────────────────────┤
                    │ userId, fullName, email │
                    │ department, authTokens  │
                    └────────────▲────────────┘
                                 │
     ┌───────────────────────────┼───────────────────────────┬─────────────────────────┐
     │                           │                           │                         │
┌────┴────────────────────┐ ┌────┴────────────────────┐ ┌────┴───────────────────┐ ┌───┴─────────────────┐
│ StudentOrgRepresentative│ │ FacultyStaffMember      │ │ FacilityCoordinator    │ │ EventApprover       │
├─────────────────────────┤ ├─────────────────────────┤ ├────────────────────────┤ ├─────────────────────┤
│ organizationName        │ │ employeeId, tenureTrack │ │ assignedBuildings      │ │ approvalTier (1-3)  │
│ bookingLimit: 5/month   │ │ expeditedReview: true   │ │ manageFacility()       │ │ evaluatePolicy()    │
└─────────────────────────┘ └─────────────────────────┘ └────────────────────────┘ └─────────────────────┘
```

### 1. User Hierarchy and Roles
All campus participants inherit from the abstract base class `User`:
* **`User` (Abstract Base Class):** Encapsulates core identity properties (`userId`, `fullName`, `email`, `department`), authentication tokens, and virtual methods for permission interrogation.
* **`StudentOrgRepresentative`:** Represents student club leaders. Enforces monthly active booking quotas (default: 5 active reservations) and links reservations to recognized student organizations.
* **`FacultyStaffMember`:** Models academic faculty and university staff. Grants expedited administrative review privileges for academic courses and departmental colloquia.
* **`FacilityCoordinator`:** Represents building managers responsible for room configurations, staging equipment, inspecting post-event facilities, and toggling maintenance flags.
* **`EventApprover`:** Administrative role evaluating requests against university venue policies, noise ordinances, and scheduling rules across three approval tiers.

### 2. Facility Entity
The `Facility` class models physical campus spaces such as lecture halls, auditoriums, seminar rooms, and outdoor plazas:
* **Attributes:** `facilityId`, `name`, `building`, `capacity`, `hourlyRate`, `status` (`RoomStatus`), and supported equipment inventory.
* **Operations:** `checkAvailability(start, end)`, `setBlackout(start, end)`, `setSmartDoorLocked(bool)`, and `updateStatus(RoomStatus)`.
* **Smart Door Integration:** Tracks real-time electronic door lock states. Doors unlock automatically during active event windows and lock during staging, turnover, maintenance, or campus lockdowns.

### 3. Reservation Aggregate Root and Child Entities
`Reservation` serves as the central aggregate root coordinating booking lifecycle rules:
* **Core Attributes:** `reservationId`, `requesterId`, `facilityId`, `eventTitle`, `eventType`, `expectedAttendance`, `startTime`, `endTime`, `status` (`ReservationStatus`), `holdExpiresAt`, and `requiresSecurity`.
* **Compositional Child Entities:**
  * **`EquipmentRequest`:** Tracks staging requirements such as projectors, PA audio systems, podiums, and folding tables with fulfillment tracking flags.
  * **`SupportRequest`:** Tracks auxiliary staffing requirements including custodial turnover, AV technician support, and campus police details with assigned officer identifiers.
  * **`Approval`:** Formal audit record of approver evaluations, recording approver ID, timestamp, decision enum, and justification notes.

### 4. Notification Service
The `NotificationService` singleton acts as an internal message dispatcher:
* Maintains a centralized audit log of all system transitions and dispatch messages.
* Routes alerts to email, SMS gateways, facility maintenance boards, and campus police dispatch.
* Provides full audit inspection capabilities through the web dashboard and CLI.

---

## State Machine Architectures

The system coordinates two distinct, synchronized state machines.

```
Reservation State Machine: Governs the administrative lifecycle of a booking request
Facility State Machine:    Governs the physical status and access control of the venue
```

### 1. Reservation Lifecycle State Machine (11 States)

```
[*] ──> Draft ──[Submit / Lock]──> Submitted ──[Route]──> UnderReview
                                                            │
                     ┌──────────────────────────────────────┴──────────────────────────────────────┐
                     │ [headcount > 100 or after-hours]                                            │ [standard criteria]
                     ▼                                                                             ▼
               SecurityReview ──[Security Cleared]─> Approved <────────────────────────────────────┘
                     │                                   │
             [Security Denied]                           ├──[Start Time Reached]──> InProgress ──> Completed ──> [*]
                     │                                   │
                     ▼                                   └──[Cancel >= 24h before]──┐
                  Rejected ──> [*]                                                  │
                     ▲                                                              ▼
                     │ [Policy Violation]                                       Cancelled ──> [*]
                     │                                                              ▲
                UnderReview ──[Request Changes]──> NeedsRevision                    │
                                                         │                          │
                                                         ├──[Resubmit in Window]──> Submitted
                                                         │                          │
                                                         ├──[Cancel]────────────────┘
                                                         │
                                                         └──[Deadline Passed]─────> Expired ──> [*]
```

1. **`Draft`:** The requester configures room, headcount, schedule, and equipment without placing holds on the venue calendar.
2. **`Submitted`:** A calendar conflict check runs. If clear, a 15-minute temporary soft-lock (`holdExpiresAt`) is placed on the room.
3. **`UnderReview`:** The booking enters the administrative queue for venue policy evaluation.
4. **`SecurityReview`:** High-risk gate automatically triggered when expected attendance exceeds 100 people or the event runs outside standard campus operating hours. Requires a dedicated campus security officer evaluation.
5. **`NeedsRevision`:** The approver requests specific schedule or logistical adjustments. The requester is given a 48-hour revision deadline.
6. **`Expired`:** The requester failed to resubmit within the revision deadline, or a soft-lock expired without submission. The calendar hold is released.
7. **`Approved`:** The request has passed all policy and security checks. The room calendar is permanently booked.
8. **`InProgress`:** The simulation clock reaches event start time. Smart doors unlock for attendee check-in.
9. **`Completed`:** Event concludes. Smart doors lock and the venue transitions to custodial turnover.
10. **`Rejected`:** The request is rejected due to safety risks, schedule clashes, or policy violations. Terminal state.
11. **`Cancelled`:** The requester cancels the booking. Approved bookings require at least 24 hours advance notice.

### 2. Facility Operational State Machine (7 States)

```
       ┌────────────────────────────────────────────────────────────────────────┐
       │                                                                        │
       ▼                                                                        │
    Available ──[Reservation Confirmed]──> Reserved ──[Setup Window]──> Setup   │
       ▲                                                                  │     │
       │                                                           [Event Starts]
       │                                                                  ▼     │
       │                                                               Occupied │
       │                                                                  │     │
       │                                                             [Event Ends]
       │                                                                  ▼     │
       └──────────────[Cleaned and Inspected]────────────────────────── Turnover│
                                                                          │     │
                                                          [Damage Found]  │     │
                                                                ▼         │     │
    Available <─────────[Repairs Certified]──────────────── OutOfService <┘     │
       ▲                                                                        │
       └────────────────[All-Clear Issued]────────────── EmergencyHold <─[Any]──┘
```

1. **`Available`:** Room is clean, unlocked for walk-in inspections, and available in the reservation directory.
2. **`Reserved`:** Confirmed reservation holds the room slot; awaits equipment staging.
3. **`Setup`:** Facility support staff is actively staging AV equipment, tables, or seating.
4. **`Occupied`:** Event is actively taking place. Smart doors are unlocked for attendee access.
5. **`Turnover`:** Event concluded. Doors lock while custodial crew cleans, sanitizes, and resets furniture.
6. **`OutOfService`:** Maintenance blackout triggered by reported damage or scheduled facility repairs.
7. **`EmergencyHold`:** Campus security or severe weather lockdown. Doors lock immediately and all active reservations are paused.

---

## Launching the Application

The executable provides four operational modes controlled via command-line flags.

### 1. Embedded Web Dashboard (`--web`)

Starts the cross-platform embedded HTTP web server (default port 8080) hosting a single-page management dashboard.

```bash
# Start on default port 8080
./build/app --web

# Or specify a custom port
./build/app --web 9000
```

Once running, open your web browser to:
`http://localhost:8080`

The Web Dashboard features:
* **Facilities Grid:** Live status cards for every campus venue showing capacity, amenities, door lock status, and operational state badges.
* **Reservation Kanban Board:** Drag-and-drop style view tracking bookings across all 11 lifecycle stages.
* **Booking Creation Modal:** Interactive form for submitting new reservations with attendance, facility selection, and equipment choices.
* **Approver Action Center:** One-click controls to Approve, Reject, or Request Changes on pending submissions.
* **Security Operations Center:** Assign campus police officers, clear high-capacity events, or issue campus-wide emergency lockdowns.
* **Simulation Clock Controller:** Buttons to advance simulation time by 15 minutes, 1 hour, or 24 hours to observe soft-lock timeouts and automatic state transitions.
* **Real-Time Audit Log:** Live feed of access control logs, door sensor changes, and notification dispatches.

### 2. Automated Test Suite (`--test`)

Executes the built-in 13-suite automated test harness with 595 verification assertions.

```bash
./build/app --test
```

Exit code is `0` when all assertions pass and `1` on any failure. The suite validates:
* Polymorphic behavior across all four User specializations.
* Facility availability algorithms, blackout handling, and smart door locks.
* Reservation aggregate root composition with equipment, support requests, and approval records.
* Notification dispatch queues and audit trails.
* Complete coverage of all 11 reservation states and valid transitions.
* Soft-lock 15-minute expiration timers.
* Large event security escalation triggers (>100 headcount).
* Revision and resubmission deadline enforcement.
* The 24-hour lead time cancellation rule.
* Rejection of invalid transitions and illegal state bypasses.
* The 7-state cyclic facility turnover and maintenance loop.
* Campus emergency lockdown preemption and all-clear recovery.
* High-level BookingSystem integration and JSON state serialization.

### 3. Pre-Built Scenario Demonstrations (`--scenario`)

Runs end-to-end demonstrations of key campus workflows with formatted console outputs detailing state changes at each step.

```bash
# Run all 6 scenarios in sequence with summary table
./build/app --scenario all

# Or run individual scenarios (1 through 6)
./build/app --scenario 1
./build/app --scenario 2
./build/app --scenario 3
./build/app --scenario 4
./build/app --scenario 5
./build/app --scenario 6
```

Summary of scenarios:
* **Scenario 1 (Standard Happy Path):** Student Org drafts a workshop, submits with a 15-minute soft-lock, receives approver sign-off, checks in at start time, completes the event, and triggers custodial turnover back to Available.
* **Scenario 2 (Large Event Security Review):** A 220-attendee competition triggers the security gate, blocks standard approver sign-off, receives dedicated police clearance, and completes approval.
* **Scenario 3 (Revision Request Loop):** An approver requests layout diagrams with a 48-hour deadline. The requester resubmits updated specs within the window, and the booking is approved.
* **Scenario 4 (Facility Staging Logistics):** Demonstrates the operational cycle from Available to Reserved, opening the staging window (Setup), event check-in (Occupied), conclusion (Turnover), and inspection.
* **Scenario 5 (Campus Emergency Lockdown):** A campus security alert immediately places all facilities into EmergencyHold, locks all smart doors, rejects new booking attempts, and safely restores normal operations upon all-clear.
* **Scenario 6 (Cancellation Lead Time Guard):** Proves that cancellations with more than 24 hours notice succeed and free the room, while cancellations attempted inside the 24-hour window are strictly blocked.

### 4. Interactive Console (`--cli`)

Launches an interactive menu-driven terminal interface:

```bash
./build/app --cli
```

Allows operators to browse facilities, list active reservations, submit test bookings, trigger emergency holds, step the simulation clock forward, and execute scenario runs interactively.

---

## Development Environment (VS Code Dev Container)

The repository includes a ready-to-use Dev Container configuration for Visual Studio Code based on Ubuntu 24.04 with GCC, GDB, CMake, and Ninja.

### Prerequisites
1. [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows or macOS) or Docker Engine (Linux).
2. [Visual Studio Code](https://code.visualstudio.com/).
3. The **Dev Containers** extension (`ms-vscode-remote.remote-containers`) in VS Code.

### Opening the Project
1. Open this repository folder in VS Code.
2. When prompted to "Reopen in Container", click it. Alternatively, press `F1`, type `Dev Containers: Reopen in Container`, and press Enter.
3. VS Code will build the container image and mount the workspace.

### Building in the Terminal
```bash
# Configure the build directory using Ninja
cmake -B build -G Ninja

# Compile the app target
cmake --build build

# Run the test suite
./build/app --test

# Launch the web dashboard
./build/app --web
```

For advanced architecture guides, REST API specifications, and transition tables, see the [C++ Development Guide](Docs/cpp-development.md).
