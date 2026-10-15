# Platform, Infrastructure & Developer Experience Strategy
**Project:** Pass the Controller (CS Capstone)  
**Lead:** Luke Kaiser (DevOps, Infrastructure, Platform & Developer Experience)  
**Target:** Team of 6 (Mixed OS: Windows, Linux, macOS)  
**Date:** October 2026

---

## 1. Executive Summary & Verdict

As the Infrastructure and Platform Lead, you are tasked with solving two foundational engineering challenges:
1. **Developer Experience (DevX):** Enabling a team of 6 with diverse operating systems (Linux, Windows, macOS) and disparate skill levels to develop, test, and integrate seamlessly without "it works on my machine" friction.
2. **Build & Distribution:** Delivering an ultra-low-latency, cross-platform desktop application to end-user gamers without burdening them with manual driver installations, CLI commands, or fragmented multi-app setups.

### The Verdict

| Decision Area | Verdict | Why It Wins |
| :--- | :--- | :--- |
| **Dev Environment (Mixed OS)** | **Hybrid Local Stack + Dockerized Infra** | A full VS Code Dev Container cannot access host GPUs for hardware capture or pass low-level virtual gamepads on Windows/Mac hosts. Running the client natively via Node.js/Electron gives instant hot-reload and native gamepad access everywhere, while Docker Compose isolates the Signaling and Coturn (STUN/TURN) servers. |
| **App Architecture** | **Unified All-in-One Electron App** | A single installable desktop app for all users. Client mode (watching, chat, voice, gamepad input) runs in pure web/Electron technologies. Host mode spawns a bundled background micro-sidecar for OS gamepad injection. |
| **Streaming Division of Labor** | **Electron WebRTC Video + Micro Input Sidecar** | **Crucial time-saver:** Electron’s Chromium core already provides hardware-accelerated screen capture (`desktopCapturer`) and WebRTC video streaming out of the box. The native sidecar only handles virtual controller emulation (`ViGEmBus` on Windows, `/dev/uinput` on Linux). |
| **Packaging & CI/CD** | **Electron-Builder + GitHub Actions Matrix** | Automatic multi-platform compilation. GitHub Actions builds the Windows `.exe` and Linux `.AppImage`/`.deb` in the cloud on every release tag, eliminating the need for local cross-compilation. |
| **Driver Strategy** | **In-App Detection + One-Click Install** | The desktop app detects if the Windows ViGEmBus driver is present when Host mode is toggled; if missing, it presents a friendly modal with a one-click button to download and run the official signed installer. |

---

## 2. Area 1: Team Development Across Mixed OSes

### Why Full VS Code Dev Containers Fail for Game Streaming
While VS Code Dev Containers are industry standard for web services and backend APIs, **they break down when applied to low-level desktop game streaming and controller emulation:**
1. **GPU & Screen Capture Virtualization:** Docker on Windows and macOS runs inside a lightweight virtual machine (WSL2 or Apple Hypervisor). A containerized process cannot capture the host OS's primary DirectX 11/12 display frame buffers.
2. **Virtual Controller Injection:** On Windows, virtual gamepad emulation requires communicating with the kernel-level `ViGEmBus` driver. A Linux Docker container cannot call Windows kernel drivers.
3. **GUI Development Lag:** Testing an Electron or web GUI inside a Docker container requires forwarding displays via X11/Wayland sockets or VNC, which introduces rendering latency, breaks hardware acceleration, and degrades developer velocity.

---

### The Recommended Developer Strategy: Tiered Development

```
+-----------------------------------------------------------------------------------+
|                           LOCAL DEVELOPER WORKSTATION                             |
|               (Windows, Linux [Debian/Arch/Fedora], or macOS)                     |
|                                                                                   |
|  [ TIER 1: Native Local Client ]               [ TIER 2: Docker Compose Infra ]  |
|  - Node.js (v20 LTS via nvm / fnm)             - Signaling Server (WebSocket)    |
|  - Vite + React / Svelte                       - Coturn (STUN / TURN Relay)      |
|  - Electron Desktop Shell                      - PostgreSQL / SQLite (Optional)  |
|  - Native HTML5 Gamepad API                                                       |
|                                                                                   |
|  [ TIER 3: Host Driver Sandbox (OS-Specific) ]                                    |
|  - Windows Developers (Sam / Luke): Direct ViGEmBus testing via Python vgamepad   |
|  - Linux Developers (Simon / Joshua): Direct /dev/uinput testing via evdev        |
+-----------------------------------------------------------------------------------+
```

