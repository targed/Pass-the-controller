# Sprint 0 Implementation Plan: Platform & Foundation Scaffold
**Project:** Pass the Controller  
**Author:** Luke Kaiser (Platform, Infra & DevX Lead)  
**Status:** In Progress (Execution Phase)  
**Target Completion:** Sprint 0 Baseline

---

## 1. Objectives & Deliverables

The goal of Sprint 0 is to eliminate all development friction across our mixed-OS team of 6 (Windows, Linux, macOS) by establishing a working monorepo scaffold, turn-key local infrastructure, and verified build pipelines.

### Key Deliverables
1. **Monorepo Root**:
   - `package.json` and `pnpm-workspace.yaml` coordinating `/client`.
   - `.gitignore` configured for Node, Python, Electron, and OS artifacts.
   - Central development scripts (`pnpm dev`, `pnpm build`, `pnpm test`).
2. **Infrastructure (`/infra` & `/signaling`)**:
   - `infra/docker-compose.yml`: Spins up local Signaling Server + Coturn (STUN/TURN) in one command.
   - `infra/coturn/turnserver.conf`: Lightweight STUN/TURN configuration with local authentication.
   - `signaling/`: Python FastAPI WebSocket service handling room codes, WebRTC SDP offer/answers, ICE candidate relays, and controller token arbitration.
   - `signaling/Dockerfile` & `signaling/requirements.txt`.
3. **Native Controller Input Sidecar (`/sidecar`)**:
   - `sidecar/protocol.json` / `protocol.md`: Binary/JSON gamepad state specification (buttons, triggers, thumbsticks).
   - `sidecar/agent_mock.py`: Cross-platform mock daemon for macOS/Linux developers to test controller passing without Windows drivers.
   - `sidecar/agent_win.py`: Windows controller injection daemon using `vgamepad` (ViGEmBus).
   - `sidecar/agent_linux.py`: Linux controller injection daemon using `/dev/uinput`.
   - `sidecar/test_sidecar.py`: Automated pytest test suite verifying packet serialization.
4. **Unified Desktop Application (`/client`)**:
   - Modern Electron + Vite + React + TypeScript scaffold.
   - `src/main/`: Electron main process with IPC bridges, screen capture source enumerator (`desktopCapturer`), and sidecar process supervisor.
   - `src/preload/`: ContextBridge exposing typed APIs (`window.passTheController`).
   - `src/renderer/`: Retro-inspired mid-2000s desktop UI (Lyla's design system scaffold) with:
     - Gamepad polling hook (HTML5 Gamepad API reading controllers at 60 Hz).
     - WebRTC client hook (connecting to the signaling server).
     - Controller "Token Passing" UI status bar.
     - Stream viewer and chat room layout.
   - `electron-builder.yml`: Build definitions for Windows (`.exe` NSIS / portable) and Linux (`.AppImage`, `.deb`).
5. **CI/CD Pipelines (`.github/workflows`)**:
   - `.github/workflows/ci.yml`: Runs linter, typecheck, client build, and backend tests on all PRs.
   - `.github/workflows/release.yml`: Automated cross-platform release builder matrix.

---

## 2. Step-by-Step Execution Sequence

### Phase 1: Repository Root & Workspace Configuration
- [ ] Create root `.gitignore`
- [ ] Create root `package.json` with scripts
- [ ] Create `pnpm-workspace.yaml`

### Phase 2: Signaling Server & Infrastructure
- [ ] Implement `signaling/app/main.py` with WebSocket rooms and token arbiter
- [ ] Implement `signaling/requirements.txt` and `signaling/Dockerfile`
- [ ] Create `infra/coturn/turnserver.conf`
- [ ] Create `infra/docker-compose.yml`
- [ ] Write unit test `signaling/tests/test_signaling.py`

### Phase 3: Controller Input Sidecar Micro-Daemon
- [ ] Implement `sidecar/protocol.md` defining gamepad payload contract
- [ ] Implement `sidecar/agent_mock.py` (Local WebSocket server receiving gamepad inputs)
- [ ] Implement `sidecar/agent_win.py` (ViGEmBus driver wrapper)
- [ ] Implement `sidecar/agent_linux.py` (uinput driver wrapper)
- [ ] Implement `sidecar/requirements.txt` and `sidecar/test_sidecar.py`

### Phase 4: Desktop Application Client (`/client`)
- [ ] Configure `client/package.json` with Electron, Vite, React, TypeScript, and electron-builder
- [ ] Configure `client/vite.config.ts` and `client/tsconfig.json`
- [ ] Implement Electron Main Process (`client/src/main/index.ts`)
- [ ] Implement Electron Preload Bridge (`client/src/preload/index.ts`)
- [ ] Implement Renderer UI (`client/src/renderer/`):
  - Retro mid-2000s styling (`App.css`, `index.css`)
  - Gamepad Polling Hook (`useGamepad.ts`)
  - WebRTC & Signaling Hook (`useWebRTC.ts`)
  - Main Layout with Stream, Chat, Token Controller Indicator, and Session modal
- [ ] Configure `client/electron-builder.yml`

### Phase 5: CI/CD Workflows
- [ ] Create `.github/workflows/ci.yml`
- [ ] Create `.github/workflows/release.yml`

### Phase 6: Rigorous Verification (`/verification-before-completion`)
- [ ] Verify `docker compose config` syntax
- [ ] Verify Python signaling server syntax & run pytest
- [ ] Verify Python sidecar mock agent runs and handles input payloads
- [ ] Verify client builds cleanly via TypeScript compiler (`tsc`) and Vite (`npm run build`)
- [ ] Document quickstart instructions for team members in `README.md`
