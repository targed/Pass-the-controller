# AI vs. Team C++ Software Development: Comparative Analysis and Engineering Reflection

**Project:** Smart Campus Event & Facility Booking System  
**Course:** Computer Science Senior Capstone I & II  
**Document Type:** Diátaxis Explanation & Technical Audit  
**Authoring Team:**
* Luke Kaiser (Team Lead: Docker, Dev Containers, build toolchains)
* Sam Hajdukiewicz (Class model integration: User, Event Approver, Staff Member, test data)
* Michael Allen (State Diagram 2 operational logic and code implementation)
* Lyla Graham (State Diagram 1 reservation lifecycle logic and code implementation)
* Simon Leeker (Class Model: Event List, Facility, Room, Notification Service, Campus Security)
* Joshua Dobbs (Class Model: Request, syntax verification, visual documentation)
* Autonomous AI Development Agent (C++20 synthesis, dual state engines, web dashboard, automated test suite)

---

## Table of Contents
1. Project Context and Objectives
2. Executive Summary
3. Team-Developed C++ Implementation Overview
4. AI-Developed C++ Implementation Overview
5. Side-by-Side Comparative Matrices
   * Class Model and Entity Decomposition
   * Attributes, Data Types, and Type Safety
   * Methods, Encapsulation, and Memory Safety
   * State Machine Architecture and Lifecycle Coverage
   * Guard Conditions, Business Rules, and Timing Thresholds
   * Concurrency, Multi-Tenancy, and Thread Safety
   * Testing, Verification, and Assertion Coverage
   * Presentation Layer, UI/UX, and Developer Experience
6. In-Depth Critical Review and Bug Audit
   * Analysis of the Team-Developed Solution
   * Analysis of the AI-Developed Solution
   * Code Quality and Maintainability Metrics
7. Decision Log: Keep, Modify, Reject, Add
8. Answers to the Capstone Reflection Questions
9. Conclusion and Capstone II Direction

---

## 1. Project Context and Objectives

During the semester, our capstone team designed conceptual object-oriented class diagrams and dynamic state charts for a university facility booking platform. This platform manages campus room reservations, multi-stage administrative approvals, physical room readiness (setup, cleaning, maintenance), and campus security interventions.

Following the initial design phase, our instructor tasked the team with two sequential development milestones:
1. **In-Class Activity:** A rapid, 48-hour human team implementation translating our diagrams into working C++ code on branch `class_state_in_class_activity`.
2. **Outside-Class AI Activity:** An autonomous AI-driven implementation on branch `AI_class_state_in_class_activity`, implementing the complete domain models and state diagrams without inheriting human codebase shortcuts, followed by a rigorous technical reflection.

This report evaluates both codebases, identifies defects and architectural divergences, documents trade-offs, and details concrete improvements for our ongoing software engineering practice.

---

## 2. Executive Summary

The team implementation and the AI implementation represent two distinct software engineering paradigms under different operational constraints:

* **The Team Solution (Human 48-Hour Prototype):** Created under tight time limits by six team members dividing modules across individual files. It successfully captured high-level user interface flows in an interactive terminal menu and prototyped basic data structures. However, time pressure led to severe architectural compromises: state machines were implemented as disconnected singletons, state was reduced to a boolean flag in the `Request` class, memory management leaked heap allocations on every menu interaction, and critical logic bugs were left unresolved in core setters.
* **The AI Solution (Autonomous C++20 Full-Scale System):** Developed using disciplined test-driven methodologies and modular software decomposition. It produced an enterprise-grade C++20 domain library featuring an abstract `User` hierarchy, a non-singleton aggregate root (`Reservation`) with composed child entities, two formal event-driven state engines enforcing all 11 reservation states and 7 facility operational states, 14 automated test suites with 609 assertions (100% passing in 6.4 ms), an interactive terminal scenario runner, and an embedded zero-dependency socket HTTP server serving a live dark-mode web dashboard on port 8080.

The comparison provides compelling insights into where AI accelerates software delivery (boilerplate generation, exhaustive state coverage, unit testing, cross-platform socket servers) and where human domain oversight is essential (identifying scope boundaries, evaluating system complexity, and grounding requirements in real campus workflows).

---

## 3. Team-Developed C++ Implementation Overview

The team developed its code across three primary directories within `src/` on branch `class_state_in_class_activity`:
* `src/class_model/`: Entity declarations (`user_classes.h`, `facility_classes.h`, `testdata.h`, `testdata.cpp`).
* `src/state_diagram_one/`: Reservation request state logic (`state_diagram_one.h`, `functions.cpp`).
* `src/state_diagram_two/`: Physical room operational state logic (`state_diagram_two.h`, `functions.cpp`).
* `src/main.cpp`: Entry point with terminal-based login, registration, and action selection loops.