#### Tier 1: Native Local Client (Everyone on the Team)
* **What runs here:** Electron frontend, retro UI components, chat messaging UI, WebRTC media player, client gamepad polling.
* **Why it works on all OSes:** Node.js, Vite, and Electron run identically on Windows, Linux, and macOS.
* **Setup for teammates:**
  ```bash
  git clone https://github.com/targed/Pass-the-controller.git
  cd Pass-the-controller/client
  pnpm install
  pnpm dev
  ```
* **Lyla & Joshua benefit:** Lyla can design and hot-reload the UI instantaneously on whatever machine she prefers. Joshua can test client-side gamepad inputs using standard USB/Bluetooth gamepads plugged directly into his PC/Mac.

#### Tier 2: Containerized Infrastructure (Docker Compose)
* **What runs here:** Signaling WebSocket server and the Coturn STUN/TURN media relay.
* **Why it works:** Michael and Simon can develop the backend signaling and room state protocols without worrying about local Python or database version discrepancies.
* **Single command for all developers:**
  ```bash
  # From project root:
  docker compose up -d
  ```
* Any developer immediately has a functioning local signaling server at `ws://localhost:8080` and STUN/TURN server at `stun:localhost:3478`.

#### Tier 3: Platform Host Sidecar (Targeted Development)
* **Windows Host Engine (Sam & Luke):** Developed natively on Windows using Python (`vgamepad`) or C++ (`ViGEmClient`).
* **Linux Host Engine (Simon & Joshua):** Developed on Linux using `/dev/uinput` (`python-evdev`).
* **Mock Sidecar for Non-Windows Developers:** Provide a mock dummy sidecar script (`sidecar_mock.py`) that logs received controller inputs to stdout, allowing macOS/Linux developers to test the full end-to-end host UI flow without needing the physical Windows driver.

---

## 3. Area 2: Application Architecture, Packaging & Distribution

### The Architectural Blueprint

```
+------------------------------------------------------------------------------------+
|                         PASS-THE-CONTROLLER DESKTOP APP                            |
|                                                                                    |
|  +------------------------------------------------------------------------------+  |
|  |                            RENDERER PROCESS (UI)                             |  |
|  |  - Retro WinAMP/AIM-Themed Interface (Lyla)                                  |  |
|  |  - WebRTC Video Display Element (<video> hardware-accelerated playback)      |  |
|  |  - HTML5 Gamepad Poller (reads client gamepad axes/buttons at 60Hz)          |  |
|  |  - Text Chat & Session State Manager                                         |  |
|  +------------------------------------------------------------------------------+  |
|                                       ^                                            |
|                                       | IPC Channel                                |
|                                       v                                            |
|  +------------------------------------------------------------------------------+  |
|  |                           ELECTRON MAIN PROCESS                              |  |
|  |  - Window Lifecycle & Native OS Menus                                        |  |
|  |  - WebRTC Video Streamer (desktopCapturer -> RTCPeerConnection)              |  |
|  |  - WebRTC DataChannel (receives guest controller packets)                    |  |
|  |  - Sidecar Process Supervisor (spawns/monitors host-agent binary)             |  |
|  +------------------------------------------------------------------------------+  |
|                                       | Local WebSocket / Stdin Pipe               |
|                                       v                                            |
|  +------------------------------------------------------------------------------+  |
|  |                 BUNDLED HOST AGENT SIDECAR (Micro-Daemon)                    |  |
|  |  - Receives button bitmask & thumbstick coordinates                          |  |
|  |  - Windows: Emulates Xbox 360 Controller via ViGEmBus (vgamepad)             |  |
|  |  - Linux: Emulates Gamepad via Linux Kernel uinput (/dev/uinput)             |  |
|  +------------------------------------------------------------------------------+  |
+------------------------------------------------------------------------------------+
```

