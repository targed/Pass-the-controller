# Research Findings: Capstone Class and State Models

**Primary Sources:**
- `Docs/Capstone Class Model and State Models.pdf`
- `Docs/AI ML Class and State Models.pdf`
- `Docs/Capstone I & II_Guide Lines.pdf`

---

## 1. Domain Overview
The project is the **Smart Campus Event & Facility Booking System** (Room and Event Reservation Platform for university facilities). It simulates the end-to-end lifecycle of reserving campus spaces, routing for administrative and safety approvals, managing physical room status (setup, turnover, maintenance, emergency locks), and handling equipment and support service requests.

---

## 2. Synthesized Class Model Architecture
(Source: `Docs/AI ML Class and State Models.pdf`, Page 36)

### User Hierarchy (Role-Based Access)
- **`User` (Abstract Base Class):**
  - Attributes: `userId: string`, `name: string`, `email: string`, `department: string`, `passwordHash: string`, `role: string`
  - Operations: `login(): bool`, `logout(): void`
- **`StudentOrgRepresentative` (inherits `User`):**
  - Attributes: `organizationName: string`
  - Operations: `submitReservation(): void`, `cancelReservation(reservationId: string): bool`
- **`FacultyStaffMember` (inherits `User`):**
  - Attributes: `employeeId: string`
  - Operations: `submitReservation(): void`, `requestExpeditedBooking(): void`
- **`FacilityCoordinator` (inherits `User`):**
  - Attributes: `assignedBuildings: List<string>`
  - Operations: `manageFacility(): void`, `setAvailability(): void`
- **`EventApprover` (inherits `User`):**
  - Attributes: `approvalTier: int`
  - Operations: `reviewRequest(id: string): void`, `approve(id: string): void`, `reject(id: string, reason: string): void`, `requestChanges(id: string, notes: string): void`

### Core Domain Entities
- **`Facility` (Physical Campus Space):**
  - Attributes: `facilityId: string`, `name: string`, `building: string`, `capacity: int`, `status: RoomStatus`
  - Operations: `checkAvailability(start: DateTime, end: DateTime): bool`, `setBlackout(start: DateTime, end: DateTime): void`
- **`Reservation` (Central Hub Entity):**
  - Attributes: `reservationId: string`, `eventTitle: string`, `eventType: string`, `expectedAttendance: int`, `startTime: DateTime`, `endTime: DateTime`, `status: ReservationStatus`, `requiresSecurity: bool`
  - Operations: `checkConflicts(): bool`, `evaluateSecurityThreshold(): bool`, `transitionStatus(newStatus: ReservationStatus): void`
  - Compositional Children:
    - `SupportRequest` (1 : 0..*): `requestId: string`, `supportType: string`, `notes: string`, `assign(): void`
    - `EquipmentRequest` (1 : 0..*): `requestId: string`, `equipmentType: string`, `quantity: int`, `fulfill(): void`, `cancel(): void`
    - `Approval` (1 : 0..*): `approvalId: string`, `decision: string`, `comments: string`, `decidedAt: DateTime`, `record(): void`
- **`NotificationService` (External Integration / Dependency):**
  - Operations: `sendConfirmation(email: string): void`, `sendReminder(email: string): void`

---

## 3. State Model 1: Reservation Lifecycle
(Source: `Docs/Capstone Class Model and State Models.pdf`, Page 3 & `Docs/AI ML Class and State Models.pdf`, Page 37)

### States
`Draft`, `Submitted`, `UnderReview`, `SecurityReview`, `Approved` (or `Confirmed`), `NeedsRevision`, `Expired`, `Rejected`, `InProgress`, `Completed`, `Cancelled`.

### Lifecycle Flow and Guards
1. **Creation:** `[*] -> Draft` via `createDraft / selectRoomAndDate`.
2. **Submission:** `Draft -> Submitted` via `submitRequest [noTimeConflicts] / lockHold(15min)`.
3. **Routing:** `Submitted -> UnderReview` via `routeToApprover()`.
4. **Conditional Security Gate:**
   - If `headcount > 100 || afterHours`: transitions `UnderReview -> SecurityReview`.
   - Cleared via `securityCleared [staffAssigned] -> Approved`.
   - Denied via `securityDenied -> Rejected`.
5. **Standard Approval:**
   - If `headcount <= 100 && !afterHours`: transitions `UnderReview -> Approved`.
6. **Revision Loop:**
   - Approver requests adjustments: `UnderReview -> NeedsRevision` via `requestChanges [commentsProvided]`.
   - Requester resubmits: `NeedsRevision -> Submitted` via `resubmit [withinDeadline]`.
   - Timeout: `NeedsRevision -> Expired` via `deadlinePassed / releaseHold()`.
7. **Rejection:** `UnderReview -> Rejected` via `reject [policyViolation]`.
8. **Event Execution:**
   - `Approved -> InProgress` when start time reached (`checkIn()`).
   - `InProgress -> Completed` when event concludes (`logUsageData()`).
9. **Cancellation:** Allowed from `Submitted`, `NeedsRevision`, and `Approved` (with guard `[timeToEvent > 24h]`), releasing holds.

---

## 4. State Model 2: Facility (Physical Room) Operational State
(Source: `Docs/Capstone Class Model and State Models.pdf`, Page 5 & `Docs/AI ML Class and State Models.pdf`, Page 37)

### States (Continuous Operational Cycle)
`Available`, `Reserved`, `Setup` (Under Reconfiguration), `Occupied`, `Turnover` (Pending Turnover / Cleaning), `OutOfService` (Maintenance), `EmergencyHold` (Blackout).

### Lifecycle Flow and Guards
1. `[*] -> Available` via `registerRoom()`.
2. `Available -> Reserved` via `reservationConfirmed / lockCalendarSlot()`.
3. `Reserved -> Setup` via `setupWindowOpens / stageEquipment()`.
4. `Setup -> Occupied` via `eventStarts [setupComplete] / unlockDoors()`.
5. `Occupied -> Turnover` via `eventEnds / lockDoors()`.
6. `Turnover -> Available` via `cleanedAndInspected [readyForUse] / clearRoom()`.
7. **Fast turnover for back-to-back bookings:** `Turnover -> Setup` via `backToBackBooking / startNextSetup()`.
8. **Maintenance branch:**
   - From `Turnover` if damage found, or from `Available`/`Reserved`/`Setup` if fault reported -> `OutOfService`.
   - `OutOfService -> Available` via `repairVerified / reinstateRoom()`.
9. **Emergency lock:**
   - Any state -> `EmergencyHold` via `emergencyDeclared / lockDoors()`.
   - `EmergencyHold -> Available` via `allClearGiven / inspectRoom()`.
