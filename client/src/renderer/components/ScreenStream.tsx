import React, { useRef, useState } from 'react';
import { Tv, MonitorUp, Wifi, Play, ShieldAlert } from 'lucide-react';
import { CaptureSource } from '../../preload/index.ts';

interface Props {
  isHost: boolean;
  controllerHolderName: string;
  isTokenHolder: boolean;
}

export const ScreenStream: React.FC<Props> = ({ isHost, controllerHolderName, isTokenHolder }) => {
  const videoRef = useRef<HTMLVideoElement | null>(null);
  const [streaming, setStreaming] = useState(false);
  const [sources, setSources] = useState<CaptureSource[]>([]);
  const [showPicker, setShowPicker] = useState(false);
  const [selectedSource, setSelectedSource] = useState<string>('');

  const handleOpenSourcePicker = async () => {
    if (window.passTheController?.getSources) {
      const availableSources = await window.passTheController.getSources();
      setSources(availableSources);
      setShowPicker(true);
    } else {
      // Browser fallback (WebRTC Screen Capture API)
      try {
        const stream = await navigator.mediaDevices.getDisplayMedia({
          video: true,
          audio: true,
        });
        if (videoRef.current) {
          videoRef.current.srcObject = stream;
          videoRef.current.play();
        }
        setStreaming(true);
      } catch (err) {
        console.warn('Screen capture cancelled or unavailable:', err);
      }
    }
  };

  const handleSelectSource = async (sourceId: string) => {
    setSelectedSource(sourceId);
    setShowPicker(false);
    try {
      const stream = await (navigator.mediaDevices as any).getUserMedia({
        audio: false,
        video: {
          mandatory: {
            chromeMediaSource: 'desktop',
            chromeMediaSourceId: sourceId,
            minWidth: 1280,
            maxWidth: 1920,
            minHeight: 720,
            maxHeight: 1080,
          },
        },
      });

      if (videoRef.current) {
        videoRef.current.srcObject = stream;
        videoRef.current.play();
      }
      setStreaming(true);
    } catch (err) {
      console.error('Failed to capture stream from source:', err);
    }
  };

  return (
    <div
      className="retro-panel retro-scanlines"
      style={{
        flex: 1,
        display: 'flex',
        flexDirection: 'column',
        position: 'relative',
        background: '#07090d',
        overflow: 'hidden',
        minHeight: '360px',
      }}
    >
      {/* Stream HUD Header */}
      <div
        style={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          padding: '6px 12px',
          background: 'rgba(23, 27, 38, 0.9)',
          borderBottom: '1px solid var(--panel-border)',
          zIndex: 20,
        }}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Tv size={16} color="var(--accent-green)" />
          <span style={{ fontFamily: 'var(--font-mono)', fontSize: '12px', color: 'var(--accent-green)', fontWeight: 700 }}>
            LIVE GAMEPLAY FEED
          </span>
          {streaming && (
            <span style={{ background: '#ff0055', color: '#fff', fontSize: '10px', padding: '1px 6px', borderRadius: '3px', fontWeight: 700 }}>
              LIVE 60FPS
            </span>
          )}
        </div>

        <div style={{ display: 'flex', alignItems: 'center', gap: '12px', fontSize: '11px', fontFamily: 'var(--font-mono)' }}>
          <span style={{ display: 'flex', alignItems: 'center', gap: '4px', color: 'var(--accent-cyan)' }}>
            <Wifi size={13} /> 18ms
          </span>
          <span style={{ color: 'var(--text-muted)' }}>H.264 / 1080p</span>
          {isHost && (
            <button className="retro-button" onClick={handleOpenSourcePicker}>
              <MonitorUp size={13} /> {streaming ? 'Change Screen' : 'Start Stream'}
            </button>
          )}
        </div>
      </div>

      {/* Main Video Viewport */}
      <div
        style={{
          flex: 1,
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          position: 'relative',
        }}
      >
        <video
          ref={videoRef}
          autoPlay
          playsInline
          muted={isHost}
          style={{
            width: '100%',
            height: '100%',
            objectFit: 'contain',
            display: streaming ? 'block' : 'none',
          }}
        />

        {!streaming && (
          <div style={{ textAlign: 'center', padding: '24px' }}>
            <div style={{ display: 'inline-block', padding: '16px', background: '#171b26', borderRadius: '50%', marginBottom: '12px' }}>
              <Tv size={36} color="var(--accent-cyan)" />
            </div>
            <h3 style={{ fontFamily: 'var(--font-mono)', color: 'var(--accent-cyan)', marginBottom: '6px' }}>
              {isHost ? 'READY TO BROADCAST' : 'AWAITING HOST STREAM'}
            </h3>
            <p style={{ color: 'var(--text-muted)', fontSize: '12px', maxWidth: '360px', margin: '0 auto 16px' }}>
              {isHost
                ? 'Select a game window or full display to stream video to friends.'
                : 'The game stream will appear here when the host starts broadcasting.'}
            </p>
            {isHost && (
              <button className="retro-button accent" onClick={handleOpenSourcePicker}>
                <Play size={14} /> Select Window to Stream
              </button>
            )}
          </div>
        )}

        {/* Controller Holder Overlay Badge */}
        {controllerHolderName && (
          <div
            style={{
              position: 'absolute',
              bottom: '12px',
              left: '12px',
              background: 'rgba(15, 17, 23, 0.85)',
              border: `1px solid ${isTokenHolder ? 'var(--accent-green)' : 'var(--panel-border)'}`,
              padding: '6px 12px',
              borderRadius: '4px',
              display: 'flex',
              alignItems: 'center',
              gap: '8px',
              zIndex: 15,
              backdropFilter: 'blur(4px)',
            }}
          >
            <span className={`led-lamp ${isTokenHolder ? 'active-green' : 'active-cyan'}`} />
            <span style={{ fontFamily: 'var(--font-mono)', fontSize: '11px', color: isTokenHolder ? 'var(--accent-green)' : 'var(--text-primary)' }}>
              Controller: <strong>{controllerHolderName}</strong>
            </span>
          </div>
        )}
      </div>

      {/* Source Selection Modal (Host Window Picker) */}
      {showPicker && (
        <div
          style={{
            position: 'absolute',
            top: 0, left: 0, right: 0, bottom: 0,
            background: 'rgba(0, 0, 0, 0.85)',
            zIndex: 50,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            padding: '24px',
          }}
        >
          <div className="retro-panel" style={{ width: '640px', maxHeight: '80%', display: 'flex', flexDirection: 'column', padding: '16px' }}>
            <h3 style={{ fontFamily: 'var(--font-mono)', color: 'var(--accent-cyan)', marginBottom: '12px' }}>
              Select Screen or Game Window
            </h3>
            <div style={{ flex: 1, overflowY: 'auto', display: 'grid', gridTemplateColumns: 'repeat(2, 1fr)', gap: '12px', marginBottom: '16px' }}>
              {sources.map((s) => (
                <div
                  key={s.id}
                  onClick={() => handleSelectSource(s.id)}
                  className="retro-panel"
                  style={{
                    padding: '8px',
                    cursor: 'pointer',
                    borderColor: selectedSource === s.id ? 'var(--accent-green)' : 'var(--panel-border)',
                  }}
                >
                  <img src={s.thumbnail} alt={s.name} style={{ width: '100%', height: '110px', objectFit: 'cover', borderRadius: '3px' }} />
                  <div style={{ fontSize: '11px', marginTop: '6px', whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' }}>
                    {s.name}
                  </div>
                </div>
              ))}
            </div>
            <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '8px' }}>
              <button className="retro-button" onClick={() => setShowPicker(false)}>Cancel</button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};