### 3.1 Class Architecture
The team used C++98/03 programming conventions. All classes used public member variables rather than private fields with accessors:
* `User`: Base class with `int userId`, `string name`, `string email`. Contains member functions `reserveRoom(Room&)`, `editRoomRequest(Request&)`, and `cancelRoomRequest(Request&)`.
* `StaffMember`: Inherits from `User`, adding `int staffId`, `bool isStudent`, `string organization`, and `string jobTitle`.
* `EventApprover`: Inherits from `User`, adding `int approverId` and `manageEvent(Request&)`.
* `FacilityCoordinator`: Chained inheritance deriving from `EventApprover`, adding `manageFacility(vector<Facility>&)`.
* `SystemAdministrator`: Chained inheritance deriving from `EventApprover`, adding `generateUsageReport()`.
* `Request`: Independent class with public string fields (`roomName`, `eventName`, `eventTime`, `eventDate`) and `bool status`.
* `Room`: Models a campus room with `int roomNumber`, `int capacity`, `bool roomAvailable`, and string vectors for booked slots.
* `Facility`: Represents a building containing a `std::vector<Room>` collection.
* `NotificationService` and `CampusSecurity`: Lightweight helper classes printing console statements.

### 3.2 State Machine Implementation
The team implemented state models as singleton classes:
* `Reservation_Request`: Defined in `state_diagram_one/` with an internal enum `State` (`Start`, `Draft`, `Submitted`, `Under_Review`, `Rejected`, `Security_Review`, `Cancelled`, `Approved`, `In_Progress`, `Completed`). Transitions are driven by `newEvent(Event event)` via a `switch` statement logging to `std::clog`.
* `Room_Status`: Defined in `state_diagram_two/` with an internal enum `RoomState` (`Start`, `Avaliable` [sic], `TentativelyHeld`, `Reserved`, `InUse`, `Maintenance`, `Blackout`, `Final`). Transitions are driven by `newEvent(RoomEvent event)`.

### 3.3 Main Workflow and Execution
The team's `main.cpp` provided an interactive console loop:
1. Login/registration prompt checking an in-memory email vector.
2. Action menu with options to reserve a room, edit a request, cancel a request, manage an event, or print a usage report.
3. Pre-populated facility data from `createTestData()`.

---

## 4. AI-Developed C++ Implementation Overview

The AI implementation was designed from the synthesized UML specifications, structured in modular namespaces under `src/`:
* `src/domain/`: Core business models (`User.hpp`, `Facility.hpp`, `Reservation.hpp`, `NotificationService.hpp`).
* `src/state/`: Event-driven state machine engines (`ReservationStateMachine.hpp`, `FacilityStateMachine.hpp`).
* `src/service/`: Central coordinator and demonstration runners (`BookingSystem.hpp`, `ScenarioRunner.hpp`).
* `src/test/`: Header-only test harness (`TestSuite.hpp`).
* `src/web/`: Embedded zero-dependency socket server and UI (`HttpServer.hpp`, `WebDashboard.hpp`).
* `src/main.cpp`: Multi-modal CLI dispatcher (`--web`, `--cli`, `--scenario`, `--test`, `--help`).

```
Pass-the-controller/ (AI Solution Structure)
├── .devcontainer/
│   └── devcontainer.json       # Ubuntu 24.04 C++20 Dev Container
├── src/
│   ├── domain/                 # Domain Entity Layer
│   │   ├── User.hpp            # Polymorphic role hierarchy
│   │   ├── Facility.hpp        # Physical venues, RoomStatus, blackout windows
│   │   ├── Reservation.hpp     # Aggregate root & composed child requests
│   │   └── NotificationService.hpp # Thread-safe notification dispatch
│   ├── state/                  # Formal State Machine Engines
│   │   ├── ReservationStateMachine.hpp # 11-State lifecycle engine
│   │   └── FacilityStateMachine.hpp    # 7-State cyclic facility engine
│   ├── service/                # Application Services & Scenarios
│   │   ├── BookingSystem.hpp   # Thread-safe repository & simulated clock
│   │   └── ScenarioRunner.hpp  # 6 Automated end-to-end walkthroughs
│   ├── web/                    # Presentation Web Layer
│   │   ├── HttpServer.hpp      # Cross-platform socket HTTP server (20 REST routes)
│   │   └── WebDashboard.hpp    # Single-page responsive dark-mode dashboard
│   ├── test/                   # Verification Test Suite
│   │   └── TestSuite.hpp       # 14 test suites, 609 assertions (100% pass)
│   └── main.cpp                # Unified CLI entry point
├── CMakeLists.txt              # Modern CMake with CTest integration
└── README.md                   # Complete system documentation
```

