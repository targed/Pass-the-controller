# Explanation: Project Status, Rationale & Roadmap

This document outlines the background, current development state, architectural decisions, and upcoming milestones for **Pass the Controller**.

---

## 1. Context & Motivation

Online multiplayer gaming is everywhere, but the experience of **taking turns on a single-player game with friends sitting on the same couch** remains awkward over the internet:
* Discord allows screen streaming, but friends cannot interact with the game.
* Steam Remote Play Together supports shared inputs, but it requires games to have native local multiplayer, works poorly across non-Steam titles, and does not provide an intuitive "pass the controller" turn queue.
* Remote desktop tools (Parsec, Moonlight) give full desktop control, which creates security risks (friends can access your files or Alt+Tab) and lacks a social lobby with chat and turn queues.

**Pass the Controller** bridges this gap: a social room with text chat and voice where a host streams their game, and friends can wait in a queue to take turns holding a virtual Xbox controller.

---

## 2. What We Have Built So Far (Sprint 0 Foundation)

During our initial setup phase, we focused on laying a reliable foundation so all 6 team members could work concurrently without blocking each other:

1. **Multi-OS Local Development Stack:**
   * Solved the mixed-OS problem (Windows, Linux, macOS) by running Node.js and Electron natively on everyone's workstation for instant UI hot-reloading and hardware gamepad access, while containerizing backend services via Docker Compose.
2. **Signaling & Controller Queue Server:**
   * Implemented a Python FastAPI service (`signaling/app/main.py`) running over WebSockets.
   * Handles room creation via codes, participant management, text chat routing, WebRTC SDP/ICE candidate exchange, and token arbitration (`request_controller`, `pass_controller`, `reclaim_controller`).
3. **Native Controller Input Sidecar:**
   * Separated OS-level input injection into a lightweight micro-daemon (`sidecar/`).
   * **Windows:** Implemented Xbox 360 controller emulation via `vgamepad` and `ViGEmBus`.
   * **Linux:** Implemented kernel-level input injection via `/dev/uinput` (`python-evdev`).
   * **Cross-Platform Mock:** Built `agent_mock.py` so Mac and Linux teammates can test the complete host and guest flows without needing Windows kernel drivers.
4. **Desktop Application Client:**
   * Scaffolded an Electron + Vite + React 19 + TypeScript application.
   * Implemented a mid-2000s retro cyber design system (brushed metal panels, bevels, glowing status LEDs).
   * Implemented a 60 Hz HTML5 Gamepad API polling hook and real-time visualizer HUD.
   * Built screen capture selection using Electron's `desktopCapturer`.
5. **Automated Testing & CI/CD:**
   * Written test suites for signaling lifecycle (`test_signaling.py`) and sidecar deadzone parsing (`test_sidecar.py`).
   * Created GitHub Actions workflows for continuous integration on PRs and multi-platform packaging for releases (`.exe`, `.AppImage`).

---

## 3. What Remains to Be Done (Roadmap)

Our capstone spans two semesters. Below is the breakdown of upcoming work:

### Phase 1: Completing the P2P WebRTC Media Loop (Immediate Priority)
* **Status:** In progress.
* **Goal:** While the host can capture their screen locally and guests can send controller inputs, the actual P2P `RTCPeerConnection` video and audio tracks need to be negotiated between host and spectator clients using our signaling server and Coturn STUN/TURN relay.
* **Owners:** Simon (WebRTC negotiation) & Sam (media pipeline).

### Phase 2: Hardware Validation with Steam Games
* **Status:** Next sprint.
* **Goal:** Test the Windows `vgamepad`/ViGEmBus sidecar on actual Windows gaming machines with real single-player games (e.g., *Hollow Knight*, *Elden Ring*, *Celeste*) to measure end-to-end latency, button responsiveness, and stick deadzones under load.
* **Owners:** Sam (Driver integration) & Joshua (QA & latency measurements).

### Phase 3: Retro UI Polish & Sound Design
* **Status:** Ongoing.
* **Goal:** Expand Lyla's mid-2000s aesthetic:
  * WinAMP/AIM-style colorway themes.
  * Retro sound effects (sound on turn pass, button clicks, join/leave chime).
  * Smooth animations and responsive layout adjustments.
* **Owner:** Lyla.

### Phase 4: Room Discovery & Session Management
* **Status:** Scheduled for Semester 2.
* **Goal:** Move beyond in-memory room codes to persistent room states, friend lists, or lightweight SQLite storage if desired.
* **Owner:** Michael.

### Extensions (Stretch Goals)
* **Retroactive 30-Second Clip Capture:** Allow spectators to press a button that saves a clip of the last 30 seconds of gameplay.
* **Host Swap:** Allow another player in the room to take over hosting without disbanding the room.
* **Keyboard & Mouse Support:** Sandboxed mouse control for strategy/point-and-click games.

---

## 4. Key Architectural Decisions Explained

### Why a Unified Electron App Instead of Pure Web or Pure C++?
* **Pure Web App:** Browsers run in a security sandbox and **cannot** create virtual Xbox controllers on Windows to control Steam games. A native host agent is strictly required.
* **Pure C++ (Qt / ImGui):** Writing a full Discord-like chat client with channels, settings modals, and custom themes in C++ would take months of boilerplate.
* **Electron Sweet Spot:** Gives us the best of both worlds:
  * Lyla can write modern, responsive CSS/React for the UI.
  * Chromium provides built-in hardware-accelerated screen capture (`desktopCapturer`) and WebRTC video decoding.
  * Node.js can easily spawn our native background driver sidecar on the host machine.

### Why WebRTC Instead of XMPP / Jingle?
Our early notes explored XMPP and Jingle. While Jingle pioneered P2P media in 2005, it has been superseded by WebRTC (which Google standardized from the same roots in 2011). WebRTC has first-class browser support, native ICE/STUN/TURN NAT traversal, adaptive bitrate, and low-latency UDP DataChannels, whereas XMPP libraries in Python/C++ are mostly unmaintained today.

### Why a Lightweight Input Sidecar Instead of Custom C++ Video Capture?
Building custom DXGI desktop duplication, NVENC hardware encoding, and packet retransmission in C++ is a multi-month systems project in itself. By letting Electron capture the screen and stream video, our custom systems code only needs to handle gamepad input injection. This reduced our host driver code to under 200 lines of focused, testable Python/C++ logic.
