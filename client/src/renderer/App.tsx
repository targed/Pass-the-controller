import React, { useState } from 'react';
import { Gamepad2, Tv, MessageSquare, Users, Server, LogIn } from 'lucide-react';
import { ChannelSidebar } from './components/ChannelSidebar.tsx';
import { ScreenStream } from './components/ScreenStream.tsx';
import { ChatPanel } from './components/ChatPanel.tsx';
import { MemberQueueSidebar } from './components/MemberQueueSidebar.tsx';
import { GamepadVisualizer } from './components/GamepadVisualizer.tsx';
import { useGamepad } from './hooks/useGamepad.ts';
import { useSignaling } from './hooks/useSignaling.ts';

/**
 * Pass the Controller - Main Application Shell
 * -------------------------------------------------------------
 * Starter template for Lyla (Frontend & UI), Simon (Networking),
 * and Joshua (QA & Input).
 *
 * For the complete working implementation, see the `playground` branch:
 *   git checkout playground
 */
export default function App() {
  const [username, setUsername] = useState('Gamer_' + Math.floor(1000 + Math.random() * 9000));
  const [roomCode, setRoomCode] = useState('LOBBY');
  const [isHost, setIsHost] = useState(false);
  const [joined, setJoined] = useState(false);

  // TODO (Simon & Michael): Hook up room state and messaging via useSignaling
  const {
    connected,
    roomState,
    messages,
    sendChatMessage,
    requestController,
    passController,
    reclaimController,
  } = useSignaling('ws://localhost:8080', joined ? roomCode : '', 'user_1', username, isHost);

  // TODO (Joshua & Sam): Hook up physical gamepad polling via useGamepad
  const gamepadSnapshot = useGamepad();

  // Screen 1: Welcome / Join Screen
  if (!joined) {
    return (
      <div
        className="retro-scanlines"
        style={{
          width: '100vw',
          height: '100vh',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          background: 'linear-gradient(135deg, #090b10 0%, #151a26 100%)',
        }}
      >
        <div
          className="retro-panel"
          style={{
            width: '440px',
            padding: '24px',
            textAlign: 'center',
          }}
        >
          <div style={{ display: 'inline-flex', padding: '12px', background: '#1c2233', borderRadius: '50%', marginBottom: '12px' }}>
            <Gamepad2 size={36} color="var(--accent-green)" />
          </div>
          <h2 style={{ fontFamily: 'var(--font-mono)', fontSize: '20px', color: '#fff', marginBottom: '4px' }}>
            PASS THE CONTROLLER
          </h2>
          <p style={{ color: 'var(--text-muted)', fontSize: '12px', marginBottom: '20px' }}>
            Capstone Starter Template • Main Branch
          </p>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '12px', textAlign: 'left', marginBottom: '20px' }}>
            <div>
              <label style={{ fontSize: '11px', fontFamily: 'var(--font-mono)', color: 'var(--text-muted)' }}>USERNAME</label>
              <input
                type="text"
                value={username}
                onChange={(e) => setUsername(e.target.value)}
                className="retro-inset"
                style={{ width: '100%', padding: '8px 10px', color: '#fff', fontSize: '13px', marginTop: '4px', outline: 'none' }}
              />
            </div>

            <div>
              <label style={{ fontSize: '11px', fontFamily: 'var(--font-mono)', color: 'var(--text-muted)' }}>ROOM CODE</label>
              <input
                type="text"
                value={roomCode}
                onChange={(e) => setRoomCode(e.target.value.toUpperCase())}
                className="retro-inset"
                style={{ width: '100%', padding: '8px 10px', color: 'var(--accent-cyan)', fontSize: '13px', marginTop: '4px', outline: 'none', fontWeight: 700 }}
              />
            </div>
          </div>

          <div style={{ display: 'flex', gap: '10px' }}>
            <button
              className="retro-button accent"
              style={{ flex: 1, justifyContent: 'center', padding: '10px' }}
              onClick={() => {
                setIsHost(true);
                setJoined(true);
              }}
            >
              <Server size={14} /> Host Session
            </button>
            <button
              className="retro-button"
              style={{ flex: 1, justifyContent: 'center', padding: '10px' }}
              onClick={() => {
                setIsHost(false);
                setJoined(true);
              }}
            >
              <LogIn size={14} /> Join Session
            </button>
          </div>
        </div>
      </div>
    );
  }

  // Screen 2: Active Room Session Layout
  return (
    <div style={{ display: 'flex', flexDirection: 'column', height: '100vh', width: '100vw', background: 'var(--bg-primary)' }}>
      {/* Top Application Bar */}
      <div
        style={{
          height: '36px',
          background: '#0a0d13',
          borderBottom: '1px solid var(--panel-border)',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          padding: '0 12px',
          fontSize: '11px',
          fontFamily: 'var(--font-mono)',
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Gamepad2 size={16} color="var(--accent-green)" />
          <span style={{ fontWeight: 700, color: '#fff' }}>PASS THE CONTROLLER</span>
          <span style={{ color: 'var(--text-muted)' }}>|</span>
          <span style={{ color: 'var(--accent-cyan)' }}>ROOM: {roomCode}</span>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
            <span className={`led-lamp ${connected ? 'active-green' : 'active-amber'}`} />
            <span style={{ color: 'var(--text-muted)' }}>Signaling: {connected ? 'Online' : 'Disconnected'}</span>
          </div>
          <button className="retro-button" style={{ padding: '2px 8px', fontSize: '10px' }} onClick={() => setJoined(false)}>
            Leave Room
          </button>
        </div>
      </div>

      {/* Main 3-Column Layout */}
      <div style={{ flex: 1, display: 'flex', overflow: 'hidden' }}>
        {/* Left: Channels Sidebar */}
        <ChannelSidebar username={username} roomCode={roomCode} isHost={isHost} connected={connected} />

        {/* Center: Stream Viewport + Gamepad Visualizer HUD */}
        <div style={{ flex: 1, display: 'flex', flexDirection: 'column', padding: '8px', gap: '8px', minWidth: 0 }}>
          <ScreenStream isHost={isHost} controllerHolderName={username} isTokenHolder={true} />
          <GamepadVisualizer snapshot={gamepadSnapshot} isTokenHolder={true} />
        </div>

        {/* Right-Center: Chat Feed */}
        <div style={{ padding: '8px 0 8px 0' }}>
          <ChatPanel messages={messages} onSendMessage={sendChatMessage} currentUsername={username} />
        </div>

        {/* Rightmost: Member List & Controller Token Queue */}
        <div style={{ padding: '8px' }}>
          <MemberQueueSidebar
            members={roomState.members}
            controllerHolderId={roomState.controllerHolder}
            controllerQueue={roomState.controllerQueue}
            currentClientId={'user_1'}
            isHost={isHost}
            onRequestController={requestController}
            onPassController={passController}
            onReclaimController={reclaimController}
          />
        </div>
      </div>
    </div>
  );
}