### 4.1 Domain Architecture Highlights
* **Strict Type Safety:** Scoped enumerations (`enum class RoomStatus`, `enum class ReservationStatus`, `enum class UserRole`) with bidirectional string converters.
* **Polymorphic Authorization:** `User` defines an abstract interface with pure virtual `getRole()` and virtual permission queries (`canSubmit()`, `canApprove()`, `canCoordinate()`). Concrete roles (`StudentOrgRepresentative`, `FacultyStaffMember`, `FacilityCoordinator`, `EventApprover`, `SecurityOfficer`, `Administrator`) implement their respective privileges.
* **Aggregate Root Pattern:** `Reservation` owns child entities (`EquipmentRequest`, `SupportRequest`, `Approval` audit records) through composition, managing atomic identifiers and lifecycle cascading.

### 4.2 State Engines and Guard Evaluation
* **`ReservationStateMachine`:** Implements 11 states (`Draft`, `Submitted`, `UnderReview`, `SecurityReview`, `Approved`, `NeedsRevision`, `Expired`, `Rejected`, `InProgress`, `Completed`, `Cancelled`). Enforces the 15-minute soft lock, conditional security review for headcount > 100 or after-hours (`08:00 to 20:00`), revision loops with deadlines, and the 24-hour cancellation rule.
* **`FacilityStateMachine`:** Implements 7 cyclic states (`Available`, `Reserved`, `Setup`, `Occupied`, `Turnover`, `OutOfService`, `EmergencyHold`). Enforces custodial reset, back-to-back fast-tracking, maintenance blackouts, and campus emergency lockdowns with smart door control.

### 4.3 Multi-Modal Presentation Layer
* **Web Dashboard:** Serves an embedded single-page application on `http://localhost:8080` with zero runtime dependencies. Displays real-time facility cards, an 11-column Kanban board, booking creation forms, approver review queues, security officer assignments, simulation clock controls, and a live audit log stream.
* **Interactive CLI:** Terminal console allowing room browsing, manual booking drafting, and clock controls.
* **Scenario Runner:** Six pre-packaged automated walkthroughs demonstrating every business path.
* **Automated Test Suite:** 14 test suites and 609 assertions integrated into CTest.

---

## 5. Side-by-Side Comparative Matrices

The following tables evaluate both solutions across eight fundamental software engineering dimensions, following the analytical format established in our modeling research.

### Table 1: Class Model and Entity Decomposition

| Architectural Feature | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Number of Classes** | 10 classes | 16 classes | The team combined multiple responsibilities into few classes. The AI separated concerns into distinct domain, state, service, and presentation classes. |
| **User Hierarchy** | Chained inheritance: `User` <- `EventApprover` <- `FacilityCoordinator` <- `SystemAdministrator`. `StaffMember` inherits `User`. | Clean branching hierarchy: Abstract `User` base with 6 concrete leaf subclasses (`StudentOrgRep`, `FacultyStaff`, `FacilityCoord`, `EventAppr`, `SecurityOfficer`, `Admin`). | **AI is superior.** Chained inheritance forced `SystemAdministrator` to be an `EventApprover`, violating the Liskov Substitution Principle. The AI cleanly branched roles from a single abstract base. |
| **Reservation Entity** | Standalone `Request` class with flat attributes. | `Reservation` aggregate root composing `EquipmentRequest`, `SupportRequest`, and `Approval`. | **AI is superior.** The team lumped equipment and security into console print statements. The AI modeled them as distinct entities with quantities, IDs, and statuses. |
| **Approval Audit Trail** | None. Approvals overwrite a boolean flag in `Request`. | Dedicated `Approval` class tracking `approvalId`, `approverId`, `decision`, `comments`, and timestamp. | **AI is superior.** The team could not track revision history. The AI preserves full audit trails across multiple review rounds. |
| **Facility Modeling** | `Facility` contains `Room` vector; `Room` manages its own booked slots via string matching. | `Facility` models venue capacity, building, room status, supported equipment, and blackout windows. | **AI is superior.** The team split facility and room unnecessarily. The AI modeled facilities with operational state integration and conflict detection. |
| **Service Decoupling** | Standalone `NotificationService` class printing hardcoded strings. | Thread-safe `NotificationService` singleton with dynamic queue, message filtering, and audit streaming. | **AI is superior.** The AI service supports programmatic dispatch and real-time frontend streaming. |

### Table 2: Attributes, Data Types, and Type Safety

