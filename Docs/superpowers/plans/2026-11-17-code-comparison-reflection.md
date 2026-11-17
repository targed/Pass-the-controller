# AI vs Team C++ Implementation Comparison and Reflection Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Author a comprehensive, rigorous technical comparison, analysis, and reflection document comparing the team's C++ solution (`class_state_in_class_activity` branch / `C++ Capstone Team Activity - Pass the Controller.pdf`) with the AI C++ solution (`AI_class_state_in_class_activity` branch), formatted after `Docs/AI ML Class and State Models.pdf`.

**Architecture:** The report follows the Diátaxis Explanation framework and the team's prior PDF submission format: Project Details, Executive Summary, Architectural Overview, Side-by-Side Comparative Matrices (8 engineering dimensions), Critical Bug & Quality Audit, Decision Log (Keep/Modify/Reject/Add), and Structured Reflection Questions answering all professor prompts.

**Tech Stack:** Markdown, Git, Diátaxis Explanation Framework, C++20 static analysis.

## Global Constraints
- Target document path: `Docs/AI_Team_Code_Comparison_and_Reflection.md`
- Template reference: `Docs/AI ML Class and State Models.pdf`
- Primary source 1: `Docs/C++ Capstone Team Activity - Pass the Controller.pdf` and git branch `origin/class_state_in_class_activity`
- Primary source 2: `AI_class_state_in_class_activity` branch code (`src/domain/`, `src/state/`, `src/service/`, `src/test/`, `src/web/`, `src/main.cpp`)
- Humanizer rules: strictly zero em dashes (—) or en dashes (–), zero "not X but Y" constructions, zero chatbot residue, objective technical tone.
- Professor requirements addressed:
  1. AI-generated development vs Team-developed solution
  2. Strengths and limitations
  3. Errors or missing components
  4. Code quality and maintainability
  5. Improvements your team would make

---

### Task 1: Comparative Data and Metrics Collection

**Files:**
- Read: `src/` on `class_state_in_class_activity` and `AI_class_state_in_class_activity`
- Output: Technical audit notes and exact line references for bugs, class hierarchies, and state coverage

- [ ] **Step 1: Extract team codebase metrics**
Document team class counts, method counts, memory leak locations, singleton limitations, and incomplete transitions.

- [ ] **Step 2: Extract AI codebase metrics**
Document AI class counts, method counts, test assertion count (609 assertions / 14 suites), web endpoints (20 REST routes), and zero-leak RAII architecture.

---

### Task 2: Document Header, Executive Summary, and Architectural Overviews

**Files:**
- Create: `Docs/AI_Team_Code_Comparison_and_Reflection.md`

- [ ] **Step 1: Write Project Details & Team Contributions**
Author team member breakdown (Luke Kaiser, Sam Hajdukiewicz, Michael Allen, Lyla Graham, Simon Leeker, Joshua Dobbs) and project metadata.

- [ ] **Step 2: Write Executive Summary & Table of Contents**
High-level summary of the comparison between the 2-day team prototype and the autonomous AI C++20 system.

- [ ] **Step 3: Document Team Solution Architecture**
Detail the team's class structure (`user_classes.h`, `facility_classes.h`), singleton state machines (`state_diagram_one`, `state_diagram_two`), and interactive terminal loop (`main.cpp`).

- [ ] **Step 4: Document AI Solution Architecture**
Detail the AI's modular architecture (`domain/`, `state/`, `service/`, `web/`, `test/`), dual state engines, web dashboard, and automated test harness.

---

### Task 3: Comprehensive Comparative Analysis Tables (8 Dimensions)

**Files:**
- Modify: `Docs/AI_Team_Code_Comparison_and_Reflection.md`

- [ ] **Step 1: Construct Comparison Matrices**
Create side-by-side tables across 8 key software engineering dimensions:
1. Class Model & Entity Decomposition
2. Attributes, Data Types, and Type Safety
3. Methods, Encapsulation, and Memory Management
4. State Machine Architecture (11-State Reservation & 7-State Facility)
5. Guard Conditions, Business Rules, and Timing Thresholds
6. Concurrency, Multi-Tenancy, and Thread Safety
7. Testing, Verification, and Assertion Coverage
8. Presentation Layer, UI/UX, and Developer Experience

---

### Task 4: In-Depth Critical Reviews and Bug Analysis

**Files:**
- Modify: `Docs/AI_Team_Code_Comparison_and_Reflection.md`

- [ ] **Step 1: Write Critical Review of Team Solution**
Analyze strengths, structural weaknesses, architectural bottlenecks (singleton anti-pattern, disconnected state machines), and concrete bugs with file:line citations:
- `user_classes.h:35` (`setEventTime` overwrites `eventName`)
- `main.cpp:97, 105, 113` (heap allocation leaks with `new Room`, `new Request`)
- `user_classes.h:20` (`Request::status` as a single `bool` instead of state enum)
- Disconnected state machines never invoked from `main.cpp`.

- [ ] **Step 2: Write Critical Review of AI Solution**
Analyze strengths, limitations, trade-offs (e.g. in-memory repository vs external relational DB, added code complexity vs prototype simplicity).

- [ ] **Step 3: Write Code Quality and Maintainability Audit**
Compare cyclomatic complexity, coupling/cohesion, const-correctness, C++ standard compliance (C++98/03 idioms vs modern C++20).

---

### Task 5: Decision Log, Reflection Q&A, and Final Polish

**Files:**
- Modify: `Docs/AI_Team_Code_Comparison_and_Reflection.md`

- [ ] **Step 1: Write Decision Log: Keep, Modify, Reject, Add**
Specify what the team would keep from their design, what they adopt/modify from AI, what they reject from AI, and what should be added next.

- [ ] **Step 2: Answer All Reflection Questions**
Exhaustive answers to the professor's core questions and advanced reflection items.

- [ ] **Step 3: Humanizer and Style Verification**
Scan for and eliminate em/en dashes, "not X but Y", and robotic jargon.

- [ ] **Step 4: Commit Report**
```bash
git add Docs/AI_Team_Code_Comparison_and_Reflection.md
git commit -m "docs: add comprehensive AI vs team code comparison and reflection report"
```
