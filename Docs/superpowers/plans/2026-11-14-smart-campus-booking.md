# Smart Campus Event & Facility Booking System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement the complete Smart Campus Event & Facility Booking System in modern C++20 with domain models, dual state machine engines, automated tests, interactive CLI scenarios, and an embedded web dashboard.

**Architecture:** Domain entities (`User`, `Facility`, `Reservation`) model campus booking semantics. Explicit state engines (`ReservationStateMachine`, `FacilityStateMachine`) enforce all 11 reservation states and 7 facility operational states with guard evaluation. An embedded cross-platform HTTP server serves a live browser dashboard alongside a terminal scenario runner and automated test suite.

**Tech Stack:** C++20, CMake, Ninja, POSIX/Winsock sockets, HTML5/CSS/JavaScript.

## Global Constraints
- C++ Standard: C++20
- Build tool: CMake (Ninja / MSVC compatible)
- Binary executable target: `app`
- Dev container image: `mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04`
- Source directory: `src/`
- Spec document: `Docs/superpowers/specs/2026-11-14-smart-campus-booking-design.md`

---

### Task 1: Domain Entities (`src/domain/`)

**Files:**
- Create: `src/domain/User.hpp`
- Create: `src/domain/Facility.hpp`
- Create: `src/domain/NotificationService.hpp`
- Create: `src/domain/Reservation.hpp`
- Test: Compile check via test harness

**Interfaces:**
- Produces: `User` hierarchy (`StudentOrgRepresentative`, `FacultyStaffMember`, `FacilityCoordinator`, `EventApprover`), `Facility` entity, `Reservation` aggregate root with child `EquipmentRequest`, `SupportRequest`, `Approval` classes, and `NotificationService`.

- [ ] **Step 1: Create `src/domain/User.hpp`**
Define abstract base `User` and specialized roles with virtual methods.

- [ ] **Step 2: Create `src/domain/Facility.hpp`**
Define `RoomStatus` enum (7 states), `Facility` entity with capacity, building, and availability helpers.

- [ ] **Step 3: Create `src/domain/NotificationService.hpp`**
Define `Notification` struct and thread-safe notification dispatch/logging service.

- [ ] **Step 4: Create `src/domain/Reservation.hpp`**
Define `ReservationStatus` enum (11 states), child requests (`EquipmentRequest`, `SupportRequest`, `Approval`), and `Reservation` aggregate root.

- [ ] **Step 5: Verify syntax and commit**
```bash
git add src/domain/
git commit -m "feat(domain): add core user, facility, and reservation models"
```

---

### Task 2: State Machine Engines (`src/state/`)

**Files:**
- Create: `src/state/ReservationStateMachine.hpp`
- Create: `src/state/FacilityStateMachine.hpp`
- Test: State transition checks

**Interfaces:**
- Consumes: `src/domain/` entities and status enums
- Produces: `ReservationStateMachine` and `FacilityStateMachine` with transition functions, guard evaluation, and action dispatches.

- [ ] **Step 1: Create `src/state/ReservationStateMachine.hpp`**
Implement the 11-state reservation lifecycle:
- `submit()` with 15-minute soft lock.
- `routeToReview()` routing to approver.
- `evaluateSecurity()` checking `headcount > 100 || isAfterHours`.
- `approve()` and `securityClear()`.
- `requestChanges()`, `resubmit()`, and `expire()`.
- `reject()`.
- `checkIn()` and `conclude()`.
- `cancel()` enforcing 24-hour lead time if approved.

- [ ] **Step 2: Create `src/state/FacilityStateMachine.hpp`**
Implement the 7-state operational facility cycle:
- `Available` -> `Reserved` -> `Setup` -> `Occupied` -> `Turnover` -> `Available`.
- Back-to-back fast track (`Turnover` -> `Setup`).
- Maintenance blackout (`OutOfService`) and repair certification.
- Campus security emergency lockdown (`EmergencyHold`) and all-clear restoration.

- [ ] **Step 3: Verify syntax and commit**
```bash
git add src/state/
git commit -m "feat(state): implement reservation and facility state machine engines"
```

---

### Task 3: Central Booking System Coordinator & Scenarios (`src/service/`)

**Files:**
- Create: `src/service/BookingSystem.hpp`
- Create: `src/service/ScenarioRunner.hpp`
- Test: Scenario execution

**Interfaces:**
- Consumes: Domain models and state engines
- Produces: In-memory repository, coordinator actions, simulated clock, and 6 pre-built executable scenarios.