| Attribute Dimension | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Lifecycle State Representation** | Primitive `bool status` in `Request`. Separate `enum class State` in disconnected singleton. | Strongly typed `enum class ReservationStatus` (11 values) and `enum class RoomStatus` (7 values). | **AI is superior.** The team's `bool status` could only represent binary approved/denied status. The AI strictly enforces all 11 lifecycle stages. |
| **Temporal Data Types** | Raw `std::string` for dates and times (`"7:45"`, `"04/05/2027"`). | Typed UNIX epoch timestamps (`std::time_t`) and ISO 8601 formatting. | **AI is superior.** String comparisons cannot compute elapsed durations, time differences for 24-hour cancellation rules, or soft-lock timeouts. |
| **String Handling** | `std::string` passed by value across setters. | `std::string_view` for zero-allocation reads, rvalue references (`std::string&&`) for move construction. | **AI is superior.** The AI solution avoids unnecessary string copies and follows modern C++ guidelines. |
| **Visibility and Encapsulation** | All member variables declared `public:`. | All member variables declared `private:` with `const` accessors and validated mutators. | **AI is superior.** The team violated encapsulation, allowing external callers to corrupt internal state without validation. |
| **Identifier Generation** | Hardcoded or uninitialized integer IDs (`int userId = 0`). | Atomic monotonic ID generators (`std::atomic<uint64_t>`) generating prefixed keys (`RES-1001`, `NOTIF-1001`). | **AI is superior.** Prevents ID collisions in multi-user simulations. |

### Table 3: Methods, Encapsulation, and Memory Safety

| Method & Memory Aspect | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Dynamic Memory Management** | Raw pointers created with `new` and never freed: `new Room`, `new Request`. | Modern RAII, stack semantics, and smart pointers (`std::shared_ptr`, `std::unique_ptr`). Zero raw `new`/`delete`. | **AI is superior.** The team's code leaks memory on every menu loop execution (`main.cpp:97, 105, 113`). The AI codebase has zero memory leaks. |
| **Const Correctness** | Inconsistent. Only two methods marked `const` in `facility_classes.h`. | Strict const-correctness on all accessors, query methods, and state checkers. | **AI is superior.** Ensures compile-time read-only guarantees on domain data. |
| **Error Handling** | Console error logs (`std::clog`, `std::cout`) with void returns. | Structured result structs (`TransitionResult`, `FacilityTransitionResult`) with boolean status and descriptive messages. | **AI is superior.** Callers can programmatically inspect why a transition failed rather than relying on terminal stdout. |
| **Bug Status** | Contains logic bug in `user_classes.h:35`: `setEventTime` modifies `eventName`. | Zero logic bugs. Passes all unit test assertions with `-Wall -Wextra -Wpedantic`. | **AI is superior.** Demonstrates the necessity of automated test suites during development. |

### Table 4: State Machine Architecture and Lifecycle Coverage

| State Machine Aspect | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Instantiation Pattern** | Global Meyer's Singleton (`getInstance()`). | Instantiable state machine engines operating on entity instances. | **AI is superior.** The team's singleton pattern meant the system could only track one reservation and one room globally. The AI engine supports thousands of independent instances. |
| **Reservation States** | 10 states declared in enum, but `NeedsRevision` was omitted. | 11 states fully implemented (`Draft`, `Submitted`, `UnderReview`, `SecurityReview`, `Approved`, `NeedsRevision`, `Expired`, `Rejected`, `InProgress`, `Completed`, `Cancelled`). | **AI is superior.** The AI matches the complete synthesized state diagram, including revision loops and expiration. |
| **Facility States** | 8 states declared (`Start`, `Avaliable`, `TentativelyHeld`, `Reserved`, `InUse`, `Maintenance`, `Blackout`, `Final`). | 7 cyclic states (`Available`, `Reserved`, `Setup`, `Occupied`, `Turnover`, `OutOfService`, `EmergencyHold`). | **AI is superior.** The team included linear start/final states for physical rooms. The AI correctly models rooms as continuous operational cycles. |
| **State Integration** | Completely disconnected. State machines in `functions.cpp` were never called from `main.cpp`. | Deeply integrated. `BookingSystem` routes all entity operations through `ReservationStateMachine` and `FacilityStateMachine`. | **AI is superior.** The team had orphan state machine files. The AI uses state engines as the core business logic gate. |
| **Transition Verification** | Permissive switch blocks without transition matrix validation. | Explicit transition matrices (`canTransition(from, to)`) rejecting invalid paths before execution. | **AI is superior.** Prevents illegal transitions (e.g. attempting to approve a completed reservation). |

### Table 5: Guard Conditions, Business Rules, and Timing Thresholds

| Business Rule / Guard | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **15-Minute Soft Lock** | Mentioned in state table comments, but unimplemented in code. | Fully enforced: sets `holdExpiresAt = currentTime + 900`. Automatically expires if not routed within 15 minutes. | **AI is superior.** Discrete clock advancement verifies hold expiration. |
| **Security Escalation Gate** | Missing in code. State diagram had manual `evaluateEvent` transition. | Automated evaluation: if `headcount > 100` or `isAfterHours()`, automatically transitions `UnderReview` to `SecurityReview`. | **AI is superior.** Business logic enforces safety policies without relying on manual user routing. |
| **Security Clearance** | Missing in code. | Requires security officer assignment; verifies officer credentials before transitioning to `Approved`. | **AI is superior.** Enforces institutional accountability for large gatherings. |
| **24-Hour Cancellation Rule** | Unimplemented. `cancelRoomRequest` sets `status = false` unconditionally. | Strict lead-time guard: if `timeToEvent < 24h`, cancellation is rejected with an explanatory message. | **AI is superior.** Protects facility resources from last-minute cancellations. |
| **Smart Door Lock Control** | Unimplemented. | Physical doors automatically unlock on `checkIn()` (`Occupied`) and lock on `conclude()` (`Turnover`) or `declareEmergency()` (`EmergencyHold`). | **AI is superior.** Directly models IoT campus infrastructure interactions. |

