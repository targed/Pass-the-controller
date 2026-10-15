#!/usr/bin/env python3
"""
Pass the Controller - Linux Native Gamepad Injection Daemon
Emulates a gamepad using the Linux Kernel uinput subsystem via python-evdev.
Listens on local WebSocket (ws://127.0.0.1:9876) and injects events into /dev/uinput.
"""

from __future__ import annotations
import argparse
import asyncio
import json
import logging
import sys

logging.basicConfig(
    level=logging.INFO,
    format="[%(asctime)s] [SIDE-LINUX] %(message)s",
    datefmt="%H:%M:%S",
)
logger = logging.getLogger("agent_linux")

try:
    import evdev
    from evdev import UInput, ecodes as e
except ImportError:
    evdev = None
    e = None

# Button mapping from protocol strings to evdev ecodes
BUTTON_MAP = {}
if e:
    BUTTON_MAP = {
        "A": e.BTN_A,
        "B": e.BTN_B,
        "X": e.BTN_X,
        "Y": e.BTN_Y,
        "LB": e.BTN_TL,
        "RB": e.BTN_TR,
        "BACK": e.BTN_SELECT,
        "START": e.BTN_START,
        "GUIDE": e.BTN_MODE,
        "LEFT_THUMB": e.BTN_THUMBL,
        "RIGHT_THUMB": e.BTN_THUMBR,
    }


class LinuxGamepadHandler:
    def __init__(self, deadzone: float = 0.08):
        self.deadzone = deadzone
        if not evdev:
            raise RuntimeError("python-evdev is not installed. Run: pip install evdev")
        logger.info("Initializing virtual gamepad via /dev/uinput...")

        # Setup capabilities
        capabilities = {
            e.EV_KEY: list(BUTTON_MAP.values()) + [e.BTN_DPAD_UP, e.BTN_DPAD_DOWN, e.BTN_DPAD_LEFT, e.BTN_DPAD_RIGHT],
            e.EV_ABS: [
                (e.ABS_X, evdev.AbsInfo(value=0, min=-32768, max=32767, fuzz=0, flat=0, resolution=0)),
                (e.ABS_Y, evdev.AbsInfo(value=0, min=-32768, max=32767, fuzz=0, flat=0, resolution=0)),
                (e.ABS_RX, evdev.AbsInfo(value=0, min=-32768, max=32767, fuzz=0, flat=0, resolution=0)),
                (e.ABS_RY, evdev.AbsInfo(value=0, min=-32768, max=32767, fuzz=0, flat=0, resolution=0)),
                (e.ABS_Z, evdev.AbsInfo(value=0, min=0, max=255, fuzz=0, flat=0, resolution=0)),
                (e.ABS_RZ, evdev.AbsInfo(value=0, min=0, max=255, fuzz=0, flat=0, resolution=0)),
            ],
        }
        self.ui = UInput(capabilities, name="Pass-The-Controller Virtual Gamepad", version=0x3)
        logger.info("Virtual Linux gamepad attached successfully!")

    def apply_deadzone(self, value: float) -> float:
        if abs(value) < self.deadzone:
            return 0.0
        return max(-1.0, min(1.0, value))

    def process_frame(self, data: dict):
        buttons = data.get("buttons", {})
        triggers = data.get("triggers", {})
        axes = data.get("axes", {})

        # 1. Buttons
        for btn_name, state in buttons.items():
            code = BUTTON_MAP.get(btn_name)
            if code:
                self.ui.write(e.EV_KEY, code, 1 if state == 1 else 0)

        # 2. DPAD
        dpad_up = buttons.get("DPAD_UP", 0)
        dpad_down = buttons.get("DPAD_DOWN", 0)
        dpad_left = buttons.get("DPAD_LEFT", 0)
        dpad_right = buttons.get("DPAD_RIGHT", 0)

        self.ui.write(e.EV_KEY, e.BTN_DPAD_UP, dpad_up)
        self.ui.write(e.EV_KEY, e.BTN_DPAD_DOWN, dpad_down)
        self.ui.write(e.EV_KEY, e.BTN_DPAD_LEFT, dpad_left)
        self.ui.write(e.EV_KEY, e.BTN_DPAD_RIGHT, dpad_right)

        # 3. Triggers (0.0 - 1.0 -> 0 - 255)
        lt = int(triggers.get("LT", 0.0) * 255)
        rt = int(triggers.get("RT", 0.0) * 255)
        self.ui.write(e.EV_ABS, e.ABS_Z, lt)
        self.ui.write(e.EV_ABS, e.ABS_RZ, rt)

        # 4. Axes (-1.0 - 1.0 -> -32768 - 32767)
        lx = int(self.apply_deadzone(axes.get("left_x", 0.0)) * 32767)
        ly = int(self.apply_deadzone(axes.get("left_y", 0.0)) * -32767)  # Invert Y for Linux
        rx = int(self.apply_deadzone(axes.get("right_x", 0.0)) * 32767)
        ry = int(self.apply_deadzone(axes.get("right_y", 0.0)) * -32767)

        self.ui.write(e.EV_ABS, e.ABS_X, lx)
        self.ui.write(e.EV_ABS, e.ABS_Y, ly)
        self.ui.write(e.EV_ABS, e.ABS_RX, rx)
        self.ui.write(e.EV_ABS, e.ABS_RY, ry)

        # Commit syn event
        self.ui.syn()

    def reset(self):
        # Zero all axes and release buttons
        for code in BUTTON_MAP.values():
            self.ui.write(e.EV_KEY, code, 0)
        self.ui.write(e.EV_ABS, e.ABS_X, 0)
        self.ui.write(e.EV_ABS, e.ABS_Y, 0)
        self.ui.write(e.EV_ABS, e.ABS_RX, 0)
        self.ui.write(e.EV_ABS, e.ABS_RY, 0)
        self.ui.write(e.EV_ABS, e.ABS_Z, 0)
        self.ui.write(e.EV_ABS, e.ABS_RZ, 0)
        self.ui.syn()
        logger.info("Virtual Linux gamepad reset")


async def websocket_handler(websocket, handler: LinuxGamepadHandler):
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
                    json.dumps({"type": "pong", "driver": "linux_uinput", "status": "active"})
                )
    except Exception as e:
        logger.warning(f"Connection ended: {e}")
    finally:
        handler.reset()
        logger.info("Host client disconnected; gamepad reset")


async def main():
    parser = argparse.ArgumentParser(description="Pass-the-Controller Linux Gamepad Sidecar")
    parser.add_argument("--host", default="127.0.0.1", help="Host binding")
    parser.add_argument("--port", type=int, default=9876, help="Port binding")
    args = parser.parse_args()

    import websockets

    handler = LinuxGamepadHandler()
    logger.info(f"Linux Gamepad Daemon listening on ws://{args.host}:{args.port}")

    async with websockets.serve(lambda ws: websocket_handler(ws, handler), args.host, args.port):
        await asyncio.Future()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Shutting down Linux gamepad daemon")
