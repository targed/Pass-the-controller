import pytest
from agent_mock import MockGamepadHandler


def test_mock_handler_initial_state():
    handler = MockGamepadHandler()
    assert handler.total_packets == 0
    assert handler.current_state["triggers"]["LT"] == 0.0
    assert handler.current_state["triggers"]["RT"] == 0.0


def test_deadzone_clamping():
    handler = MockGamepadHandler(deadzone=0.1)
    # Inside deadzone
    assert handler.apply_deadzone(0.05) == 0.0
    assert handler.apply_deadzone(-0.09) == 0.0
    # Outside deadzone
    assert handler.apply_deadzone(0.5) == 0.5
    assert handler.apply_deadzone(-0.8) == -0.8
    # Boundary clamping
    assert handler.apply_deadzone(1.5) == 1.0
    assert handler.apply_deadzone(-1.5) == -1.0


def test_frame_processing():
    handler = MockGamepadHandler(deadzone=0.08)
    frame_payload = {
        "seq": 101,
        "buttons": {"A": 1, "B": 0, "X": 1},
        "triggers": {"LT": 0.5, "RT": 1.0},
        "axes": {"left_x": 0.04, "left_y": -0.85, "right_x": 0.9, "right_y": 0.02},
    }
    result = handler.process_frame(frame_payload)
    assert result["seq"] == 101
    assert "A" in result["pressed"]
    assert "X" in result["pressed"]
    assert "B" not in result["pressed"]
    assert result["triggers"]["RT"] == 1.0
    # Axis inside deadzone was zeroed
    assert result["axes"]["left_x"] == 0.0
    assert result["axes"]["right_y"] == 0.0
    # Axis outside deadzone was preserved
    assert result["axes"]["left_y"] == -0.85
    assert result["axes"]["right_x"] == 0.9
    assert handler.total_packets == 1


def test_reset():
    handler = MockGamepadHandler()
    handler.process_frame({"buttons": {"A": 1}})
    assert handler.current_state["buttons"].get("A") == 1
    handler.reset()
    assert handler.current_state["buttons"] == {}