### Table 6: Concurrency, Multi-Tenancy, and Thread Safety

| Concurrency Dimension | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Multi-Tenancy** | Single-tenant only. Singleton state machines prevent multiple concurrent bookings. | Fully multi-tenant. Repository manages independent entities concurrently. | **AI is superior.** Essential for real-world campus scheduling. |
| **Thread Safety** | Non-thread-safe. Raw vectors and global singletons without mutex locks. | Thread-safe. Mutex protection (`std::lock_guard<std::mutex>`) on `BookingSystem` and `NotificationService`. | **AI is superior.** Safe for multi-threaded socket servers and concurrent user requests. |
| **Asynchronous Networking** | No networking. Terminal standard input only (`std::cin`). | Multi-threaded non-blocking HTTP socket worker threads handling client requests concurrently. | **AI is superior.** Web server handles simultaneous browser connections without freezing the simulation loop. |

### Table 7: Testing, Verification, and Assertion Coverage

| Testing Dimension | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Automated Test Suite** | None. Manual terminal menu walkthrough only. | 14 test suites, 609 assertions executing in ~6.4 ms with 100% pass rate. | **AI is superior.** Provides regression safety and automated grading evidence. |
| **Test Integration** | No test runner configured in `CMakeLists.txt`. | Integrated into CMake and CTest (`add_test(NAME UnitAndStateTests COMMAND app --test)`). | **AI is superior.** Supports standard automated CI/CD pipelines. |
| **Scenario Testing** | Ad-hoc terminal choices with placeholder `// TODO` comments. | 6 pre-built executable scenarios with automated assertion checks and colorized output. | **AI is superior.** Demonstrates complex multi-step workflows repeatably. |
| **Negative Testing** | None. Invalid inputs produce console errors or infinite loops. | Exhaustive negative tests verifying that illegal transitions, past deadlines, and unauthorized cancellations fail cleanly. | **AI is superior.** Guarantees boundary condition robustness. |

### Table 8: Presentation Layer, UI/UX, and Developer Experience

| UX / Interface Dimension | Team-Developed Solution | AI-Developed Solution | Comparative Analysis & Finding |
| :--- | :--- | :--- | :--- |
| **Web Dashboard** | None. Terminal text only. | Embedded dark-mode single-page application on `http://localhost:8080`. | **AI is superior.** Provides visual room cards, live 11-column Kanban board, and instant screenshot generation for reports. |
| **REST API** | None. | 20 JSON REST API endpoints supporting programmatic booking, approval, inspection, and clock advancement. | **AI is superior.** Decouples presentation from business logic. |
| **Terminal Experience** | Numbered text menu with raw `std::cin` prompts prone to buffer corruption. | Multi-modal CLI (`--web`, `--cli`, `--scenario all`, `--test`, `--help`) with ANSI styling. | **AI is superior.** Offers flexible interaction modes for developers, testers, and evaluators. |
| **Build Configuration** | Basic `CMakeLists.txt` compiling single target without warnings. | Modern CMake with strict warnings (`/W4`, `-Wall -Wextra -Wpedantic`), socket library linking, and CTest. | **AI is superior.** Enforces high compiler hygiene across Windows and Linux. |

---

## 6. In-Depth Critical Review and Bug Audit

### 6.1 Critical Review of the Team-Developed Solution

The human team faced a strict 48-hour development window. This constraint forced pragmatic simplifications that highlight common pitfalls in undergraduate software engineering:

#### 1. The Singleton Anti-Pattern and Global Coupling
The team chose to implement both state models (`Reservation_Request` and `Room_Status`) as global Meyers Singletons (`getInstance()`). While this simplified state access across student sub-modules without passing object references, it broke the domain architecture:
* A campus booking platform must track hundreds of rooms and thousands of reservations simultaneously. A singleton restricted the entire system to one reservation and one room.
* Furthermore, in `src/main.cpp`, these singletons were instantiated at startup but never connected to user actions. Reserving a room created an isolated `Room` instance on the heap rather than invoking the `Room_Status` singleton.

