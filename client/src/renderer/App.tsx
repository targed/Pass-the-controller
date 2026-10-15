import React, { useState, useEffect, useRef } from 'react';
import { Gamepad2, Tv, Users, LogIn, Server, ShieldCheck, Play } from 'lucide-react';
import { useGamepad, GamepadSnapshot } from './hooks/useGamepad.ts';
import { useSignaling } from './hooks/useSignaling.ts';
import { ChannelSidebar } from './components/ChannelSidebar.tsx';
import { ScreenStream } from './components/ScreenStream.tsx';
import { ChatPanel } from './components/ChatPanel.tsx';
import { MemberQueueSidebar } from './components/MemberQueueSidebar.tsx';
import { GamepadVisualizer } from './components/GamepadVisualizer.tsx';

export default function App() {
  const [username, setUsername] = useState('Gamer_' + Math.floor(1000 + Math.random() * 9000));
  const [clientId] = useState('client_' + Math.random().toString(36).substring(2, 9));
  const [roomCode, setRoomCode] = useState('LOBBY');
  const [isHost, setIsHost] = useState(false);
  const [joined, setJoined] = useState(false);
  const [sidecarStatus, setSidecarStatus] = useState<string>('idle');

  // Signaling Hook
  const {
    connected,
    roomState,
    messages,
    sendChatMessage,
    requestController,
    passController,
    reclaimController,
  } = useSignaling(
    'ws://localhost:8080',
    joined ? roomCode : '',
    clientId,
    username,
    isHost
  );

  const isTokenHolder = roomState.controllerHolder === clientId;
  const currentHolderMember = roomState.members.find((m) => m.client_id === roomState.controllerHolder);

  // Local sidecar connection for Host to inject inputs
  const sidecarSocketRef = useRef<WebSocket | null>(null);

  // Host launches sidecar daemon when hosting
  const handleStartHosting = async () => {
    setIsHost(true);
    setJoined(true);
    if (window.passTheController?.startSidecar) {
      setSidecarStatus('starting');
      const res = await window.passTheController.startSidecar(9876);
      setSidecarStatus(res.status);
    }
  };

  // Connect local WebSocket to sidecar on localhost:9876 if host
  useEffect(() => {
    if (isHost && joined) {
      const ws = new WebSocket('ws://127.0.0.1:9876');
      ws.onopen = () => {
        console.log('[Sidecar Bridge] Connected to local input daemon');
        setSidecarStatus('connected');
      };
      ws.onerror = () => {
        setSidecarStatus('offline');
      };
      sidecarSocketRef.current = ws;
      return () => {
        ws.close();
      };
    }
  }, [isHost, joined]);

  // Read gamepad and forward to sidecar if holding controller
  const handleGamepadFrame = (snapshot: GamepadSnapshot) => {
    if (isTokenHolder && sidecarSocketRef.current?.readyState === WebSocket.OPEN) {
      sidecarSocketRef.current.send(
        JSON.stringify({
          type: 'gamepad_state',
          buttons: snapshot.buttons,
          triggers: snapshot.triggers,
          axes: snapshot.axes,
        })
      );
    }
  };

  const gamepadSnapshot = useGamepad(handleGamepadFrame);

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
            boxShadow: '0 8px 32px rgba(0,0,0,0.6)',
          }}
        >
          <div style={{ display: 'inline-flex', padding: '12px', background: '#1c2233', borderRadius: '50%', marginBottom: '12px' }}>
            <Gamepad2 size={36} color="var(--accent-green)" />
          </div>
          <h2 style={{ fontFamily: 'var(--font-mono)', fontSize: '20px', letterSpacing: '1px', color: '#fff', marginBottom: '4px' }}>
            PASS THE CONTROLLER
          </h2>
          <p style={{ color: 'var(--text-muted)', fontSize: '12px', marginBottom: '20px' }}>
            Social Game Streaming & Virtual Controller Sharing
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
              onClick={handleStartHosting}
            >
              <Server size={14} /> Host Game
            </button>
            <button
              className="retro-button"
              style={{ flex: 1, justifyContent: 'center', padding: '10px' }}
              onClick={() => {
                setIsHost(false);
                setJoined(true);
              }}
            >
              <LogIn size={14} /> Join Room
            </button>
          </div>
        </div>
      </div>
    );
  }

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
          <span style={{ fontWeight: 700, color: '#fff' }}>PASS THE CONTROLLER v1.0</span>
          <span style={{ color: 'var(--text-muted)' }}>|</span>
          <span style={{ color: 'var(--accent-cyan)' }}>ROOM: {roomCode}</span>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          {isHost && (
            <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
              <span className={`led-lamp ${sidecarStatus === 'connected' ? 'active-green' : 'active-amber'}`} />
              <span style={{ color: 'var(--text-muted)' }}>Input Daemon: {sidecarStatus}</span>
            </div>
          )}
          <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
            <span className={`led-lamp ${connected ? 'active-green' : 'active-amber'}`} />
            <span style={{ color: 'var(--text-muted)' }}>Signaling: {connected ? 'Online' : 'Connecting...'}</span>
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
          <ScreenStream
            isHost={isHost}
            controllerHolderName={currentHolderMember?.username || 'Host'}
            isTokenHolder={isTokenHolder}
          />
          <GamepadVisualizer snapshot={gamepadSnapshot} isTokenHolder={isTokenHolder} />
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
            currentClientId={clientId}
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
