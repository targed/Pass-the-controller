# Pass the Controller

> **Social Game Streaming & Virtual Controller Passing Platform**  
> Computer Science Senior Capstone Project

---

## About the Project
**Pass the Controller** is a social desktop application designed to recreate the classic couch co-op experience of playing a game with friends in the same room and "passing the controller" when taking turns.

While platforms like Discord or Steam Remote Play allow screen sharing, they lack a dedicated social lobby system designed around turn-based controller passing, token arbitration, and spectator queues.

### Core Features
* **Virtual Controller Passing Engine:** A synchronized token system where the room host or token holder can pass or reclaim game control on the fly.
* **Low-Latency Game Streaming:** WebRTC video and audio stream with hardware acceleration.
* **Multi-OS Native Controller Emulation:** 
  * Windows: Emulates Xbox 360 gamepads via the kernel `ViGEmBus` driver (`vgamepad`).
  * Linux: Emulates gamepads via the `/dev/uinput` subsystem (`evdev`).
* **Hardware Gamepad Polling:** Reads any connected USB or Bluetooth gamepad at 60 Hz via the HTML5 Gamepad API.
* **Social Hub:** Text channels, session voice rooms, and retro mid-2000s cyber aesthetic.

---

## Architecture & Monorepo Structure

```
Pass-the-controller/
├── client/              # Desktop App (Electron + Vite + React + TypeScript)
│   ├── src/main/        # Electron Main Process (DesktopCapturer & Sidecar Supervisor)
│   ├── src/preload/     # Secure ContextBridge APIs
│   ├── src/renderer/    # Retro mid-2000s UI, Gamepad visualizer, WebRTC stream
│   └── electron-builder.yml
├── signaling/           # WebRTC Signaling & Token Arbiter (FastAPI + WebSockets)
│   ├── app/main.py      # Room sessions, token queue, SDP & ICE exchange
│   └── Dockerfile
├── sidecar/             # Native Gamepad Injection Micro-Daemon
│   ├── agent_win.py     # Windows ViGEmBus implementation (Xbox 360 controller)
│   ├── agent_linux.py   # Linux /dev/uinput implementation
│   ├── agent_mock.py    # Cross-platform mock daemon for macOS/Linux devs
│   └── protocol.md      # Gamepad serialization specification
├── infra/               # Local Dev & Production Infrastructure
│   ├── docker-compose.yml # Signaling + Coturn STUN/TURN in one command
│   └── coturn/          # STUN/TURN configuration for apartment/dorm NAT traversal
├── .github/workflows/   # CI/CD Workflows (PR lint/tests + multi-platform release matrix)
└── Docs/                # Project planning, proposals, and architecture strategy
```

---

## Quickstart for Developers

The codebase is structured so all 6 team members can develop on their preferred operating system (Windows, Linux, macOS) without configuration friction.

### Prerequisites
* **Node.js:** v20+ LTS
* **Python:** 3.10+
* **Docker & Docker Compose** (for backend infrastructure)

### 1. Start Local Infrastructure (Signaling & STUN/TURN)
In a terminal, run:
```bash
docker compose -f infra/docker-compose.yml up -d
```
* Signaling WebSocket server: `ws://localhost:8080`
* Coturn STUN/TURN server: `stun:localhost:3478`

### 2. Start the Desktop Client
In another terminal:
```bash
cd client
npm install
npm run dev
```
* The Vite dev server will start at `http://localhost:5173`.
* You can open it in your browser or run inside Electron:
```bash
npm start
```

### 3. (Optional) Run the Input Sidecar Daemon
To test controller injection locally:
* **macOS / Linux / Windows (Mock):**
  ```bash
  python3 sidecar/agent_mock.py --port 9876
  ```
* **Windows (Native ViGEmBus):**
  ```bash
  pip install -r sidecar/requirements.txt vgamepad
  python sidecar/agent_win.py --port 9876
  ```
* **Linux (Native /dev/uinput):**
  ```bash
  pip install -r sidecar/requirements.txt evdev
  python3 sidecar/agent_linux.py --port 9876
  ```

---

## Running Automated Tests

### Python Backend & Sidecar Tests
```bash
PYTHONPATH=signaling pytest signaling/tests -v
pytest sidecar/tests -v
```

### Client TypeScript Typecheck & Build
```bash
cd client
npm run build
```

---

## Building Distributable Installers

To package the application for end users:
```bash
cd client
npm run package
```
* **Windows:** Produces `.exe` installer (NSIS) and portable `.exe` in `client/release/`.
* **Linux:** Produces `.AppImage` and `.deb` in `client/release/`.

Our GitHub Actions workflow automatically builds both platforms whenever a version tag (`v*.*.*`) is pushed.

---

## 👥 The Team
* **Luke Kaiser:** DevOps, Infrastructure, Platform & DevX Lead
* **Lyla Graham:** Frontend Developer & UI/UX Lead
* **Simon Leeker:** Networking & Protocols Lead
* **Michael Allen:** Product Owner & Backend/Signaling Lead
* **Sam Hajdukiewicz:** System Architect & Host Input Engine Lead
* **Joshua Dobbs:** QA, Testing & Client Input Lead

---

## Project Documentation
* [Platform and Developer Experience Strategy](file:///home/targed/Documents/Git/Pass-the-controller/Docs/Platform%20and%20Developer%20Experience%20Strategy.md)
* [Project Proposal](file:///home/targed/Documents/Git/Pass-the-controller/Docs/Project%20Proposal.md)
* [Project Planning](file:///home/targed/Documents/Git/Pass-the-controller/Docs/Project%20Planning.md)
* [Host Input Protocol Specification](file:///home/targed/Documents/Git/Pass-the-controller/sidecar/protocol.md)