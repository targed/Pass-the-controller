import React from 'react';
import { Gamepad2, ArrowRight, Shield, Crown, RotateCcw } from 'lucide-react';
import { RoomMember } from '../hooks/useSignaling.ts';

interface Props {
  members: RoomMember[];
  controllerHolderId: string;
  controllerQueue: string[];
  currentClientId: string;
  isHost: boolean;
  onRequestController: () => void;
  onPassController: (targetId: string) => void;
  onReclaimController: () => void;
}

export const MemberQueueSidebar: React.FC<Props> = ({
  members,
  controllerHolderId,
  controllerQueue,
  currentClientId,
  isHost,
  onRequestController,
  onPassController,
  onReclaimController,
}) => {
  const holderMember = members.find((m) => m.client_id === controllerHolderId);
  const isCurrentHolder = currentClientId === controllerHolderId;
  const inQueue = controllerQueue.includes(currentClientId);

  return (
    <div
      className="retro-panel"
      style={{
        width: '240px',
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        background: 'var(--bg-secondary)',
        borderLeft: '1px solid var(--panel-border)',
      }}
    >
      {/* Controller Token Management Widget */}
      <div
        style={{
          padding: '12px',
          borderBottom: '1px solid var(--panel-border)',
          background: 'var(--bg-tertiary)',
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: '6px', marginBottom: '8px' }}>
          <Gamepad2 size={16} color="var(--accent-green)" />
          <span style={{ fontFamily: 'var(--font-mono)', fontSize: '11px', fontWeight: 700, color: 'var(--accent-green)' }}>
            CONTROLLER TOKEN
          </span>
        </div>

        <div className="retro-inset" style={{ padding: '8px', marginBottom: '10px' }}>
          <div style={{ fontSize: '10px', color: 'var(--text-muted)', fontFamily: 'var(--font-mono)' }}>CURRENT HOLDER:</div>
          <div style={{ fontSize: '13px', fontWeight: 700, color: '#fff', marginTop: '2px', display: 'flex', alignItems: 'center', gap: '6px' }}>
            <span className="led-lamp active-green" />
            {holderMember ? holderMember.username : 'Host (Neutral)'}
          </div>
        </div>

        {/* Action Buttons */}
        <div style={{ display: 'flex', flexDirection: 'column', gap: '6px' }}>
          {!isCurrentHolder && (
            <button
              className={`retro-button ${inQueue ? '' : 'accent'}`}
              style={{ width: '100%', justifyContent: 'center' }}
              onClick={onRequestController}
              disabled={inQueue}
            >
              <Gamepad2 size={13} /> {inQueue ? 'In Queue (Waiting)' : 'Request Controller'}
            </button>
          )}

          {isHost && !isCurrentHolder && (
            <button
              className="retro-button danger"
              style={{ width: '100%', justifyContent: 'center' }}
              onClick={onReclaimController}
            >
              <RotateCcw size={13} /> Reclaim Controller
            </button>
          )}
        </div>
      </div>

      {/* Queue Section */}
      <div style={{ padding: '10px 12px', borderBottom: '1px solid var(--panel-border)' }}>
        <div style={{ fontSize: '10px', fontWeight: 700, color: 'var(--text-muted)', fontFamily: 'var(--font-mono)', marginBottom: '6px' }}>
          PASS QUEUE ({controllerQueue.length})
        </div>
        {controllerQueue.length === 0 ? (
          <div style={{ fontSize: '11px', color: 'var(--text-muted)', fontStyle: 'italic' }}>
            No one waiting in line.
          </div>
        ) : (
          <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
            {controllerQueue.map((cid, idx) => {
              const qMember = members.find((m) => m.client_id === cid);
              return (
                <div
                  key={cid}
                  style={{
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'space-between',
                    padding: '4px 6px',
                    background: '#12151e',
                    borderRadius: '3px',
                    fontSize: '11px',
                  }}
                >
                  <span>
                    #{idx + 1} {qMember?.username || cid}
                  </span>
                  {(isHost || isCurrentHolder) && (
                    <button
                      className="retro-button"
                      style={{ padding: '2px 6px', fontSize: '10px' }}
                      onClick={() => onPassController(cid)}
                    >
                      <ArrowRight size={10} /> Pass
                    </button>
                  )}
                </div>
              );
            })}
          </div>
        )}
      </div>

      {/* Active Room Members List */}
      <div style={{ flex: 1, overflowY: 'auto', padding: '10px 12px' }}>
        <div style={{ fontSize: '10px', fontWeight: 700, color: 'var(--text-muted)', fontFamily: 'var(--font-mono)', marginBottom: '6px' }}>
          IN ROOM ({members.length})
        </div>
        <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
          {members.map((m) => {
            const isPlaying = m.client_id === controllerHolderId;
            return (
              <div
                key={m.client_id}
                style={{
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'space-between',
                  padding: '4px 6px',
                  borderRadius: '3px',
                  fontSize: '11px',
                  background: isPlaying ? 'rgba(0, 255, 136, 0.08)' : 'transparent',
                }}
              >
                <div style={{ display: 'flex', alignItems: 'center', gap: '6px' }}>
                  <span className={`led-lamp ${isPlaying ? 'active-green' : 'active-cyan'}`} />
                  <span style={{ color: isPlaying ? 'var(--accent-green)' : 'var(--text-primary)', fontWeight: isPlaying ? 700 : 400 }}>
                    {m.username}
                  </span>
                  {m.is_host && <Crown size={11} color="var(--accent-amber)" />}
                </div>

                {(isHost || isCurrentHolder) && !isPlaying && (
                  <button
                    className="retro-button"
                    style={{ padding: '2px 6px', fontSize: '9px' }}
                    onClick={() => onPassController(m.client_id)}
                  >
                    Pass
                  </button>
                )}
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
};
