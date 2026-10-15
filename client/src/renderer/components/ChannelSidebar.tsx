import React from 'react';
import { Hash, Volume2, Gamepad2, Settings, User } from 'lucide-react';

interface Props {
  username: string;
  roomCode: string;
  isHost: boolean;
  connected: boolean;
}

export const ChannelSidebar: React.FC<Props> = ({ username, roomCode, isHost, connected }) => {
  return (
    <div
      className="retro-panel"
      style={{
        width: '220px',
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        background: 'var(--bg-secondary)',
        borderRight: '1px solid var(--panel-border)',
      }}
    >
      {/* App & Server Title */}
      <div
        style={{
          padding: '12px',
          borderBottom: '1px solid var(--panel-border)',
          background: 'var(--bg-tertiary)',
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Gamepad2 size={18} color="var(--accent-green)" />
          <span style={{ fontFamily: 'var(--font-mono)', fontWeight: 700, fontSize: '13px', color: '#fff', letterSpacing: '0.5px' }}>
            PASS THE CONTROLLER
          </span>
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: '6px', marginTop: '6px', fontSize: '11px', color: 'var(--text-muted)' }}>
          <span className={`led-lamp ${connected ? 'active-green' : 'active-amber'}`} />
          <span style={{ fontFamily: 'var(--font-mono)' }}>ROOM: {roomCode || 'DISCONNECTED'}</span>
        </div>
      </div>

      {/* Channels List */}
      <div style={{ flex: 1, overflowY: 'auto', padding: '12px 8px' }}>
        {/* Text Channels */}
        <div style={{ fontSize: '10px', fontWeight: 700, color: 'var(--text-muted)', fontFamily: 'var(--font-mono)', marginBottom: '6px', paddingLeft: '6px' }}>
          TEXT CHANNELS
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '6px',
            padding: '6px 8px',
            borderRadius: '4px',
            background: 'var(--bg-accent)',
            color: 'var(--accent-cyan)',
            fontFamily: 'var(--font-mono)',
            fontSize: '12px',
            marginBottom: '4px',
            cursor: 'pointer',
          }}
        >
          <Hash size={14} /> session-chat
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '6px',
            padding: '6px 8px',
            borderRadius: '4px',
            color: 'var(--text-muted)',
            fontFamily: 'var(--font-mono)',
            fontSize: '12px',
            marginBottom: '12px',
            cursor: 'pointer',
          }}
        >
          <Hash size={14} /> clips-and-highlights
        </div>

        {/* Voice Channels */}
        <div style={{ fontSize: '10px', fontWeight: 700, color: 'var(--text-muted)', fontFamily: 'var(--font-mono)', marginBottom: '6px', paddingLeft: '6px' }}>
          VOICE CHANNELS
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '6px',
            padding: '6px 8px',
            borderRadius: '4px',
            color: 'var(--text-primary)',
            fontFamily: 'var(--font-mono)',
            fontSize: '12px',
            cursor: 'pointer',
          }}
        >
          <Volume2 size={14} color="var(--accent-green)" /> Game Audio + Voice
        </div>
      </div>

      {/* User Footer Profile */}
      <div
        style={{
          padding: '10px',
          borderTop: '1px solid var(--panel-border)',
          background: '#10131a',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <div
            style={{
              width: '28px',
              height: '28px',
              borderRadius: '4px',
              background: '#23293a',
              border: '1px solid #3c4663',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            <User size={16} color="var(--accent-cyan)" />
          </div>
          <div>
            <div style={{ fontSize: '12px', fontWeight: 700, color: '#fff' }}>{username}</div>
            <div style={{ fontSize: '10px', color: isHost ? 'var(--accent-amber)' : 'var(--text-muted)', fontFamily: 'var(--font-mono)' }}>
              {isHost ? 'SESSION HOST' : 'SPECTATOR'}
            </div>
          </div>
        </div>
        <Settings size={14} color="var(--text-muted)" style={{ cursor: 'pointer' }} />
      </div>
    </div>
  );
};
