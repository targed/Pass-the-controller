# Tutorial: Getting Started

This guide walks you through setting up your local development environment from scratch and testing the full controller-passing loop on a single machine.

By the end of this tutorial, you will have:
1. The local signaling and STUN/TURN servers running in Docker.
2. The Electron/React frontend running with hot reloading.
3. The mock controller sidecar receiving input packets.
4. Two instances communicating in the same room.

---

## Prerequisites

Make sure you have the following installed:
* **Node.js:** version 20 LTS or higher (`node -v`)
* **Python:** version 3.10 or higher (`python3 --version`)
* **Docker & Docker Compose:** (`docker compose version`)
* **A USB or Bluetooth gamepad:** (Optional, but useful for testing physical inputs. If you do not have one, the UI still allows testing queue and pass mechanics.)

---

## Step 1: Clone the Repository & Install Client Packages

Clone the repository and enter the workspace:

```bash
git clone https://github.com/targed/Pass-the-controller.git
cd Pass-the-controller
```

Install the client dependencies:

```bash
cd client
npm install
cd ..
```

---

## Step 2: Start Local Infrastructure

We use Docker Compose to run the signaling WebSocket server and the Coturn STUN/TURN server locally. This gives every developer identical backend behavior regardless of OS.

From the project root:

```bash
docker compose -f infra/docker-compose.yml up -d
```

Verify the containers are running:

```bash
docker compose -f infra/docker-compose.yml ps
```

You should see two healthy services:
* `ptc-signaling` on `http://localhost:8080` (WebSocket at `ws://localhost:8080`)
* `ptc-coturn` on port `3478` (UDP & TCP)

You can check the signaling server health check endpoint in your browser or curl:

```bash
curl http://localhost:8080/health
# Response: {"status":"healthy","service":"pass-the-controller-signaling","active_rooms":0,"total_users":0}
```

---

## Step 3: Start the Controller Sidecar

The host machine runs a small background daemon that receives gamepad packets and writes them to the operating system. For development on Linux or Mac (or on Windows without drivers installed), run the mock sidecar:

In a new terminal window:

```bash
# If you have not created a virtualenv, websockets can be installed locally or via pip:
pip install websockets

python3 sidecar/agent_mock.py --port 9876
```

You will see:
```text
[SIDE-MOCK] Starting Mock Gamepad Sidecar on ws://127.0.0.1:9876
[SIDE-MOCK] Ready to receive gamepad frames from Electron host.
```

Leave this terminal open.

---

## Step 4: Launch the Frontend

In another terminal window:

```bash
cd client
npm run dev
```

Vite will start the web application at `http://localhost:5173`.

You can also run it inside the Electron desktop shell:

```bash
cd client
npm start
```

---

## Step 5: Test the Multi-User Controller Passing Flow

To simulate a game session between two players without needing a second computer:

1. **Host Window (Player 1):**
   * Open `http://localhost:5173` (or the Electron app).
   * Enter your name: `Alice`
   * Keep the room code as `LOBBY`.
   * Click **Host Game**.
   * You will see the main 3-column room layout. The top status bar will show `Signaling: Online` and `Input Daemon: connected`.

2. **Guest Window (Player 2):**
   * Open a second browser window (e.g. Incognito or an alternate browser profile) to `http://localhost:5173`.
   * Enter your name: `Bob`
   * Set the room code to `LOBBY`.
   * Click **Join Room**.

3. **Verify Synchronization:**
   * Look at Alice's window: Bob will appear in the **IN ROOM** list on the right sidebar.
   * Send a chat message from Bob: Alice will see it instantly in the chat panel.

4. **Pass the Virtual Controller:**
   * In Bob's window, look at the right sidebar and click **Request Controller**.
   * Bob is added to the **PASS QUEUE (#1 Bob)**.
   * In Alice's window, Alice will see Bob waiting in the queue. Click the **Pass** button next to Bob's name.
   * Both screens update: `CURRENT HOLDER: Bob`.
   * Bob's status badge turns green: `INPUT ACTIVE (YOU CONTROL GAME)`.

5. **Test Controller Input Transmission:**
   * If Bob has a controller plugged in and presses any button (or moves a stick), observe the terminal running `sidecar/agent_mock.py`.
   * The mock daemon logs incoming frames in real time:
     ```text
     [SIDE-MOCK] Frame #42: Pressed=['A'] Triggers={'LT': 0.0, 'RT': 0.0}
     ```

6. **Host Reclaim:**
   * In Alice's window, click the red **Reclaim Controller** button.
   * The token immediately returns to Alice, revoking Bob's input permissions.

---

## Next Steps

Now that your local environment is working:
* Read **[How-To: Developer Workflows](file:///home/targed/Documents/Git/Pass-the-controller/documentation/how-to/developer-workflows.md)** to learn how to run test suites, rebuild the client, and build distributable binaries.
* Read **[Reference: Architecture & Protocols](file:///home/targed/Documents/Git/Pass-the-controller/documentation/reference/architecture-and-specs.md)** to see the exact message formats used between services.
