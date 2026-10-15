# Desktop Client (`/client`)

**Primary Lead:** Lyla Graham (Frontend & UI/UX)  
**Collaborators:** Simon Leeker (WebRTC Media & Peer Connections), Joshua Dobbs (QA & Input Testing)  
**Tech Stack:** Electron, React 19, TypeScript, Vite

---

## Component Purpose
The `/client` directory contains the cross-platform desktop application. It serves as both the **spectator view** (where friends watch the game, chat, and send controller inputs) and the **host manager** (where the game host selects their screen and manages the virtual controller token).

> **Reference Implementation Available:** A fully verified proof-of-concept of this client is preserved on the `playground` branch (`git checkout playground`). Consult it for working code examples and integration patterns.

---

## Getting Started
```bash
cd client
npm install
npm run dev      # Starts Vite with hot reload at http://localhost:5173
npm start        # Launches inside the Electron desktop shell
npm run build    # Compiles TypeScript and builds production bundle
npm run package  # Builds installers via electron-builder
```

---

## Lyla's UI/UX Roadmap (Mid-2000s Retro Cyber Aesthetic)
The goal is a distinct retro desktop aesthetic reminiscent of WinAMP, MSN Messenger, and early AIM, while maintaining modern usability:
1. **Design Tokens (`src/renderer/index.css`):**
   * Pre-configured CSS variables: dark steel slate (`#0f1117`), glowing neon accents (`--accent-green: #00ff88`, `--accent-cyan: #00e5ff`), and scanline overlays.
   * Beveled panels (`.retro-panel`, `.retro-inset`), retro buttons, and LED status lamps.
2. **Component Layout (`src/renderer/App.tsx`):**
   * **Top Bar:** App title, current room code, connection status lamps.
   * **Left Sidebar (`components/ChannelSidebar.tsx`):** Text channels (`#general`, `#gaming`) and voice rooms.
   * **Center Stage (`components/ScreenStream.tsx`):** Low-latency game video display with stream HUD badges (FPS, latency) and host window selector.
   * **Controller HUD (`components/GamepadVisualizer.tsx`):** Real-time interactive gamepad visualization showing button presses and stick movements.
   * **Right Sidebars:**
     * `components/ChatPanel.tsx`: Live chat feed with timestamps and avatars.
     * `components/MemberQueueSidebar.tsx`: Active room members and the **Pass Queue** with "Request Controller", "Pass", and "Reclaim" buttons.

---

## Gamepad & Input Integration (Joshua & Sam)
* **Hook:** `src/renderer/hooks/useGamepad.ts` polls `navigator.getGamepads()` at 60 Hz via `requestAnimationFrame`.
* When a player holds the controller token, their gamepad snapshot is serialized and sent to the host over the WebRTC DataChannel (or local WebSocket).

---

## WebRTC & Signaling Integration (Simon & Michael)
* **Hook:** `src/renderer/hooks/useSignaling.ts` connects to the Python signaling server (`ws://localhost:8080/ws/{roomCode}/{clientId}`).
* Manages room state synchronization, chat messages, and token arbitration events.

---

## Directory Structure
```
client/
├── src/
│   ├── main/          # Electron Main Process (desktopCapturer & sidecar spawner)
│   ├── preload/       # Secure ContextBridge APIs (window.passTheController)
│   └── renderer/      # React 19 Web Application
│       ├── components/ # Modular UI components (Stream, Chat, Queue, Visualizer)
│       ├── hooks/      # useGamepad & useSignaling hooks
│       ├── App.tsx     # Main application layout
│       └── index.css   # Retro design system and tokens
├── electron-builder.yml # Multi-platform packaging configuration
├── package.json
└── vite.config.ts
```
