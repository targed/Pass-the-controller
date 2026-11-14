# Smart Campus Event & Facility Booking System Design Specification

## Overview
This document specifies the architecture, domain class models, state engines, and presentation layers for the Smart Campus Event & Facility Booking System. The design implements the synthesized class model and dual state models formulated in the team's capstone modeling research (`Docs/Capstone Class Model and State Models.pdf` and `Docs/AI ML Class and State Models.pdf`), delivering an autonomous, full-featured C++20 implementation.

---

## 1. Domain Class Architecture

### 1.1 User Hierarchy and Roles
All actors in the system derive from an abstract base class `User`, ensuring consistent identity and authentication fields while supporting role-specific behaviors.

```
                  ┌──────────────────────┐
                  │     User (Abstract)  │
                  ├──────────────────────┤
                  │ userId: string       │
                  │ fullName: string     │
                  │ email: string        │
                  │ department: string   │
                  └──────────▲───────────┘
                             │
     ┌───────────────────────┼───────────────────────┬────────────────────────┐
     │                       │                       │                        │
┌────┴─────────────────┐ ┌───┴───────────────────┐ ┌─┴──────────────────────┐ ┌──┴──────────────────┐
│ StudentOrgRepresentative│ │ FacultyStaffMember   │ │ FacilityCoordinator   │ │ EventApprover        │
├──────────────────────┤ ├───────────────────────┤ ├────────────────────────┤ ├──────────────────────┤
│ organizationName: str│ │ employeeId: str       │ │ assignedBuildings: List│ │ approvalTier: int    │
│ submitBooking()      │ │ submitExpedited()     │ │ manageFacility()       │ │ reviewRequest()      │
│ cancelBooking()      │ │                       │ │ setAvailability()      │ │ approve() / reject() │
└──────────────────────┘ └───────────────────────┘ └────────────────────────┘ └──────────────────────┘
```

* **`User` (Abstract Base Class):** Defines `userId`, `fullName`, `email`, `department`, `login()`, `logout()`, and role interrogation helpers.
* **`StudentOrgRepresentative`:** Models student club officers. Enforces organization-specific booking limits.
* **`FacultyStaffMember`:** Models academic and administrative staff with expedited review privileges for curricular events.
* **`FacilityCoordinator`:** Responsible for building schedules, facility maintenance flags, equipment staging, and custodial turnover.
* **`EventApprover`:** Responsible for evaluating reservation requests against university policies, approving, denying with written justification, or requesting revisions.

### 1.2 Facilities and Aggregate Root
* **`Facility`:** Represents a physical campus space (auditorium, classroom, conference hall, athletic facility).
  * Attributes: `facilityId` (string), `name` (string), `building` (string), `capacity` (int), `status` (`RoomStatus`), `hourlyRate` (double), `supportedEquipment` (vector of string).
  * Methods: `checkAvailability(start, end)`, `setBlackout(start, end)`, `updateStatus(newStatus)`.
* **`Reservation` (Aggregate Root):** Central entity coordinating the booking lifecycle.
  * Attributes: `reservationId` (string), `requesterId` (string), `facilityId` (string), `eventTitle` (string), `eventType` (string), `expectedAttendance` (int), `startTime` (time_t), `endTime` (time_t), `status` (`ReservationStatus`), `holdExpiresAt` (time_t), `requiresSecurity` (bool).
  * Compositional Children:
    * `EquipmentRequest` (1 to 0..*): `requestId`, `equipmentType`, `quantity`, `isFulfilled`.
    * `SupportRequest` (1 to 0..*): `requestId`, `supportType`, `notes`, `isAssigned`, `assignedOfficerId`.
    * `Approval` (1 to 0..*): `approvalId`, `approverId`, `decision`, `comments`, `decidedAt`.
* **`NotificationService`:** Singleton service maintaining a message queue of dispatched alerts, confirmation receipts, and custodial notifications.

---

## 2. State Machine Specifications

### 2.1 Model 1: Reservation Lifecycle State Model
Governs the end-to-end lifecycle of an event reservation request through 11 formal states:

```
[*] ──> Draft ──[noConflicts / lockHold]──> Submitted ──> UnderReview
                                                            │
                     ┌──────────────────────────────────────┴──────────────────────────────────────┐
                     │ [headcount > 100 || afterHours]                                             │ [headcount <= 100 && !afterHours]
                     ▼                                                                             ▼
               SecurityReview ──[securityCleared]──> Approved <────────────────────────────────────┘
                     │                                   │
             [securityDenied]                            ├──[startReached]──> InProgress ──> Completed ──> [*]
                     │                                   │
                     ▼                                   └──[cancel && timeToEvent > 24h]──┐
                  Rejected ──> [*]                                                         │
                     ▲                                                                     ▼
                     │ [policyViolation]                                               Cancelled ──> [*]
                     │                                                                     ▲
                UnderReview ──[requestChanges]──> NeedsRevision                            │
                                                        │                                  │
                                                        ├──[resubmit withinDeadline]──> Submitted
                                                        │                                  │
                                                        ├──[cancel]────────────────────────┘
                                                        │
                                                        └──[deadlinePassed]───────────> Expired ──> [*]
```

