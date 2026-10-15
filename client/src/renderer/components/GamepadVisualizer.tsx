import React from 'react';
import { GamepadSnapshot } from '../hooks/useGamepad.ts';

interface Props {
  snapshot: GamepadSnapshot;
  isTokenHolder: boolean;
}

export const GamepadVisualizer: React.FC<Props> = ({ snapshot, isTokenHolder }) => {
  const { connected, buttons, triggers, axes } = snapshot;

  if (!connected) {
    return (
      <div className="retro-inset" style={{ padding: '8px 12px', fontSize: '11px', color: 'var(--text-muted)' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
          <span className="led-lamp" />
          <span>No Gamepad Detected (Plug in USB/Bluetooth controller)</span>
        </div>
      </div>
    );
  }

  const isBtnPressed = (name: string) => buttons[name] === 1;

  return (
    <div className="retro-panel" style={{ padding: '8px 12px', background: '#12151e' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '6px' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
          <span className={`led-lamp ${isTokenHolder ? 'active-green' : 'active-amber'}`} />
          <span style={{ fontFamily: 'var(--font-mono)', fontSize: '11px', color: isTokenHolder ? 'var(--accent-green)' : 'var(--text-primary)' }}>
            {isTokenHolder ? 'INPUT ACTIVE (YOU CONTROL GAME)' : 'GAMEPAD CONNECTED (SPECTATOR)'}
          </span>
        </div>
        <span style={{ fontSize: '10px', color: 'var(--text-muted)', fontFamily: 'var(--font-mono)' }}>
          {snapshot.id.slice(0, 24)}...
        </span>
      </div>

      <div style={{ display: 'flex', gap: '12px', alignItems: 'center', flexWrap: 'wrap' }}>
        {/* Face Buttons */}
        <div style={{ display: 'flex', gap: '4px' }}>
          {['A', 'B', 'X', 'Y'].map((btn) => (
            <span
              key={btn}
              style={{
                width: '20px',
                height: '20px',
                borderRadius: '50%',
                display: 'inline-flex',
                alignItems: 'center',
                justifyContent: 'center',
                fontSize: '10px',
                fontWeight: 700,
                fontFamily: 'var(--font-mono)',
                background: isBtnPressed(btn) ? 'var(--accent-green)' : '#23293a',
                color: isBtnPressed(btn) ? '#000' : 'var(--text-primary)',
                border: '1px solid #3c4663',
                boxShadow: isBtnPressed(btn) ? '0 0 6px var(--accent-green)' : 'none',
              }}
            >
              {btn}
            </span>
          ))}
        </div>

        {/* Bumpers & Triggers */}
        <div style={{ display: 'flex', gap: '6px', fontSize: '10px', fontFamily: 'var(--font-mono)' }}>
          <span style={{ color: isBtnPressed('LB') ? 'var(--accent-cyan)' : 'var(--text-muted)' }}>LB</span>
          <span style={{ color: isBtnPressed('RB') ? 'var(--accent-cyan)' : 'var(--text-muted)' }}>RB</span>
          <span style={{ color: triggers.LT > 0.1 ? 'var(--accent-cyan)' : 'var(--text-muted)' }}>
            LT:{(triggers.LT * 100).toFixed(0)}%
          </span>
          <span style={{ color: triggers.RT > 0.1 ? 'var(--accent-cyan)' : 'var(--text-muted)' }}>
            RT:{(triggers.RT * 100).toFixed(0)}%
          </span>
        </div>

        {/* Thumbsticks coordinates */}
        <div style={{ display: 'flex', gap: '8px', fontSize: '10px', fontFamily: 'var(--font-mono)', color: 'var(--text-muted)' }}>
          <span>L: ({axes.left_x.toFixed(2)}, {axes.left_y.toFixed(2)})</span>
          <span>R: ({axes.right_x.toFixed(2)}, {axes.right_y.toFixed(2)})</span>
        </div>
      </div>
    </div>
  );
};
