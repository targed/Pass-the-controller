import { useEffect, useState, useRef } from 'react';

export interface GamepadSnapshot {
  connected: boolean;
  id: string;
  buttons: Record<string, number>;
  triggers: { LT: number; RT: number };
  axes: { left_x: number; left_y: number; right_x: number; right_y: number };
}

const BUTTON_NAMES = [
  'A', 'B', 'X', 'Y',
  'LB', 'RB', 'LT_BTN', 'RT_BTN',
  'BACK', 'START',
  'LEFT_THUMB', 'RIGHT_THUMB',
  'DPAD_UP', 'DPAD_DOWN', 'DPAD_LEFT', 'DPAD_RIGHT',
  'GUIDE'
];

export function useGamepad(onFrame?: (snapshot: GamepadSnapshot) => void) {
  const [snapshot, setSnapshot] = useState<GamepadSnapshot>({
    connected: false,
    id: 'No Gamepad Detected',
    buttons: {},
    triggers: { LT: 0, RT: 0 },
    axes: { left_x: 0, left_y: 0, right_x: 0, right_y: 0 },
  });

  const onFrameRef = useRef(onFrame);
  onFrameRef.current = onFrame;

  useEffect(() => {
    let animId: number;

    const pollGamepads = () => {
      const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
      const gp = gamepads[0]; // Primary controller

      if (gp && gp.connected) {
        const buttons: Record<string, number> = {};
        BUTTON_NAMES.forEach((name, idx) => {
          if (gp.buttons[idx]) {
            buttons[name] = gp.buttons[idx].pressed ? 1 : 0;
          }
        });

        // Triggers (analog values 0.0 - 1.0)
        const ltVal = gp.buttons[6] ? gp.buttons[6].value : 0;
        const rtVal = gp.buttons[7] ? gp.buttons[7].value : 0;

        // Thumbsticks (-1.0 to 1.0)
        const axes = {
          left_x: gp.axes[0] || 0,
          left_y: -(gp.axes[1] || 0), // Standard XInput invert Y
          right_x: gp.axes[2] || 0,
          right_y: -(gp.axes[3] || 0),
        };

        const currentSnapshot: GamepadSnapshot = {
          connected: true,
          id: gp.id,
          buttons,
          triggers: { LT: ltVal, RT: rtVal },
          axes,
        };

        setSnapshot(currentSnapshot);
        if (onFrameRef.current) {
          onFrameRef.current(currentSnapshot);
        }
      } else {
        setSnapshot((prev) => (prev.connected ? { ...prev, connected: false } : prev));
      }

      animId = requestAnimationFrame(pollGamepads);
    };

    animId = requestAnimationFrame(pollGamepads);
    return () => cancelAnimationFrame(animId);
  }, []);

  return snapshot;
}
