# Reference: Architecture, Protocols & File Map

This document is a technical reference detailing file locations, network protocols, message schemas, and CLI commands for **Pass the Controller**.

---

## 1. Repository File Tree Map

Here is the complete map of where components live in the repository:

```
Pass-the-controller/
├── .github/
│   └── workflows/
│       ├── ci.yml                 # Runs linter, typecheck & pytest on PRs
│       └── release.yml            # Multi-OS packaging matrix on tag push
├── client/                        # Desktop App (Electron + React 19 + TypeScript)
│   ├── src/
│   │   ├── main/
│   │   │   └── index.ts           # Electron Main (desktopCapturer & sidecar spawner)
│   │   ├── preload/
│   │   │   └── index.ts           # ContextBridge secure APIs
│   │   └── renderer/
│   │       ├── components/
│   │       │   ├── ChannelSidebar.tsx      # Text/Voice channel navigation
│   │       │   ├── ChatPanel.tsx           # Session chat log
│   │       │   ├── GamepadVisualizer.tsx   # Real-time controller input HUD
│   │       │   ├── MemberQueueSidebar.tsx  # Token pass/reclaim controls
│   │       │   └── ScreenStream.tsx        # Video element & window picker
│   │       ├── hooks/
│   │       │   ├── useGamepad.ts           # 60Hz Gamepad API poller
│   │       │   └── useSignaling.ts         # WebSocket room client
│   │       ├── App.tsx            # Main layout & sidecar IPC bridge
│   │       ├── index.css          # Retro mid-2000s cyber design system
│   │       └── main.tsx           # React DOM root
│   ├── electron-builder.yml       # Windows/Linux packaging rules
│   ├── package.json               # Client dependencies
│   ├── tsconfig.json              # TypeScript compiler configuration
│   └── vite.config.ts             # Vite build settings
├── documentation/                 # Diátaxis technical documentation
│   ├── README.md                  # Documentation index
│   ├── tutorials/
│   │   └── getting-started.md     # Step-by-step setup walkthrough
│   ├── how-to/
│   │   └── developer-workflows.md # Test, build & driver guides
│   ├── reference/
│   │   └── architecture-and-specs.md # This specification file
│   └── explanation/
│       └── project-status-and-roadmap.md # Technical rationale & future sprints
├── infra/                         # Containerized Infrastructure
│   ├── coturn/
│   │   └── turnserver.conf        # STUN/TURN configuration
│   └── docker-compose.yml         # Local dev service orchestration
├── sidecar/                       # Native Input Injection Micro-Daemon
│   ├── agent_linux.py             # Linux /dev/uinput daemon
│   ├── agent_mock.py              # Cross-platform mock daemon
│   ├── agent_win.py               # Windows ViGEmBus daemon
│   ├── protocol.md                # Input protocol specification
│   ├── requirements.txt           # Python dependencies (websockets, pytest)
│   └── tests/
│       └── test_sidecar.py        # Deadzone & parsing unit tests
├── signaling/                     # WebRTC Signaling & Room State Service
│   ├── app/
│   │   ├── main.py                # FastAPI WebSocket server & token queue
│   │   └── __init__.py
│   ├── Dockerfile                 # Container image build file
│   ├── requirements.txt           # Python dependencies (FastAPI, uvicorn)
│   └── tests/
│       └── test_signaling.py      # Room lifecycle & token tests
├── package.json                   # Root monorepo scripts
├── pnpm-workspace.yaml            # pnpm workspace definition
└── README.md                      # Project front-page overview
```

---

## 2. Signaling WebSocket Protocol Reference

All signaling messages travel over WebSocket to `ws://<host>:8080/ws/{room_code}/{client_id}?username={name}&is_host={bool}`. Payloads are formatted as JSON.

### Message Types

| Type | Direction | Payload Example | Purpose |
| :--- | :--- | :--- | :--- |
| `room_state` | Server -> All | `{"type": "room_state", "room_code": "LOBBY", "host_id": "c1", "controller_holder": "c1", "controller_queue": [], "members": [...]}` | Broadcast when users join/leave or initial connection establishes. |
| `chat_message` | Bidirectional | `{"type": "chat_message", "sender_id": "c2", "username": "Bob", "text": "GG!", "timestamp": 1728950000}` | Normal text message routed to all room members. |
| `webrtc_offer` | Client -> Server -> Target | `{"type": "webrtc_offer", "target_id": "c2", "sdp": "v=0..."}` | Host transmits WebRTC video stream SDP offer to a specific spectator. |
| `webrtc_answer` | Client -> Server -> Target | `{"type": "webrtc_answer", "target_id": "c1", "sdp": "v=0..."}` | Spectator transmits WebRTC SDP answer back to the host. |
| `ice_candidate` | Bidirectional | `{"type": "ice_candidate", "target_id": "c1", "candidate": {...}}` | Relays STUN/TURN ICE candidates between peers to punch firewalls. |
| `request_controller` | Client -> Server | `{"type": "request_controller"}` | Client requests to be added to the queue for a turn with the game. |
| `controller_queue_updated` | Server -> All | `{"type": "controller_queue_updated", "controller_holder": "c1", "controller_queue": ["c2", "c3"]}` | Broadcast when someone joins or leaves the queue. |
| `pass_controller` | Host/Holder -> Server | `{"type": "pass_controller", "target_id": "c2"}` | Passes the virtual controller token to a specific client. |
| `controller_passed` | Server -> All | `{"type": "controller_passed", "controller_holder": "c2", "holder_name": "Bob"}` | Broadcast when controller ownership changes. |
| `reclaim_controller` | Host -> Server | `{"type": "reclaim_controller"}` | Host forces immediate return of the virtual controller. |
| `controller_reclaimed` | Server -> All | `{"type": "controller_reclaimed", "controller_holder": "c1"}` | Broadcast confirming host reclaimed control. |