- [ ] **Step 1: Create `src/service/BookingSystem.hpp`**
Coordinate facilities, reservations, users, clock ticks, and events in an in-memory repository with thread safety.

- [ ] **Step 2: Create `src/service/ScenarioRunner.hpp`**
Implement executable automated walkthroughs:
1. Standard Happy Path
2. Large Event Security Path
3. Revision & Resubmission Loop
4. Facility Staging & Custodial Turnover
5. Campus Emergency Lockdown & All-Clear
6. 24-Hour Cancellation Lead Time Enforcement

- [ ] **Step 3: Verify syntax and commit**
```bash
git add src/service/
git commit -m "feat(service): add booking coordinator and scenario runner"
```

---

### Task 4: Automated Verification Test Suite (`src/test/`)

**Files:**
- Create: `src/test/TestSuite.hpp`
- Test: Execute tests via test runner

**Interfaces:**
- Consumes: All domain, state, and service classes
- Produces: Self-contained test suite verifying all 11 reservation states, 7 facility states, guard conditions, and negative tests.

- [ ] **Step 1: Create `src/test/TestSuite.hpp`**
Include unit tests for:
- Role permissions and polymorphism.
- Full Reservation lifecycle happy path and exception paths.
- Security escalation guard (`headcount > 100`).
- Revision deadline timeout (`Expired`).
- Cancellation window guard (`timeToEvent > 24h`).
- Full Facility operational cycle and emergency preemption.
- Invalid state transition rejection.

- [ ] **Step 2: Verify syntax and commit**
```bash
git add src/test/
git commit -m "test: add comprehensive automated test suite for state machines and models"
```

---

### Task 5: Embedded Web Dashboard & Server (`src/web/`)

**Files:**
- Create: `src/web/HttpServer.hpp`
- Create: `src/web/WebDashboard.hpp`
- Test: Socket communication and HTTP response verification

**Interfaces:**
- Consumes: `BookingSystem` state
- Produces: Cross-platform HTTP server serving single-page HTML5/CSS/JS dashboard and JSON REST API.

- [ ] **Step 1: Create `src/web/WebDashboard.hpp`**
Raw string literal containing the single-page application with modern responsive dark-mode styling, live room cards, reservation Kanban pipeline, booking form, approver queue, and clock controls.

- [ ] **Step 2: Create `src/web/HttpServer.hpp`**
Cross-platform socket HTTP server (Windows Winsock and Linux POSIX) handling REST endpoints (`/api/state`, `/api/book`, `/api/approve`, `/api/reject`, `/api/advance-time`, `/api/emergency`, `/api/all-clear`, `/api/scenario`).

- [ ] **Step 3: Verify syntax and commit**
```bash
git add src/web/
git commit -m "feat(web): add embedded HTTP server and visual interactive dashboard"
```

---

### Task 6: Main Entry Point and Build Integration

**Files:**
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt`
- Test: Full CMake build and executable options

**Interfaces:**
- Produces: Executable `app` supporting `--web` (dashboard), `--cli` (interactive scenarios), `--test` (automated tests), and `--help`.

- [ ] **Step 1: Implement `src/main.cpp`**
Parse command-line flags and dispatch to the Web dashboard, CLI scenario runner, or Test suite.

- [ ] **Step 2: Update `CMakeLists.txt`**
Link `ws2_32` on Windows and pthread on Linux. Add all header include paths.

- [ ] **Step 3: Verify clean build and execution**
Run: `cmake -B build -S . && cmake --build build && .\build\Debug\app.exe --test`
Expected: 100% tests pass.

- [ ] **Step 4: Commit Task 6 changes**
```bash
git add src/main.cpp CMakeLists.txt
git commit -m "feat: integrate main entry point with web, cli, and test modes"
```

---

### Task 7: Final Documentation, Verification, and Clean Tree

**Files:**
- Modify: `README.md`
- Modify: `Docs/cpp-development.md`
- Test: Full build, verification, git status

- [ ] **Step 1: Update `README.md`**
Document the architecture, state models, interactive web dashboard, CLI scenarios, and test suite.

- [ ] **Step 2: Update `Docs/cpp-development.md`**
Provide detailed instructions for launching the web dashboard, running scenarios, and compiling in the dev container.

- [ ] **Step 3: Fresh verification test**
Execute clean build, verify `--test` and `--cli` execution.

- [ ] **Step 4: Final commit**
```bash
git add README.md Docs/cpp-development.md
git commit -m "docs: update system documentation for smart campus booking system"
```
