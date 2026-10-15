# Pass the Controller: Documentation Index

Welcome to the internal engineering documentation for **Pass the Controller**. 

This documentation is structured following the [Diátaxis framework](https://diataxis.fr/) into four distinct areas:

```
documentation/
├── tutorials/       # Step-by-step walkthroughs for new contributors
├── how-to/          # Concrete recipes for common development tasks
├── reference/       # Protocol specifications, CLI commands, and file maps
└── explanation/     # Architecture rationale, status, and upcoming roadmap
```

---

## Quick Navigation

| Document | Purpose |
| :--- | :--- |
| **[Tutorial: Getting Started](file:///home/targed/Documents/Git/Pass-the-controller/documentation/tutorials/getting-started.md)** | Start here if you just cloned the repository. Walks you through running the app, local signaling, and testing controller passing in under 5 minutes. |
| **[How-To: Developer Workflows](file:///home/targed/Documents/Git/Pass-the-controller/documentation/how-to/developer-workflows.md)** | Day-to-day commands: running tests, packaging installers, testing gamepads, debugging WebSocket connections, and managing local Docker services. |
| **[Reference: Architecture & Protocols](file:///home/targed/Documents/Git/Pass-the-controller/documentation/reference/architecture-and-specs.md)** | Exact protocol schemas (WebSocket messages, gamepad payload format), file tree map, network ports, and CLI cheat sheet. |
| **[Explanation: Status & Roadmap](file:///home/targed/Documents/Git/Pass-the-controller/documentation/explanation/project-status-and-roadmap.md)** | High-level project context: what is built today, what remains to be built across upcoming sprints, and why our system is designed the way it is. |

---

## Project Overview in 60 Seconds

* **What it is:** A desktop application combining live screen sharing with a virtual controller "token" that friends pass around in a shared room.
* **Tech Stack:**
  * **Frontend / Shell:** Electron + React 19 + TypeScript + Vite.
  * **Video Streaming:** WebRTC via Chromium's `desktopCapturer` and `RTCPeerConnection`.
  * **Signaling Server:** Python FastAPI + WebSockets running in Docker.
  * **NAT Traversal:** Coturn STUN/TURN server running in Docker.
  * **Controller Emulation:** Lightweight Python sidecar (`vgamepad` on Windows for Xbox 360 emulation, `evdev` on Linux for `/dev/uinput`, with a mock daemon for development on Mac/Linux).
* **Target Audience:** Gamers who want to share single-player or turn-based games with remote friends without complicated setup.

---

## Team Roster & Ownership

| Name | Role | Primary Focus |
| :--- | :--- | :--- |
| **Luke Kaiser** | DevOps, Infra & DevX | Monorepo setup, Docker infra, CI/CD, packaging |
| **Lyla Graham** | Frontend & UI/UX | Retro mid-2000s desktop UI, layout, theme |
| **Simon Leeker** | Networking & Protocols | WebRTC peer connections, ICE negotiation, TURN relay |
| **Michael Allen** | Backend & Signaling | Room sessions, controller queue state, database (if added) |
| **Sam Hajdukiewicz** | Systems & Host Engine | Windows ViGEmBus integration, input injection |
| **Joshua Dobbs** | QA & Client Testing | Gamepad input testing, test automation, multi-OS QA |