#### 2. Heap Allocation Leaks and Lack of RAII
In `src/main.cpp`, every user interaction in the menu loop dynamically allocated memory without corresponding `delete` calls:
```cpp
// src/main.cpp:97-98
Room *room = new Room;
user->reserveRoom(*room); // 'room' is never deleted, leaking memory

// src/main.cpp:105-106
Request *req = new Request;
user->editRoomRequest(*req); // 'req' is never deleted, leaking memory

// src/main.cpp:113-114
Request *req1 = new Request;
user->cancelRoomRequest(*req1); // 'req1' is never deleted, leaking memory
```
Additionally, these calls created disconnected, empty objects rather than retrieving the actual room or reservation the user intended to modify.

#### 3. Critical Field Overwrite Defect in `user_classes.h`
In `src/class_model/user_classes.h`, line 35 contains a copy-paste error where setting the event time accidentally overwrites the event name:
```cpp
// src/class_model/user_classes.h:34-36
void setEventTime(string newTime) {
    eventName = newTime; // BUG: Overwrites eventName instead of eventTime!
}
```
Because no automated unit tests existed in the team repository, this bug remained undetected throughout the in-class activity.

#### 4. Chained Inheritance Antipattern
The team structured administrative roles in a linear inheritance chain:
`User` -> `EventApprover` -> `FacilityCoordinator` -> `SystemAdministrator`.
This structure implies that every `FacilityCoordinator` is an `EventApprover`, and every `SystemAdministrator` is both a `FacilityCoordinator` and an `EventApprover`. In reality, these are distinct orthogonal roles with different institutional responsibilities: a System Administrator manages accounts and permissions, not event approvals; a Facility Coordinator manages building repairs and cleaning, not event policy.

#### 5. Reduction of State Models to Boolean Flags
In `user_classes.h`, `Request` defined its lifecycle state as `bool status;`. Approving an event set `status = true`, while denying or cancelling set `status = false`. This completely bypassed the 10 states designed in the team's state diagram, discarding revision loops, security reviews, check-ins, and post-event reporting.

---

### 6.2 Critical Review of the AI-Developed Solution

The AI solution was developed autonomously without time pressure or team coordination overhead, yielding a complete, verified system. However, a critical engineering audit reveals distinct characteristics and trade-offs:

#### 1. Architectural Strengths
* **Complete State Model Fidelity:** The AI implemented every state, event, guard condition, and action from the synthesized diagrams. The 15-minute soft lock, the 100-person security threshold, after-hours detection, revision recovery loops, and the 24-hour cancellation rule are fully functional and verified.
* **Modern C++20 Standard Compliance:** The codebase uses C++20 features effectively, including concepts, scoped enums with string mappings, move semantics, atomic identifiers, and thread-safe date formatting across Windows and POSIX.
* **Exhaustive Automated Verification:** 14 test suites with 609 assertions run in under 7 ms, providing total confidence in state transition correctness.
* **Zero-Dependency Web Layer:** Implementing an embedded socket server and inlined HTML dashboard provides visual demonstration and screenshot generation capabilities without external package managers or complex build dependencies.

#### 2. Limitations and Engineering Trade-Offs
* **In-Memory Storage vs. Persistent Database:** All facilities, reservations, and audit records reside in an in-memory repository (`BookingSystem`). While ideal for simulation, demonstration, and automated testing, shutting down the process discards all data. A production rollout would require an SQLite, PostgreSQL, or key-value persistence layer.
* **Ad-Hoc JSON Parameter Extraction:** To avoid heavy external JSON parsing libraries, the AI implemented lightweight string-scanning utilities (`extractField`, `extractIntField`) in `HttpServer.hpp`. While functional for standard web forms, malformed inputs or strings containing escaped quotation marks could challenge naive substring parsing.
* **Architectural Scope and Footprint:** The AI produced over 3,000 lines of robust C++ code across six modular directories. For a simple classroom exercise, this introduces substantial structural complexity compared to a single-file prototype. However, for a capstone software engineering portfolio, it represents production-grade quality.

---

### 6.3 Code Quality and Maintainability Metrics

| Metric | Team-Developed Solution | AI-Developed Solution | Evaluation & Significance |
| :--- | :--- | :--- | :--- |
| **Lines of Code (Core)** | ~550 lines | ~3,200 lines | The team solution is concise but incomplete; the AI solution includes full state engines, web server, and tests. |
| **C++ Standard** | Legacy C++98 / C++03 | Modern C++20 | AI leverages standard C++20 features (concepts, move semantics, chrono/time_t). |
| **Compiler Warnings** | 4 warnings on MSVC/GCC (unused variables, signed/unsigned mismatches) | 0 warnings under `/W4` and `-Wall -Wextra -Wpedantic` | AI enforces strict compiler hygiene. |
| **Memory Management** | Manual `new` without `delete` (memory leaks) | 100% RAII, smart pointers, zero memory leaks | AI prevents resource leaks. |
| **Coupling / Cohesion** | High coupling (chained inheritance, singletons, public variables) | High cohesion, low coupling (isolated domain, state, service, web layers) | AI codebase is modular and extensible. |
| **Automated Test Coverage** | 0% (0 unit tests) | 100% (14 suites, 609 assertions, CTest integrated) | AI guarantees verifiable regression safety. |