---

## 3. Host Gamepad Sidecar Protocol Reference

Communication between the Electron app and the native sidecar occurs over local WebSocket (`ws://127.0.0.1:9876`).

### Continuous Input Frame (`gamepad_state`)

```json
{
  "type": "gamepad_state",
  "seq": 450,
  "buttons": {
    "A": 1,
    "B": 0,
    "X": 0,
    "Y": 0,
    "LB": 0,
    "RB": 0,
    "BACK": 0,
    "START": 0,
    "GUIDE": 0,
    "LEFT_THUMB": 0,
    "RIGHT_THUMB": 0,
    "DPAD_UP": 0,
    "DPAD_DOWN": 0,
    "DPAD_LEFT": 0,
    "DPAD_RIGHT": 0
  },
  "triggers": {
    "LT": 0.0,
    "RT": 0.75
  },
  "axes": {
    "left_x": 0.0,
    "left_y": -0.85,
    "right_x": 0.12,
    "right_y": 0.0
  }
}
```

### Button Mapping Translation Matrix

| HTML5 API Index | Button Name | Windows ViGEmBus Constant | Linux evdev ecode |
| :---: | :---: | :--- | :--- |
| `0` | `A` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_A` | `e.BTN_A` |
| `1` | `B` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_B` | `e.BTN_B` |
| `2` | `X` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_X` | `e.BTN_X` |
| `3` | `Y` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_Y` | `e.BTN_Y` |
| `4` | `LB` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_LEFT_SHOULDER` | `e.BTN_TL` |
| `5` | `RB` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_SHOULDER` | `e.BTN_TR` |
| `6` | `LT` | `gamepad.left_trigger_float(val)` | `e.ABS_Z` (0-255) |
| `7` | `RT` | `gamepad.right_trigger_float(val)` | `e.ABS_RZ` (0-255) |
| `8` | `BACK` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_BACK` | `e.BTN_SELECT` |
| `9` | `START` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_START` | `e.BTN_START` |
| `10` | `LEFT_THUMB` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_LEFT_THUMB` | `e.BTN_THUMBL` |
| `11` | `RIGHT_THUMB` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_THUMB` | `e.BTN_THUMBR` |
| `12` | `DPAD_UP` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_UP` | `e.BTN_DPAD_UP` |
| `13` | `DPAD_DOWN` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_DOWN` | `e.BTN_DPAD_DOWN` |
| `14` | `DPAD_LEFT` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_LEFT` | `e.BTN_DPAD_LEFT` |
| `15` | `DPAD_RIGHT` | `vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_RIGHT` | `e.BTN_DPAD_RIGHT` |
| `16` | `GUIDE` | `None` | `e.BTN_MODE` |

### Value Bounds & Deadzones
* **Buttons:** `0` (released) or `1` (pressed).
* **Triggers (`LT`, `RT`):** Float from `0.0` to `1.0`.
* **Analog Sticks:** Float from `-1.0` to `+1.0`. Values within `[-0.08, +0.08]` are automatically clamped to `0.0` by the sidecar to eliminate hardware stick drift.

---

## 4. Network Ports Reference

| Port | Protocol | Service | Description |
| :---: | :---: | :--- | :--- |
| `8080` | TCP / HTTP / WS | Signaling Server | Room discovery, WebSocket session state & SDP relay |
| `3478` | UDP / TCP | Coturn STUN/TURN | ICE NAT discovery and media relay for strict firewalls |
| `5173` | TCP / HTTP | Vite Dev Server | Local React frontend hot-reloading |
| `9876` | TCP / WS | Host Input Sidecar | Local communication between Electron host and native driver |

---

## 5. Command Reference Cheat Sheet

| Task | Command |
| :--- | :--- |
| **Start Backend Infra** | `docker compose -f infra/docker-compose.yml up -d` |
| **Stop Backend Infra** | `docker compose -f infra/docker-compose.yml down` |
| **Start Frontend Dev Server** | `cd client && npm run dev` |
| **Run Electron Desktop App** | `cd client && npm start` |
| **Run Mock Sidecar** | `python3 sidecar/agent_mock.py --port 9876` |
| **Run Windows Sidecar** | `python sidecar/agent_win.py --port 9876` |
| **Run Linux Sidecar** | `python3 sidecar/agent_linux.py --port 9876` |
| **Run All Python Tests** | `PYTHONPATH=signaling pytest signaling/tests -v && pytest sidecar/tests -v` |
| **Build Client Bundle** | `cd client && npm run build` |
| **Build Desktop Installers** | `cd client && npm run package` |
