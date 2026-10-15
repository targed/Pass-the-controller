# Pass-the-Controller: Host Gamepad Input Protocol Specification
**Version:** 1.0.0  
**Transport:** Local WebSocket (`ws://127.0.0.1:9876`) or Standard Input JSON-Lines (`stdin`)  
**Target Emulation:** Standard Xbox 360 / XInput Gamepad

---

## 1. Overview
When a guest holds the controller token, their local gamepad inputs are read at 60 Hz by the Electron client using the HTML5 Gamepad API. These inputs are serialized into JSON packets and transmitted over the low-latency WebRTC `RTCDataChannel` to the host's Electron application.

The host Electron app immediately forwards these packets to the **Host Agent Sidecar** via local WebSocket or stdin. The sidecar translates the payload into native kernel driver calls:
* **Windows:** ViGEmBus via `vgamepad.VX360Gamepad`
* **Linux:** Linux Kernel Subsystem via `/dev/uinput` (`python-evdev`)

---

## 2. Gamepad State JSON Schema

```json
{
  "type": "gamepad_state",
  "seq": 1042,
  "timestamp": 1728941000.123,
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
    "RT": 0.85
  },
  "axes": {
    "left_x": 0.05,
    "left_y": -0.92,
    "right_x": 0.0,
    "right_y": 0.0
  }
}
```

### Value Ranges
* **Buttons:** `0` (released) or `1` (pressed).
* **Triggers (`LT`, `RT`):** Float from `0.0` (fully released) to `1.0` (fully pressed).
* **Thumbsticks (`left_x`, `left_y`, `right_x`, `right_y`):**
  * Float from `-1.0` to `+1.0`.
  * Horizontal (`_x`): `-1.0` is full left, `+1.0` is full right.
  * Vertical (`_y`): Standard XInput convention (`-1.0` down, `+1.0` up).
  * Recommended deadzone threshold: `0.08`.

---

## 3. Control Commands

In addition to continuous `gamepad_state` frames, the sidecar accepts lifecycle control messages:

### Reset / Release All
Sent when the guest loses the controller token or disconnects:
```json
{
  "type": "reset"
}
```

### Ping / Health Check
```json
{
  "type": "ping"
}
```
Response:
```json
{
  "type": "pong",
  "driver": "vgamepad" // or "uinput" or "mock"
}
```
