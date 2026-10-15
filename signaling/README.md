# Signaling & Room Management Service (`/signaling`)

**Primary Lead:** Michael Allen (Backend & Session State)  
**Collaborator:** Simon Leeker (Networking & Distributed Protocols)  
**Tech Stack:** Python 3.11+, FastAPI, WebSockets, Pydantic, pytest

---

## Component Purpose
The `/signaling` service is the central nervous system for Pass the Controller. It does **not** stream video frames directly (video travels peer-to-peer over WebRTC). Instead, it acts as a lightweight coordination server that:
1. Manages temporary room sessions identified by short room codes (e.g. `LOBBY`, `GAME1`).
2. Relays WebRTC signaling payloads (SDP Offer/Answer and ICE candidates) between peers so they can punch through NATs.
3. Broadcasts real-time text chat messages.
4. Arbitrates the **Virtual Controller Token** (manages the FIFO queue of waiting players, token passes, and host emergency reclaims).

> **Reference Implementation Available:** A fully verified proof-of-concept of this signaling service is preserved on the `playground` branch (`git checkout playground`). Consult `signaling/app/main.py` there for working logic.

---

## Running the Service

### Option A: Via Docker Compose (Recommended)
From the repository root:
```bash
docker compose -f infra/docker-compose.yml up -d signaling
docker compose -f infra/docker-compose.yml logs -f signaling
```

### Option B: Local Python Development
```bash
cd signaling
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt

uvicorn app.main:app --host 0.0.0.0 --port 8080 --reload
```

---

## Running Automated Tests
```bash
cd signaling
PYTHONPATH=. pytest tests -v
```

---

## WebSocket Endpoint & Message Schema

* **Endpoint URL:** `ws://<host>:8080/ws/{room_code}/{client_id}?username={name}&is_host={bool}`

### Key Events to Implement

#### 1. WebRTC SDP & ICE Relay
```json
{
  "type": "webrtc_offer" | "webrtc_answer" | "ice_candidate",
  "target_id": "client_abc",
  "payload": { ... }
}
```
* **Behavior:** When received from a peer, forward it directly to `target_id`. If `target_id` is omitted, broadcast to all other members in the room.

#### 2. Chat Message
```json
{
  "type": "chat_message",
  "text": "Hello world!"
}
```
* **Behavior:** Wrap with sender's `client_id`, `username`, and `timestamp`, then broadcast to everyone in the room.

#### 3. Controller Token Queue
* `request_controller`: Client asks to join the waiting queue.
* `pass_controller`: Current holder or host passes token to `target_id`.
* `reclaim_controller`: Host overrides and immediately returns token to host.

#### 4. Disconnect Handling (Important Edge Case!)
* If a spectator disconnects: remove from room and queue.
* If the **current controller holder** disconnects: automatically return control to the host.
* If the **host** disconnects: notify everyone or appoint a new host.

---

## Directory Structure
```
signaling/
├── app/
│   ├── main.py          # FastAPI WebSocket route and room manager
│   └── __init__.py
├── tests/
│   └── test_signaling.py # Pytest room lifecycle test suite
├── Dockerfile           # Production container build
├── requirements.txt     # Python dependencies
└── README.md            # This guide
```