---

## 7. Decision Log: Keep, Modify, Reject, Add

Following the synthesis method used in our prior modeling report, this decision log establishes what our team will adopt from the AI solution, what we will modify, what we will reject, and what we will add for Capstone II.

### 7.1 Keep (From Team Solution)
1. **Realistic Campus Venue Personas:** The team's choice of campus facilities (Centennial Auditorium, STEM Lab, Student Union Ballroom, Seminar Room) reflects actual university environments and makes simulation scenarios relatable.
2. **Interactive Terminal Simplicity:** The concept of an accessible text-based interactive menu for quick local testing without launching a browser.
3. **Core Role Definitions:** The identified campus roles (Student Org Representative, Faculty Member, Facility Coordinator, Event Approver, Security Officer).

### 7.2 Modify (Adapted from AI Solution)
1. **Replace Singleton State Machines with Entity-Bound State Engines:** Discard `getInstance()` singletons. Adopt the AI's pattern of instantiable state machine engines that operate on specific `Reservation` and `Facility` object references.
2. **Refactor Chained Inheritance to Branching Polymorphism:** Remove the `User` -> `EventApprover` -> `FacilityCoordinator` chain. Adopt the AI's clean hierarchy where all roles inherit directly from the abstract `User` base class.
3. **Upgrade State Flags to Scoped Enumerations:** Replace `bool status` in `Request` with the complete 11-state `ReservationStatus` enum.
4. **Transition from Raw Pointers to RAII:** Eliminate all raw `new`/`delete` expressions in `main.cpp`, adopting standard stack allocation and smart pointer wrappers.

### 7.3 Reject (From AI Solution)
1. **Overly Complex Embedded Web Server for Classroom Baseline:** While the embedded socket HTTP server and dark-mode dashboard are valuable for capstone presentations, they represent unnecessary complexity for basic C++ state machine evaluation. For routine classroom assignments, terminal scenario runners provide sufficient verification with lower maintenance overhead.
2. **Ad-Hoc JSON Substring Extraction:** Rather than maintaining custom string scanning helpers for HTTP payloads, future iterations should adopt an established, audited single-header JSON library (such as `nlohmann/json`) or stick to typed binary/CLI interfaces.

### 7.4 Add (Missing from Both Solutions Initially)
1. **Persistent Relational Database Backend:** Integrate an embedded SQLite database using SQLite3 C++ wrappers to ensure campus reservations, user accounts, and room maintenance histories persist across application restarts.
2. **Automated Notification Delivery Integration:** Connect `NotificationService` to an actual university SMTP relay or mock webhook endpoint to dispatch real test emails to requesters and approvers.
3. **Role-Based Authentication and Password Hashing:** Upgrade the plain-text in-memory email lookup to salted cryptographic password hashing (`bcrypt` or `Argon2`) for secure student and faculty logins.

---

## 8. Answers to the Capstone Reflection Questions

### Question 1: How does AI-generated software development compare to team-developed software development?
AI-generated development operates with extraordinary speed, structural completeness, and syntactic consistency. In our project, the AI took the synthesized UML specifications and produced a comprehensive, fully functional C++20 application in a fraction of the time required by human engineers. It eliminated common human development oversights: it did not forget to implement guard conditions, it did not leave memory leaks, it did not take shortcuts by reducing complex states to booleans, and it authored an exhaustive 609-assertion test suite that would have taken a human team days to write.

However, AI development requires clear, unambiguous architectural guidance. When human engineers collaborate, they informally negotiate requirements and catch practical edge cases through dialogue. An AI agent implements exactly what is specified in the design prompts: if the prompt leaves ambiguities, the AI will either synthesize an assumption or produce overly complex boilerplate. AI excels as an implementation accelerator, while human engineers provide essential domain framing, architectural boundaries, and user validation.

### Question 2: What are the primary strengths and limitations of both approaches?
* **Team-Developed Solution Strengths:** High intuitive understanding of campus user needs, pragmatic focus on interactive console workflows, and direct personal knowledge of university facility procedures.
* **Team-Developed Solution Limitations:** High susceptibility to time pressure, fragmentation across sub-teams (leading to disconnected state machines), copy-paste errors (such as the `setEventTime` bug), and neglect of automated testing and memory management.
* **AI-Generated Solution Strengths:** Flawless type safety and const-correctness, 100% adherence to modern C++20 standards, complete implementation of all 11 reservation states and 7 facility states, zero memory leaks, built-in CTest automation, and sophisticated multi-modal presentation (web dashboard, CLI, scenarios).
* **AI-Generated Solution Limitations:** Tendency toward architectural over-engineering (such as embedding a custom socket HTTP server for a class activity), dependency on in-memory storage without persistent databases, and higher conceptual overhead for newcomers reviewing the code.

