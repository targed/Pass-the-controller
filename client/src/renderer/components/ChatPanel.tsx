import React, { useState, useRef, useEffect } from 'react';
import { Send, MessageSquare } from 'lucide-react';
import { ChatMessage } from '../hooks/useSignaling.ts';

interface Props {
  messages: ChatMessage[];
  onSendMessage: (text: string) => void;
  currentUsername: string;
}

export const ChatPanel: React.FC<Props> = ({ messages, onSendMessage, currentUsername }) => {
  const [inputText, setInputText] = useState('');
  const endRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    endRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages]);

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    if (inputText.trim()) {
      onSendMessage(inputText.trim());
      setInputText('');
    }
  };

  return (
    <div
      className="retro-panel"
      style={{
        display: 'flex',
        flexDirection: 'column',
        height: '100%',
        width: '320px',
        background: 'var(--bg-secondary)',
      }}
    >
      {/* Chat Header */}
      <div
        style={{
          padding: '8px 12px',
          borderBottom: '1px solid var(--panel-border)',
          display: 'flex',
          alignItems: 'center',
          gap: '8px',
          background: 'var(--bg-tertiary)',
        }}
      >
        <MessageSquare size={14} color="var(--accent-cyan)" />
        <span style={{ fontFamily: 'var(--font-mono)', fontSize: '12px', fontWeight: 700, color: 'var(--accent-cyan)' }}>
          SESSION CHAT
        </span>
      </div>

      {/* Message Feed */}
      <div
        className="retro-inset"
        style={{
          flex: 1,
          overflowY: 'auto',
          padding: '12px',
          margin: '8px',
          display: 'flex',
          flexDirection: 'column',
          gap: '8px',
        }}
      >
        {messages.length === 0 && (
          <div style={{ textAlign: 'center', color: 'var(--text-muted)', fontSize: '11px', marginTop: '20px' }}>
            No messages yet. Say hello!
          </div>
        )}

        {messages.map((m, idx) => {
          const isMe = m.username === currentUsername;
          const timeStr = new Date(m.timestamp * 1000).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });

          return (
            <div key={idx} style={{ fontSize: '12px', lineHeight: '1.4' }}>
              <div style={{ display: 'flex', alignItems: 'baseline', gap: '6px' }}>
                <span
                  style={{
                    fontWeight: 700,
                    color: isMe ? 'var(--accent-green)' : 'var(--accent-cyan)',
                    fontFamily: 'var(--font-mono)',
                    fontSize: '11px',
                  }}
                >
                  {m.username}:
                </span>
                <span style={{ fontSize: '10px', color: 'var(--text-muted)' }}>{timeStr}</span>
              </div>
              <div style={{ color: 'var(--text-primary)', wordBreak: 'break-word', marginTop: '2px' }}>
                {m.text}
              </div>
            </div>
          );
        })}
        <div ref={endRef} />
      </div>

      {/* Input Field */}
      <form onSubmit={handleSubmit} style={{ padding: '8px', borderTop: '1px solid var(--panel-border)', display: 'flex', gap: '6px' }}>
        <input
          type="text"
          value={inputText}
          onChange={(e) => setInputText(e.target.value)}
          placeholder="Type message..."
          style={{
            flex: 1,
            background: '#090b0e',
            border: '1px solid #343c54',
            borderRadius: '3px',
            padding: '6px 10px',
            color: '#fff',
            fontFamily: 'var(--font-sans)',
            fontSize: '12px',
            outline: 'none',
          }}
        />
        <button type="submit" className="retro-button accent" style={{ padding: '6px 10px' }}>
          <Send size={12} />
        </button>
      </form>
    </div>
  );
};
