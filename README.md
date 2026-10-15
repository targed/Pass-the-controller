# Pass the Controller

> **Social Game Streaming & Virtual Controller Passing Platform**  
> Computer Science Senior Capstone Project • Team Repository

---

## About the Project
**Pass the Controller** is a desktop social gaming platform designed to recreate the couch co-op experience of playing a game with friends in the same room and taking turns by "passing the controller."

The application enables remote game streaming combined with a real-time virtual controller token passing engine, spectator queue, text channels, and low-latency voice.

---

## Repository Branches: How We Work Together
To ensure that all 6 team members can collaborate, learn, and contribute meaningfully to our capstone grading:

* **`main` (You are here):** The collaborative project foundation. It contains the monorepo architecture, runnable starter templates, protocol contracts, and component README guides for each sub-team to build upon.
* **`playground` (`git checkout playground`):** Preserves the team's initial verified end-to-end proof-of-concept. Team members can consult this branch at any time for working code patterns, driver logic, and reference implementations.

---

## Architecture & Component Ownership

```
Pass-the-controller/
├── client/              # Desktop App (Electron + React 19 + TypeScript)
│   # Leads: Lyla Graham (UI/UX) & Simon Leeker (WebRTC Media)
│   # Read: client/README.md
│
├── signaling/           # WebRTC Signaling & Room State (FastAPI + WebSockets)
│   # Leads: Michael Allen (Backend) & Simon Leeker (Networking)
│   # Read: signaling/README.md
│
├── sidecar/             # Native Gamepad Driver Micro-Daemon
│   # Leads: Sam Hajdukiewicz (Host Engine) & Joshua Dobbs (QA/Testing)
│   # Read: sidecar/README.md
│
├── infra/               # Local Docker Infrastructure (Signaling + Coturn STUN/TURN)
│   # Lead: Luke Kaiser (Platform & DevOps)
│   # Read: infra/README.md
│
├── documentation/       # Comprehensive Technical Documentation (Diátaxis)
└── Docs/                # Capstone Project Proposals, Strategy, & Prototype Images
```

---

## Quickstart for Team Members

### 1. Boot Backend Infrastructure
```bash
docker compose -f infra/docker-compose.yml up -d
```

### 2. Launch the Desktop Client
```bash
cd client
npm install
npm run dev
```

### 3. Start the Input Sidecar (Mock Mode)
```bash
python3 sidecar/agent_mock.py --port 9876
```

---

## Running Automated Tests
```bash
# Python backend & sidecar tests
PYTHONPATH=signaling pytest signaling/tests -v
pytest sidecar/tests -v

# Client TypeScript typecheck & production build
cd client && npm run build
```

---

## Documentation
* **[Getting Started Tutorial](file:///home/targed/Documents/Git/Pass-the-controller/documentation/tutorials/getting-started.md)**
* **[Developer Workflows & How-To Guides](file:///home/targed/Documents/Git/Pass-the-controller/documentation/how-to/developer-workflows.md)**
* **[Architecture & Protocol Reference](file:///home/targed/Documents/Git/Pass-the-controller/documentation/reference/architecture-and-specs.md)**
* **[Project Status & Roadmap](file:///home/targed/Documents/Git/Pass-the-controller/documentation/explanation/project-status-and-roadmap.md)**
* **[Platform Architecture Strategy](file:///home/targed/Documents/Git/Pass-the-controller/Docs/Platform%20and%20Developer%20Experience%20Strategy.md)**

---

## The Capstone Team
* **Luke Kaiser:** DevOps, Infrastructure, Platform & DevX Lead
* **Lyla Graham:** Frontend Developer & UI/UX Lead
* **Simon Leeker:** Networking & Distributed Protocols Lead
* **Michael Allen:** Product Owner & Backend/Signaling Lead
* **Sam Hajdukiewicz:** System Architect & Host Input Engine Lead
* **Joshua Dobbs:** QA, Testing & Multi-OS Client Lead