#!/usr/bin/env python3
"""
Pass the Controller - Windows Native Gamepad Injection Daemon
Emulates a physical Xbox 360 controller using the ViGEmBus driver and vgamepad.
Listens on local WebSocket (ws://127.0.0.1:9876) and injects inputs directly into Windows.
"""

from __future__ import annotations
import argparse
import asyncio
import json
import logging
import sys

logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] [SIDE-WIN] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("agent_win")

try:
    import vgamepad as vg
except ImportError:
    vg = None

# Button mapping from protocol strings to vgamepad constants
BUTTON_MAP = {}
if vg:
    BUTTON_MAP = {
        "A": vg.XUSB_BUTTON.XUSB_GAMEPAD_A,
        "B": vg.XUSB_BUTTON.XUSB_GAMEPAD_B,
        "X": vg.XUSB_BUTTON.XUSB_GAMEPAD_X,
        "Y": vg.XUSB_BUTTON.XUSB_GAMEPAD_Y,
        "LB": vg.XUSB_BUTTON.XUSB_GAMEPAD_LEFT_SHOULDER,
        "RB": vg.XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_SHOULDER,
        "BACK": vg.XUSB_BUTTON.XUSB_GAMEPAD_BACK,
        "START": vg.XUSB_BUTTON.XUSB_GAMEPAD_START,
        "LEFT_THUMB": vg.XUSB_BUTTON.XUSB_GAMEPAD_LEFT_THUMB,
        "RIGHT_THUMB": vg.XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_THUMB,
        "DPAD_UP": vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_UP,
        "DPAD_DOWN": vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_DOWN,
        "DPAD_LEFT": vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_LEFT,
        "DPAD_RIGHT": vg.XUSB_BUTTON.XUSB_GAMEPAD_DPAD_RIGHT,
    }


class WindowsGamepadHandler:
    def __init__(self, deadzone: float = 0.08):
        self.deadzone = deadzone
        if not vg:
            raise RuntimeError(
                "vgamepad is not installed or ViGEmBus driver is missing. "
                "Run: pip install vgamepad, and ensure ViGEmBus driver is installed."
            )
        logger.info("Initializing virtual Xbox 360 gamepad via ViGEmBus...")
        self.gamepad = vg.VX360Gamepad()
        logger.info("Virtual Xbox 360 gamepad attached successfully!")

    def apply_deadzone(self, value: float) -> float:
        if abs(value) < self.deadzone:
            return 0.0
        return max(-1.0, min(1.0, value))

    def process_frame(self, data: dict):
        buttons = data.get("buttons", {})
        triggers = data.get("triggers", {})
        axes = data.get("axes", {})

        # 1. Update Buttons
        for btn_name, state in buttons.items():
            mapped_btn = BUTTON_MAP.get(btn_name)
            if mapped_btn:
                if state == 1:
                    self.gamepad.press_button(mapped_btn)
                else:
                    self.gamepad.release_button(mapped_btn)

        # 2. Update Triggers (0.0 to 1.0)
        lt = triggers.get("LT", 0.0)
        rt = triggers.get("RT", 0.0)
        self.gamepad.left_trigger_float(value_float=float(lt))
        self.gamepad.right_trigger_float(value_float=float(rt))

        # 3. Update Joysticks (-1.0 to 1.0)
        lx = self.apply_deadzone(axes.get("left_x", 0.0))
        ly = self.apply_deadzone(axes.get("left_y", 0.0))
        rx = self.apply_deadzone(axes.get("right_x", 0.0))
        ry = self.apply_deadzone(axes.get("right_y", 0.0))

        self.gamepad.left_joystick_float(x_value_float=float(lx), y_value_float=float(ly))
        self.gamepad.right_joystick_float(x_value_float=float(rx), y_value_float=float(ry))

        # Push state to OS
        self.gamepad.update()

    def reset(self):
        self.gamepad.reset()
        self.gamepad.update()
        logger.info("Virtual Xbox 360 gamepad state reset to neutral")


async def websocket_handler(websocket, handler: WindowsGamepadHandler):
    logger.info(f"Host client connected: {websocket.remote_address}")
    try:
        async for message in websocket:
            try:
                payload = json.loads(message)
            except json.JSONDecodeError:
                continue

            msg_type = payload.get("type")
            if msg_type == "gamepad_state":
                handler.process_frame(payload)
            elif msg_type == "reset":
                handler.reset()
                await websocket.send(json.dumps({"type": "reset_ack"}))
            elif msg_type == "ping":
                await websocket.send(
                    json.dumps({"type": "pong", "driver": "vgamepad_xinput", "status": "active"})
                )
    except Exception as e:
        logger.warning(f"Connection ended: {e}")
    finally:
        handler.reset()
        logger.info("Host client disconnected; gamepad reset")


async def main():
    parser = argparse.ArgumentParser(description="Pass-the-Controller Windows Gamepad Sidecar")
    parser.add_argument("--host", default="127.0.0.1", help="Host binding")
    parser.add_argument("--port", type=int, default=9876, help="Port binding")
    args = parser.parse_args()

    import websockets

    handler = WindowsGamepadHandler()
    logger.info(f"Windows Gamepad Daemon listening on ws://{args.host}:{args.port}")

    async with websockets.serve(lambda ws: websocket_handler(ws, handler), args.host, args.port):
        await asyncio.Future()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Shutting down Windows gamepad daemon")