#### State Definitions
1. **`Draft`:** Requester selects room, date, headcount, and equipment. Not visible to approvers.
2. **`Submitted`:** Conflict check passed; temporary 15-minute soft lock placed on room calendar.
3. **`UnderReview`:** Request routed to Event Approver queue for initial policy evaluation.
4. **`SecurityReview`:** High-risk gate triggered when `headcount > 100` or event occurs after normal campus hours. Requires dedicated security officer assignment.
5. **`NeedsRevision`:** Approver requests adjustments. Requester has a defined deadline to fix issues and resubmit.
6. **`Expired`:** Revision deadline passed without requester resubmission. Hold released.
7. **`Approved`:** Formally approved and confirmed. Room calendar slot locked permanently.
8. **`InProgress`:** Event start time reached. Smart doors unlocked; attendees on site.
9. **`Completed`:** Event finished. Smart doors locked; custodial turnover triggered.
10. **`Rejected`:** Request denied due to policy violation or unresolvable conflict. Terminal.
11. **`Cancelled`:** Booking cancelled by requester. Terminal. Enforces 24-hour lead time if approved.

### 2.2 Model 2: Facility (Physical Room) Operational State Model
Governs physical space availability, safety locks, and turnover across 7 continuous cyclic states:

```
   ┌────────────────────────────────────────────────────────────────────────┐
   │                                                                        │
   ▼                                                                        │
Available ──[reservationConfirmed]──> Reserved ──[setupWindow]──> Setup     │
   ▲                                                                │       │
   │                                                         [eventStarts]  │
   │                                                                ▼       │
   │                                                             Occupied   │
   │                                                                │       │
   │                                                           [eventEnds]  │
   │                                                                ▼       │
   └──────────────[cleanedAndInspected]────────────────────────── Turnover  │
                                                                    │       │
                                                    [damageFound]   │       │
                                                          ▼         │       │
Available <─────────[repairsCertified]────────────── OutOfService <─┘       │
   ▲                                                                        │
   └────────────[allClearGiven]────────────── EmergencyHold <──[any state]──┘
```

#### State Definitions
1. **`Available`:** Room is clean, unlocked for walk-in inspections, and bookable in the directory.
2. **`Reserved`:** Confirmed booking holds the calendar slot; room awaits staging window.
3. **`Setup`:** Facility support staff is actively configuring AV, tables, or specialized equipment.
4. **`Occupied`:** Event actively occurring. Smart doors unlocked; attendees present.
5. **`Turnover`:** Event concluded. Room locked; custodial staff inspects, cleans, and resets furniture.
6. **`OutOfService`:** Maintenance blackout. Scheduled repairs or reported damage block bookings.
7. **`EmergencyHold`:** Campus security emergency lockdown. Doors securely locked; active reservations suspended until all-clear inspection.

---

## 3. Application Components & Presentation

The project compiles into a single executable `app` supporting three distinct modes:

### 3.1 Embedded Web Dashboard (`--web`, default port 8080)
* Single-header HTTP server (`httplib.h`) with zero external dynamic dependencies.
* Serves a modern single-page dashboard containing:
  * **Facilities Grid:** Cards showing each campus venue, capacity, features, and color-coded operational state badge.
  * **Reservation Kanban:** Visual cards organized across the 11 lifecycle stages.
  * **Interactive Booking Modal:** Submit new reservations with equipment and attendance options.
  * **Approver Action Center:** One-click Approve, Reject, or Request Changes buttons.
  * **Security Center:** Assign security personnel and trigger simulated campus lockdowns.
  * **Simulation Clock Controller:** Advance time by 15 minutes, 1 hour, or 1 day to trigger timers and state transitions.
  * **Event Log:** Live feed of dispatches, smart door triggers, and audit logs.

### 3.2 Interactive Scenario Simulator (`--cli`)
A terminal interface providing automated walkthroughs of the 6 core business flows:
1. Standard Happy Path (Student Org booking -> Review -> Approval -> InProgress -> Completed).
2. Large Event Security Path (Headcount > 100 -> Security Review -> Officer Clearance -> Approval).
3. Revision Loop (Approver changes requested -> NeedsRevision -> Resubmit -> Approval).
4. Facility Logistics (Setup staging -> Check-in door unlock -> Turnover cleaning -> Available).
5. Campus Emergency Preemption (Active event -> Emergency lockdown -> All-clear restoration).
6. Cancellation Constraints (Verification of 24-hour lead time requirement).

### 3.3 Automated Verification Test Suite (`--test`)
Self-contained unit and state engine tests verifying:
* Class inheritance, attribute setting, and virtual method dispatch.
* Complete coverage of all 11 reservation states and all valid transitions.
* Complete coverage of all 7 facility states and cyclic transitions.
* Guard verification: 15-minute hold expiry, attendance threshold checks, 24-hour cancellation rule.
* Negative test cases: invalid transitions rejected, unauthorized actions prevented.

---

## 4. File Layout

```
src/
├── domain/
│   ├── User.hpp                # Abstract User and role specializations
│   ├── Facility.hpp            # Facility entity, RoomStatus, availability
│   ├── Reservation.hpp         # Reservation aggregate root & child requests
│   └── NotificationService.hpp # Alert and dispatch service
├── state/
│   ├── ReservationStateMachine.hpp # 11-state reservation lifecycle engine
│   └── FacilityStateMachine.hpp    # 7-state operational facility engine
├── service/
│   ├── BookingSystem.hpp       # Central system coordinator and repository
│   └── ScenarioRunner.hpp      # Pre-built demonstration scenarios
├── web/
│   ├── HttpServer.hpp          # Embedded httplib server
│   ├── WebDashboard.hpp        # HTML/CSS/JavaScript single-page asset
│   └── httplib.h               # Single-header HTTP library
├── test/
│   └── TestSuite.hpp           # Automated test harness
└── main.cpp                    # CLI flag parser and entry point
```
