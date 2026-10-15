# Native Gamepad Injection Sidecar (`/sidecar`)

**Primary Lead:** Sam Hajdukiewicz (Host Input Engine & Architecture)  
**Collaborator:** Joshua Dobbs (QA, Multi-OS Testing & Input Calibration)  
**Tech Stack:** Python 3.10+, `websockets`, `vgamepad` (Windows), `evdev` (Linux), `pytest`

---

## Component Purpose
When a friend holds the virtual controller token, their button taps and analog stick movements travel over the network to the host's machine. 

Browsers cannot directly inject inputs into native Windows or Linux games due to operating system sandboxing. The `/sidecar` is a lightweight background micro-daemon that runs on the host machine. It listens on a local WebSocket (`ws://127.0.0.1:9876`), receives sanitized JSON gamepad frames from the Electron app, and injects them into the operating system using virtual gamepad drivers:
* **Windows (95% of PC Gamers):** Emulates an Xbox 360 controller via the **ViGEmBus** kernel driver using `vgamepad`.
* **Linux (Steam Deck / Team Developers):** Injects gamepad events into the kernel `/dev/uinput` subsystem using `python-evdev`.
* **Mock Daemon:** Provides a cross-platform simulator that prints controller frames to stdout for development on macOS and Linux workstations.

> **Reference Implementation Available:** A fully verified proof-of-concept of all three daemons is preserved on the `playground` branch (`git checkout playground`). Consult `sidecar/agent_win.py` and `sidecar/agent_linux.py` there for working driver code.

---

## Running the Sidecar

### 1. Development Mock (Mac / Linux / Windows)
Runs anywhere without drivers:
```bash
pip install websockets
python3 sidecar/agent_mock.py --port 9876
```

### 2. Windows Host (Physical Testing)
1. Install the official signed [ViGEmBus Driver](https://github.com/nefarius/ViGEmBus/releases).
2. Install Python packages:
   ```bash
   pip install websockets vgamepad
   ```
3. Run:
   ```bash
   python sidecar/agent_win.py --port 9876
   ```
4. Verify by opening Windows `joy.cpl` (Set up USB game controllers) to confirm the virtual Xbox 360 gamepad appears.

### 3. Linux Host (`/dev/uinput`)
1. Ensure write permissions to `/dev/uinput`:
   ```bash
   sudo usermod -aG input $USER
   ```
2. Install packages:
   ```bash
   pip install websockets evdev
   ```
3. Run:
   ```bash
   python3 sidecar/agent_linux.py --port 9876
   ```

---

## Running Automated Tests
```bash
pytest sidecar/tests -v
```

---

## Protocol & Deadzone Specification
See **[`sidecar/protocol.md`](file:///home/targed/Documents/Git/Pass-the-controller/sidecar/protocol.md)** for:
* JSON frame schemas (`gamepad_state`, `reset`, `ping`, `pong`).
* Button mapping constants (A, B, X, Y, LB, RB, LT, RT, DPad, Sticks).
* Axis ranges (`-1.0` to `+1.0`) and deadzone clamping (`[-0.08, +0.08]`).

---

## Directory Structure
```
sidecar/
├── agent_mock.py      # Cross-platform simulation daemon
├── agent_win.py       # Windows ViGEmBus / vgamepad daemon
├── agent_linux.py     # Linux /dev/uinput / evdev daemon
├── protocol.md        # Input packet schema specification
├── requirements.txt   # Dependencies
├── tests/
│   └── test_sidecar.py # Deadzone and parsing tests
└── README.md          # This guide
```
