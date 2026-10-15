# How-To: Developer Workflows

This guide provides concrete instructions for everyday engineering tasks: running tests, building release packages, testing native drivers on different operating systems, and debugging common issues.

---

## 1. Running Automated Tests

Always run tests before opening a pull request. We have automated checks for both Python services and the TypeScript client.

### Run All Tests at Once
From the project root:

```bash
# 1. Python signaling tests
PYTHONPATH=signaling pytest signaling/tests -v

# 2. Sidecar protocol and deadzone tests
pytest sidecar/tests -v

# 3. Client TypeScript check & Vite production build
cd client && npm run build && cd ..
```

### Running Specific Python Tests
To run a single test function with detailed print output:

```bash
PYTHONPATH=signaling pytest signaling/tests/test_signaling.py -k test_websocket_room_lifecycle -s
```

---

## 2. Packaging Distributable Installers

We use `electron-builder` to bundle the Electron app, its React frontend, and the sidecar executables into platform-specific installers.

### Build Installers Locally
Navigate to the `client/` folder:

```bash
cd client
npm run build     # Compiles Vite frontend into client/dist/
npm run package   # Calls electron-builder
```

Output files will appear in `client/release/`:
* **On Windows:**
  * `Pass the Controller-Windows-1.0.0.exe` (NSIS installer)
  * `Pass the Controller-Windows-1.0.0-portable.exe` (No install needed)
* **On Linux:**
  * `Pass the Controller-Linux-1.0.0.AppImage` (Universal binary)
  * `pass-the-controller_1.0.0_amd64.deb` (Debian/Ubuntu package)

> **Note:** Building a Windows `.exe` from a Linux machine requires Wine. It is usually easier to push to GitHub and let our GitHub Actions release workflow build the native Windows installer in the cloud.

---

## 3. Testing Native Host Drivers

The client detects whether it is running on Windows or Linux and selects the appropriate sidecar.

### Testing on Windows (ViGEmBus)
1. Download and run the official signed [ViGEmBus Driver Installer](https://github.com/nefarius/ViGEmBus/releases).
2. Install Python dependencies:
   ```bash
   pip install websockets vgamepad
   ```
3. Run the Windows daemon:
   ```bash
   python sidecar/agent_win.py --port 9876
   ```
4. Verify by opening Windows **Set up USB game controllers** (`joy.cpl`). When the daemon starts, a new **Xbox 360 Controller for Windows** device will appear in the list with `OK` status.

### Testing on Linux (`/dev/uinput`)
1. Ensure your user has permissions to write to `/dev/uinput`:
   ```bash
   sudo usermod -aG input $USER
   # Or create a udev rule:
   echo 'KERNEL=="uinput", MODE="0660", GROUP="input", OPTIONS+="static_node=uinput"' | sudo tee /etc/udev/rules.d/99-uinput.rules
   sudo udevadm control --reload-rules && sudo udevadm trigger
   ```
2. Install dependencies:
   ```bash
   pip install websockets evdev
   ```
3. Run the Linux daemon:
   ```bash
   python3 sidecar/agent_linux.py --port 9876
   ```
4. Verify by opening a gamepad utility like `jstest-gtk` or running:
   ```bash
   evtest /dev/input/by-id/...
   ```

---

## 4. Working on the UI (Lyla's Workflow)

The frontend is located in `client/src/renderer/`.

### Hot Reloading
When working on styling or components, use Vite's hot-reload server:

```bash
cd client
npm run dev
```

Open `http://localhost:5173` in any modern browser (Chrome, Edge, Brave, Firefox). Any changes made to CSS or React components update in the browser instantly without restarting Electron.

### Key Files for UI Development
* **Design System & CSS Variables:** `client/src/renderer/index.css`
  * Modify `--bg-primary`, `--accent-green`, `--accent-cyan`, and `.retro-panel` to adjust bevels, scanlines, and retro styling.
* **Component Layout:** `client/src/renderer/App.tsx`
* **Gamepad HUD:** `client/src/renderer/components/GamepadVisualizer.tsx`
* **Stream Viewer:** `client/src/renderer/components/ScreenStream.tsx`
* **Chat Window:** `client/src/renderer/components/ChatPanel.tsx`
* **Controller Queue:** `client/src/renderer/components/MemberQueueSidebar.tsx`

---

## 5. Managing the Local Infrastructure

Our local backend runs under Docker Compose.

### View Container Logs
```bash
docker compose -f infra/docker-compose.yml logs -f
# Or view only signaling logs:
docker compose -f infra/docker-compose.yml logs -f signaling
```

### Restart Containers
```bash
docker compose -f infra/docker-compose.yml restart
```

### Stop Containers
```bash
docker compose -f infra/docker-compose.yml down
```

### Rebuild Signaling Image After Modifying Python Code
If you edit `signaling/app/main.py` and want to update the Docker container:

```bash
docker compose -f infra/docker-compose.yml up -d --build signaling
```

---

## 6. Troubleshooting Common Pitfalls

### Issue: Port 8080 or Port 3478 is already in use
* **Symptom:** Docker fails with `bind: address already in use`.
* **Fix:** Check what process is occupying the port:
  ```bash
  sudo lsof -i :8080
  sudo lsof -i :3478
  ```
  Terminate the lingering process or adjust the host port mapping in `infra/docker-compose.yml`.

### Issue: Gamepad Not Detected in Browser
* **Symptom:** The gamepad visualizer says "No Gamepad Detected" even with a controller plugged in.
* **Cause:** The HTML5 Gamepad API does not expose gamepads to web pages until the user presses at least one button on the controller while the tab is focused (browser security requirement).
* **Fix:** Click inside the browser window and press the 'A' button or tap a bumper. The controller will immediately appear.

### Issue: Sidecar "Connection Refused"
* **Symptom:** Electron top bar shows `Input Daemon: offline`.
* **Cause:** The background Python sidecar is not running on port 9876.
* **Fix:** Start `python3 sidecar/agent_mock.py --port 9876` in an open terminal.
