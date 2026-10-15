"""
Pass the Controller - Signaling & Room Coordination Service
------------------------------------------------------------
Primary Maintainers: Michael Allen (Backend) & Simon Leeker (Networking)

Starter template on `main`. For the complete reference implementation,
see the `playground` branch:
  git checkout playground
"""

from __future__ import annotations
import json
import logging
import time
from typing import Dict, List, Optional
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("signaling")

app = FastAPI(title="Pass the Controller - Signaling Service", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class RoomSession:
    """Represents an active gaming session identified by a room code."""

    def __init__(self, room_code: str, host_id: str):
        self.room_code = room_code
        self.host_id = host_id
        self.active_connections: Dict[str, WebSocket] = {}
        self.members: Dict[str, dict] = {}
        self.controller_holder: str = host_id
        self.controller_queue: List[str] = []
        self.created_at = time.time()

    def add_member(self, client_id: str, username: str, ws: WebSocket, is_host: bool = False):
        self.active_connections[client_id] = ws
        self.members[client_id] = {
            "client_id": client_id,
            "username": username,
            "is_host": is_host,
            "joined_at": time.time(),
        }

    def remove_member(self, client_id: str):
        self.active_connections.pop(client_id, None)
        self.members.pop(client_id, None)
        if client_id in self.controller_queue:
            self.controller_queue.remove(client_id)
        # Fallback: if token holder disconnects, return control to host
        if self.controller_holder == client_id:
            self.controller_holder = self.host_id

    async def broadcast(self, message: dict, exclude: Optional[str] = None):
        payload = json.dumps(message)
        dead: List[str] = []
        for cid, ws in self.active_connections.items():
            if exclude and cid == exclude:
                continue
            try:
                await ws.send_text(payload)
            except Exception as e:
                logger.warning(f"Error sending to {cid}: {e}")
                dead.append(cid)
        for cid in dead:
            self.remove_member(cid)

    async def send_to(self, target_id: str, message: dict):
        ws = self.active_connections.get(target_id)
        if ws:
            try:
                await ws.send_text(json.dumps(message))
            except Exception as e:
                logger.warning(f"Error sending to {target_id}: {e}")
                self.remove_member(target_id)


# Global registry of active rooms
rooms: Dict[str, RoomSession] = {}


@app.get("/health")
async def health_check():
    return {
        "status": "healthy",
        "service": "pass-the-controller-signaling",
        "active_rooms": len(rooms),
        "total_users": sum(len(r.active_connections) for r in rooms.values()),
    }


@app.get("/rooms/{room_code}")
async def get_room(room_code: str):
    room = rooms.get(room_code.upper())
    if not room:
        return {"exists": False}
    return {
        "exists": True,
        "room_code": room.room_code,
        "host_id": room.host_id,
        "members_count": len(room.members),
        "controller_holder": room.controller_holder,
        "controller_queue": room.controller_queue,
    }


@app.websocket("/ws/{room_code}/{client_id}")
async def websocket_endpoint(
    websocket: WebSocket,
    room_code: str,
    client_id: str,
    username: str = "Player",
    is_host: bool = False,
):
    await websocket.accept()
    room_code = room_code.upper()

    # Room creation or joining
    if room_code not in rooms:
        rooms[room_code] = RoomSession(room_code=room_code, host_id=client_id)
        is_host = True
        logger.info(f"Room {room_code} created by {client_id} ({username})")

    room = rooms[room_code]
    room.add_member(client_id, username, websocket, is_host=is_host)

    # Initial room state broadcast
    await room.broadcast(
        {
            "type": "room_state",
            "room_code": room_code,
            "host_id": room.host_id,
            "controller_holder": room.controller_holder,
            "controller_queue": room.controller_queue,
            "members": list(room.members.values()),
            "event": f"{username} joined the room",
        }
    )

    try:
        while True:
            raw_data = await websocket.receive_text()
            try:
                data = json.loads(raw_data)
            except json.JSONDecodeError:
                continue

            msg_type = data.get("type")

            # -----------------------------------------------------------------
            # 1. WebRTC Signaling Relay (SDP Offer/Answer & ICE Candidates)
            # -----------------------------------------------------------------
            if msg_type in ("webrtc_offer", "webrtc_answer", "ice_candidate"):
                target_id = data.get("target_id")
                data["sender_id"] = client_id
                if target_id:
                    await room.send_to(target_id, data)
                else:
                    await room.broadcast(data, exclude=client_id)

            # -----------------------------------------------------------------
            # 2. Text Chat Message
            # -----------------------------------------------------------------
            elif msg_type == "chat_message":
                text = data.get("text", "").strip()
                if text:
                    await room.broadcast(
                        {
                            "type": "chat_message",
                            "sender_id": client_id,
                            "username": username,
                            "text": text,
                            "timestamp": time.time(),
                        }
                    )

            # -----------------------------------------------------------------
            # 3. Controller Token Queue & Handoff
            # -----------------------------------------------------------------
            elif msg_type == "request_controller":
                if client_id not in room.controller_queue and client_id != room.controller_holder:
                    room.controller_queue.append(client_id)
                    await room.broadcast(
                        {
                            "type": "controller_queue_updated",
                            "controller_holder": room.controller_holder,
                            "controller_queue": room.controller_queue,
                            "requested_by": username,
                        }
                    )

            elif msg_type == "pass_controller":
                target_holder = data.get("target_id")
                if client_id in (room.host_id, room.controller_holder):
                    if target_holder and target_holder in room.members:
                        room.controller_holder = target_holder
                        if target_holder in room.controller_queue:
                            room.controller_queue.remove(target_holder)
                        await room.broadcast(
                            {
                                "type": "controller_passed",
                                "controller_holder": room.controller_holder,
                                "controller_queue": room.controller_queue,
                                "holder_name": room.members[target_holder]["username"],
                            }
                        )

            elif msg_type == "reclaim_controller":
                if client_id == room.host_id:
                    room.controller_holder = room.host_id
                    await room.broadcast(
                        {
                            "type": "controller_reclaimed",
                            "controller_holder": room.host_id,
                            "controller_queue": room.controller_queue,
                            "message": "Host reclaimed virtual controller",
                        }
                    )

    except WebSocketDisconnect:
        logger.info(f"Client {client_id} disconnected from room {room_code}")
    finally:
        room.remove_member(client_id)
        if len(room.active_connections) == 0:
            logger.info(f"Room {room_code} empty; tearing down")
            rooms.pop(room_code, None)
        else:
            await room.broadcast(
                {
                    "type": "room_state",
                    "room_code": room_code,
                    "host_id": room.host_id,
                    "controller_holder": room.controller_holder,
                    "controller_queue": room.controller_queue,
                    "members": list(room.members.values()),
                    "event": f"{username} left the room",
                }
            )
