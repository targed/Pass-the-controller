"""
Pass the Controller - Signaling & Room Management Server
Provides WebSocket endpoints for WebRTC signaling (SDP/ICE) and
manages virtual controller token arbitration and room chat.
"""

from __future__ import annotations
import json
import logging
import time
from typing import Dict, List, Optional, Set
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

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
    def __init__(self, room_code: str, host_id: str):
        self.room_code = room_code
        self.host_id = host_id
        # client_id -> WebSocket
        self.active_connections: Dict[str, WebSocket] = {}
        # client_id -> metadata (username, is_host)
        self.members: Dict[str, dict] = {}
        # Client ID who currently has input permissions
        self.controller_holder: str = host_id
        # Queue of client IDs waiting for the controller
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
        # If the person who disconnected was holding the controller, return it to host
        if self.controller_holder == client_id:
            self.controller_holder = self.host_id

    async def broadcast(self, message: dict, exclude: Optional[str] = None):
        payload = json.dumps(message)
        dead_connections: List[str] = []
        for cid, ws in self.active_connections.items():
            if exclude and cid == exclude:
                continue
            try:
                await ws.send_text(payload)
            except Exception as e:
                logger.warning(f"Error broadcasting to {cid}: {e}")
                dead_connections.append(cid)
        for cid in dead_connections:
            self.remove_member(cid)

    async def send_to(self, target_id: str, message: dict):
        ws = self.active_connections.get(target_id)
        if ws:
            try:
                await ws.send_text(json.dumps(message))
            except Exception as e:
                logger.warning(f"Error sending message to {target_id}: {e}")
                self.remove_member(target_id)


# In-memory registry of active rooms (room_code -> RoomSession)
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

    # Room creation or joining logic
    if room_code not in rooms:
        # First one into the room becomes the room creator / host
        rooms[room_code] = RoomSession(room_code=room_code, host_id=client_id)
        is_host = True
        logger.info(f"Room {room_code} created by {client_id} ({username})")

    room = rooms[room_code]
    room.add_member(client_id, username, websocket, is_host=is_host)

    # Notify everyone in the room about the new member and current controller state
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

            # 1. WebRTC Signaling (SDP Offer/Answer & ICE Candidates)
            if msg_type in ("webrtc_offer", "webrtc_answer", "ice_candidate"):
                target_id = data.get("target_id")
                data["sender_id"] = client_id
                if target_id:
                    await room.send_to(target_id, data)
                else:
                    # Broadcast to everyone else if no explicit target (e.g. host announces stream)
                    await room.broadcast(data, exclude=client_id)

            # 2. Text Chat Message
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

            # 3. Request Controller Token
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

            # 4. Pass Controller (Host action or voluntary handoff)
            elif msg_type == "pass_controller":
                target_holder = data.get("target_id")
                # Only current host or current holder can pass
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

            # 5. Reclaim Controller (Host emergency override button)
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
            logger.info(f"Room {room_code} is empty, tearing down")
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