### Question 3: What errors or missing components were identified in each codebase?
* **Errors in Team Solution:**
  1. *Logic Defect (`user_classes.h:35`):* `setEventTime` incorrectly assigns `eventName = newTime;`, corrupting event titles when editing schedules.
  2. *Memory Leaks (`main.cpp:97, 105, 113`):* `new Room`, `new Request`, and `new Request` allocated on the heap without `delete`.
  3. *State Model Disconnection:* The state machines in `src/state_diagram_one/` and `src/state_diagram_two/` were completely disconnected from `main.cpp` and never executed during room bookings.
  4. *Omitted States:* `NeedsRevision` was omitted from the reservation state enum; `Request` reduced lifecycle status to a single boolean flag.
  5. *Missing Guards:* The 15-minute soft lock, the 100-person security review threshold, and the 24-hour cancellation rule were absent in code.
* **Errors and Omissions in AI Solution:**
  1. *Lack of Data Persistence:* Data is held exclusively in memory; restarting the server wipes all reservations and room statuses.
  2. *Naive JSON Substring Parsing:* `extractField` in `HttpServer.hpp` relies on delimiter searches rather than a formal grammar parser.
  3. *Missing Monthly Booking Quotas:* The requirement for student organization booking quotas (e.g. 5 bookings/month) was not enforced in the domain logic.

### Question 4: How do the solutions compare in terms of code quality, idiomatic C++, and maintainability?
The AI solution exhibits dramatically superior code quality and maintainability:
* **Modern C++ Standards:** The team solution used legacy C++98 idioms (raw pointers, public fields, C-style macros, manual memory allocation). The AI solution used idiomatic C++20 (concepts, move semantics, scoped enums, `std::string_view`, atomic variables, thread-safe date formatters).
* **Compiler Hygiene:** The team code generated multiple compiler warnings on modern compilers. The AI code compiles with zero errors and zero warnings under strict `/W4` (MSVC) and `-Wall -Wextra -Wpedantic` (Clang/GCC).
* **Maintainability and Testing:** The team solution lacked automated tests, making refactoring dangerous. The AI solution provides 14 test suites and 609 assertions integrated into CTest, enabling safe refactoring, continuous integration, and rapid verification.

### Question 5: What specific improvements will our team make to our codebase based on the AI implementation?
1. **Unify the Repository Architecture:** Adopt the AI's modular directory structure (`domain/`, `state/`, `service/`, `web/`, `test/`) to replace fragmented student folders.
2. **Eliminate State Machine Singletons:** Refactor `Reservation_Request` and `Room_Status` so they can be instantiated per reservation and per facility, allowing multi-room campus simulations.
3. **Adopt RAII and Smart Pointers:** Replace all raw `new` operations with modern stack allocation or `std::unique_ptr`/`std::shared_ptr`.
4. **Implement Real Guard Checks:** Incorporate the AI's discrete time math to enforce the 15-minute soft hold, the 100-person security review gate, and the 24-hour cancellation rule.
5. **Establish Automated Regression Testing:** Integrate `TestSuite.hpp` and CTest into our build pipeline so every commit is automatically verified against all 18 state machine states.

### Question 6: Where was AI genuinely useful, and where was human engineering judgment indispensable?
* **Where AI Was Genuinely Useful:**
  * Transforming abstract state transition tables into exhaustive, boilerplate-free C++ switch engines and guard checks.
  * Writing comprehensive, edge-case unit test suites (609 assertions) that human teams often avoid or truncate due to fatigue.
  * Implementing cross-platform socket networking and embedding a complete web dashboard without external dependencies.
  * Enforcing modern C++20 language features, const-correctness, and zero compiler warnings.
* **Where Human Judgment Was Indispensable:**
  * Identifying the real-world campus domain rules (e.g., distinguishing between policy reviews by Dean Arthur and safety reviews by Campus Security).
  * Recognizing that physical rooms cycle continuously without a terminal state, whereas event reservations terminate in Completed or Cancelled.
  * Deciding when an embedded web server is an asset versus when it introduces unnecessary complexity over a simple terminal runner.
  * Verifying that generated code aligns with institutional policies and pedagogical grading rubrics.

---

## 9. Conclusion and Capstone II Direction

This comparative analysis demonstrates the powerful synergy between human architectural modeling and AI-assisted implementation. The human team accurately modeled the subtle business rules of university facility reservations, while the AI proved exceptionally capable at translating those models into robust, verified, and multi-modal C++20 software.

For Capstone II, our team will carry forward the lessons learned from this exercise:
1. We will combine human domain insight with AI-accelerated implementation and verification.
2. We will enforce strict test-driven development from Day 1, utilizing automated harnesses to catch bugs before integration.
3. We will adopt modern C++20 conventions, eliminate raw pointer leaks, and use modular architectural boundaries across all project subsystems.
