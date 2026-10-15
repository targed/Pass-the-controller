import { useEffect, useRef, useState, useCallback } from 'react';

export interface RoomMember {
  client_id: string;
  username: string;
  is_host: boolean;
}

export interface ChatMessage {
  sender_id: string;
  username: string;
  text: string;
  timestamp: number;
}

export interface RoomState {
  roomCode: string;
  hostId: string;
  controllerHolder: string;
  controllerQueue: string[];
  members: RoomMember[];
}

export function useSignaling(
  serverUrl: string = 'ws://localhost:8080',
  roomCode: string,
  clientId: string,
  username: string,
  isHost: boolean
) {
  const wsRef = useRef<WebSocket | null>(null);
  const [connected, setConnected] = useState<boolean>(false);
  const [roomState, setRoomState] = useState<RoomState>({
    roomCode: '',
    hostId: '',
    controllerHolder: '',
    controllerQueue: [],
    members: [],
  });
  const [messages, setMessages] = useState<ChatMessage[]>([]);

  useEffect(() => {
    if (!roomCode || !clientId) return;

    const encodedUser = encodeURIComponent(username);
    const url = `${serverUrl}/ws/${roomCode}/${clientId}?username=${encodedUser}&is_host=${isHost}`;
    const ws = new WebSocket(url);
    wsRef.current = ws;

    ws.onopen = () => {
      setConnected(true);
      console.log(`[Signaling] Connected to room ${roomCode}`);
    };

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        const { type } = data;

        if (type === 'room_state') {
          setRoomState({
            roomCode: data.room_code,
            hostId: data.host_id,
            controllerHolder: data.controller_holder,
            controllerQueue: data.controller_queue,
            members: data.members,
          });
        } else if (type === 'controller_queue_updated') {
          setRoomState((prev) => ({
            ...prev,
            controllerQueue: data.controller_queue,
            controllerHolder: data.controller_holder,
          }));
        } else if (type === 'controller_passed' || type === 'controller_reclaimed') {
          setRoomState((prev) => ({
            ...prev,
            controllerHolder: data.controller_holder,
            controllerQueue: data.controller_queue,
          }));
        } else if (type === 'chat_message') {
          setMessages((prev) => [...prev, data]);
        }
      } catch (e) {
        console.error('[Signaling] Failed to parse message:', e);
      }
    };

    ws.onclose = () => {
      setConnected(false);
      console.log('[Signaling] Disconnected');
    };

    return () => {
      ws.close();
      wsRef.current = null;
    };
  }, [serverUrl, roomCode, clientId, username, isHost]);

  const sendChatMessage = useCallback((text: string) => {
    if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
      wsRef.current.send(JSON.stringify({ type: 'chat_message', text }));
    }
  }, []);

  const requestController = useCallback(() => {
    if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
      wsRef.current.send(JSON.stringify({ type: 'request_controller' }));
    }
  }, []);

  const passController = useCallback((targetId: string) => {
    if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
      wsRef.current.send(JSON.stringify({ type: 'pass_controller', target_id: targetId }));
    }
  }, []);

  const reclaimController = useCallback(() => {
    if (wsRef.current && wsRef.current.readyState === WebSocket.OPEN) {
      wsRef.current.send(JSON.stringify({ type: 'reclaim_controller' }));
    }
  }, []);

  return {
    connected,
    roomState,
    messages,
    sendChatMessage,
    requestController,
    passController,
    reclaimController,
  };
}
