# Local Infrastructure & DevOps (`/infra`)

**Lead:** Luke Kaiser (DevOps, Infrastructure, Platform & Developer Experience)  
**Tech Stack:** Docker, Docker Compose, Coturn (STUN/TURN), Bash

---

## Component Purpose
The `/infra` directory provides one-command orchestration for the backend services required by Pass the Controller. 

Because our team develops across Linux, Windows, and macOS, running the backend services in Docker ensures that all 6 developers test against identical networking and signaling environments without "works on my machine" issues.

---

## Quick Commands for the Team

### Start All Services (Detached Mode)
From the repository root:
```bash
docker compose -f infra/docker-compose.yml up -d
```

### Check Running Status & Logs
```bash
docker compose -f infra/docker-compose.yml ps
docker compose -f infra/docker-compose.yml logs -f
```

### Stop Services
```bash
docker compose -f infra/docker-compose.yml down
```

---

## Service Architecture

| Container | Image | Ports | Description |
| :--- | :--- | :--- | :--- |
| **`ptc-signaling`** | Built from `../signaling/Dockerfile` | `8080:8080` (HTTP/WS) | Python FastAPI WebSocket server managing rooms and controller tokens. |
| **`ptc-coturn`** | `coturn/coturn:latest` | `3478:3478` (UDP & TCP) | Open-source STUN/TURN server enabling WebRTC peer connections through strict apartment/dorm NAT firewalls. |

---

## Configuration
* **STUN/TURN Settings:** Configured in `infra/coturn/turnserver.conf`.
  * Realm: `passthecontroller.local`
  * Local Dev Credentials: `ptcuser:ptcpassword123`
  * Dynamic Port Range: `49152` to `65535`