### The "Game-Changer" Streaming Division of Labor
In traditional custom streaming engines (like Parsec or Moonlight), teams spend months writing C++ DXGI desktop capture, CUDA/NVENC hardware video encoding, packet loss retransmission, and audio loopback capture.

**By utilizing Electron's Chromium subsystem, your team gets this for free:**
1. **Screen Capture:** Electron exposes `desktopCapturer.getSources({ types: ['screen', 'window'] })`. The host selects a monitor or specific game window.
2. **Video & Audio Stream:** Captured via standard `navigator.mediaDevices.getUserMedia` with hardware-accelerated H.264/VP9 encoding built directly into Chromium.
3. **Transport:** Transmitted directly to the guest over standard `RTCPeerConnection` with native ICE/STUN/TURN NAT traversal.
4. **Latency:** Under 20-40ms on local networks/good connections.
5. **The Sidecar's ONLY Job:** When the guest moves a joystick or taps an 'A' button, the packet travels over `RTCDataChannel` (low-latency UDP/SCTP) to the host Electron app. Electron forwards this state to the local sidecar, which writes it directly to the virtual gamepad driver.

---

### Packaging & Distribution Strategy

#### Target Deliverables
* **Windows (95% of PC Gamers):**
  * `Pass-The-Controller-Setup-<version>.exe` (NSIS Installer)
  * `Pass-The-Controller-Portable-<version>.exe` (Standalone executable, no install required)
* **Linux (Steam Deck / Team Devs):**
  * `Pass-The-Controller-<version>.AppImage` (Universal Linux executable)
  * `pass-the-controller_<version>_amd64.deb` (Debian/Ubuntu package)
* **macOS (Guest / Chat mode):**
  * `Pass-The-Controller-<version>.dmg` (Universal binary)

#### Bundling the Native Sidecar
1. The Python input daemon is frozen into a standalone executable using `PyInstaller`:
   * On Windows runner: `pyinstaller --onefile sidecar/agent_win.py -n host-agent.exe`
   * On Linux runner: `pyinstaller --onefile sidecar/agent_linux.py -n host-agent`
2. `electron-builder` includes this binary inside the Electron application package under `resources/bin/`:
   ```json
   "extraResources": [
     {
       "from": "sidecar/dist/",
       "to": "bin/",
       "filter": ["host-agent*", "!*.py"]
     }
   ]
   ```
3. When the host initiates a session, Electron spawns the binary using Node's `child_process.spawn()`:
   ```javascript
   const sidecarPath = path.join(process.resourcesPath, 'bin', process.platform === 'win32' ? 'host-agent.exe' : 'host-agent');
   const sidecarProcess = spawn(sidecarPath, ['--port', '9876']);
   ```

#### ViGEmBus Driver Handling
To prevent installation roadblocks:
* The host agent detects if `ViGEmBus` is present on the Windows machine.
* If absent, the Electron app displays a clean modal:
  > **Virtual Gamepad Driver Missing**  
  > To allow friends to control games on your PC, Windows requires the open-source ViGEmBus driver.  
  > `[ Download & Install Driver ]` *(Launches official signed MSI installer)*

---

## 4. Repository Structure (Monorepo Blueprint)

