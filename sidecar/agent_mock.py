#!/usr/bin/env python3
"""
Pass the Controller - Mock Gamepad Injection Sidecar
Cross-platform daemon for development and testing on macOS, Linux, and Windows.
Listens on local WebSocket (default ws://127.0.0.1:9876) and logs parsed controller frames.
"""

from __future__ import annotations
import argparse
import asyncio
import json
import logging
import sys

logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] [SIDE-MOCK] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("agent_mock")


class MockGamepadHandler:
    def __init__(self, deadzone: float = 0.08):
        self.deadzone = deadzone
        self.current_state = {
            "buttons": {},
            "triggers": {"LT": 0.0, "RT": 0.0},
            "axes": {"left_x": 0.0, "left_y": 0.0, "right_x": 0.0, "right_y": 0.0},
        }
        self.total_packets = 0

    def apply_deadzone(self, value: float) -> float:
        if abs(value) < self.deadzone:
            return 0.0
        return max(-1.0, min(1.0, value))

    def process_frame(self, data: dict) -> dict:
        self.total_packets += 1
        buttons = data.get("buttons", {})
        triggers = data.get("triggers", {})
        axes = data.get("axes", {})

        # Filter deadzones on axes
        filtered_axes = {
            axis: self.apply_deadzone(val) for axis, val in axes.items()
        }

        # Detect pressed buttons for cleaner logs
        pressed = [btn for btn, state in buttons.items() if state == 1]

        self.current_state = {
            "buttons": buttons,
            "triggers": triggers,
            "axes": filtered_axes,
        }

        return {
            "seq": data.get("seq", self.total_packets),
            "pressed": pressed,
            "triggers": triggers,
            "axes": filtered_axes,
        }

    def reset(self):
        self.current_state = {
            "buttons": {},
            "triggers": {"LT": 0.0, "RT": 0.0},
            "axes": {"left_x": 0.0, "left_y": 0.0, "right_x": 0.0, "right_y": 0.0},
        }
        logger.info("Virtual controller reset - all buttons and sticks zeroed.")


async def websocket_handler(websocket, handler: MockGamepadHandler):
    logger.info(f"Electron host connected: {websocket.remote_address}")
    try:
        async for message in websocket:
            try:
                payload = json.loads(message)
            except json.JSONDecodeError:
                continue

            msg_type = payload.get("type")

            if msg_type == "gamepad_state":
                frame = handler.process_frame(payload)
                if frame["pressed"] or any(v > 0.1 for v in frame["triggers"].values()):
                    logger.info(f"Frame #{frame['seq']}: Pressed={frame['pressed']} Triggers={frame['triggers']}")

            elif msg_type == "reset":
                handler.reset()
                await websocket.send(json.dumps({"type": "reset_ack"}))

            elif msg_type == "ping":
                await websocket.send(
                    json.dumps(
                        {
                            "type": "pong",
                            "driver": "mock",
                            "status": "ready",
                            "packets_received": handler.total_packets,
                        }
                    )
                )

    except Exception as e:
        logger.warning(f"Connection ended: {e}")
    finally:
        logger.info("Electron host disconnected")


async def main():
    parser = argparse.ArgumentParser(description="Pass-the-Controller Mock Gamepad Sidecar")
    parser.add_argument("--host", default="127.0.0.1", help="Host binding (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=9876, help="Port binding (default: 9876)")
    args = parser.parse_args()

    # Import websockets
    try:
        import websockets
    except ImportError:
        logger.error("The 'websockets' package is required. Install via: pip install websockets")
        sys.exit(1)

    handler = MockGamepadHandler()
    logger.info(f"Starting Mock Gamepad Sidecar on ws://{args.host}:{args.port}")
    logger.info("Ready to receive gamepad frames from Electron host.")

    async with websockets.serve(lambda ws: websocket_handler(ws, handler), args.host, args.port):
        await asyncio.Future()  # run forever


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Shutting down mock sidecar")