```
Pass-the-controller/
├── .github/
│   └── workflows/
│       ├── ci.yml                 # Linting, unit tests on all PRs
│       └── release.yml            # Multi-OS packaging matrix on tag push
├── client/                        # Unified Desktop Application
│   ├── src/
│   │   ├── main/                  # Electron Main Process (IPC, WebRTC host, sidecar spawner)
│   │   ├── preload/               # Electron Preload Scripts (Secure bridge)
│   │   └── renderer/              # Web Frontend (Lyla's UI, chat, video element, gamepad reader)
│   ├── package.json
│   ├── electron-builder.yml       # Packaging configurations for Win/Linux/Mac
│   └── vite.config.ts
├── sidecar/                       # Native Controller Injection Micro-Daemon
│   ├── agent_win.py               # Windows implementation (vgamepad / ViGEmBus)
│   ├── agent_linux.py             # Linux implementation (evdev / uinput)
│   ├── agent_mock.py              # Cross-platform mock for macOS/Linux devs
│   ├── build.py                   # PyInstaller freezing script
│   └── requirements.txt
├── signaling/                     # Signaling & Room State Server
│   ├── app/
│   │   ├── main.py                # FastAPI WebSocket server for SDP exchange & rooms
│   │   ├── room_manager.py        # Token passing queue & timeout enforcement
│   │   └── schemas.py
│   ├── Dockerfile
│   └── requirements.txt
├── infra/                         # DevOps & Deployment Configurations
│   ├── coturn/
│   │   └── turnserver.conf        # STUN/TURN server configuration
│   └── docker-compose.yml         # Local dev orchestration (Signaling + Coturn)
├── docs/                          # Project documentation, diagrams, mockups
├── package.json                   # Root monorepo workspace definition (pnpm)
└── README.md
```

---

## 5. Automated CI/CD Pipeline (GitHub Actions)

As Infrastructure Lead, you will configure two GitHub Actions workflows:

### 1. CI Workflow (`.github/workflows/ci.yml`)
* **Triggers:** Push to `main`, all Pull Requests.
* **Jobs:**
  * `lint-and-typecheck`: Runs ESLint, Prettier, and TypeScript compiler check on `/client`.
  * `test-signaling`: Runs pytest test suite on `/signaling`.
  * `test-sidecar`: Runs controller serialization unit tests on `/sidecar`.

### 2. Release & Packaging Matrix (`.github/workflows/release.yml`)
* **Trigger:** Git tag creation (`v*.*.*`).
* **Matrix Strategy:**
  ```yaml
  strategy:
    matrix:
      os: [windows-latest, ubuntu-latest, macos-latest]
  ```
* **Steps per runner:**
  1. Freeze sidecar binary for target OS via PyInstaller.
  2. Install client dependencies via `pnpm install`.
  3. Execute `pnpm run build` (Vite compilation).
  4. Execute `pnpm exec electron-builder --publish always`.
* **Output:** GitHub Releases automatically populated with:
  * `Pass-The-Controller-Setup-1.0.0.exe`
  * `Pass-The-Controller-1.0.0.AppImage`
  * `pass-the-controller_1.0.0_amd64.deb`
  * `Pass-The-Controller-1.0.0.dmg`

---

## 6. Sprint 0 Action Plan for Luke (DevOps & Platform Lead)

| Milestone | Deliverable | Est. Hours | Impact on Team |
| :--- | :--- | :--- | :--- |
| **Step 1** | **Monorepo & Workspace Scaffold** | 3 hrs | Sets up root `pnpm` workspace with `/client`, `/signaling`, and `/sidecar` skeletons. |
| **Step 2** | **Docker Compose Dev Stack** | 2 hrs | Delivers `docker compose up -d` running local Signaling + Coturn STUN/TURN. |
| **Step 3** | **Electron + Vite Starter Shell** | 3 hrs | Unblocks **Lyla** to begin styling the retro UI without wrestling with Electron config. |
| **Step 4** | **Input Sidecar Interface Spec** | 2 hrs | Defines JSON protocol (`{ button: "A", state: 1 }`) so **Sam** and **Joshua** can build controller injection. |
| **Step 5** | **GitHub Actions CI/CD Scaffold** | 3 hrs | Automates continuous build checks and releases for all platforms. |

---

## 7. Summary Checklist for Team Meeting

- [x] **Dev Environment:** Everyone develops locally with Node.js/Electron; infrastructure runs in Docker Compose.
- [x] **Application Type:** Single unified desktop application built on Electron.
- [x] **Host Architecture:** Electron handles screen capture & WebRTC video; sidecar handles virtual controller input.
- [x] **Controller Driver:** Windows ViGEmBus driver detected at runtime with in-app one-click installation.
- [x] **Cross-Platform Delivery:** GitHub Actions builds Windows `.exe` and Linux `.AppImage` automatically.
