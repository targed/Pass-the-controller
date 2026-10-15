import pytest
from fastapi.testclient import TestClient
from app.main import app, rooms

client = TestClient(app)


def test_health_check():
    response = client.get("/health")
    assert response.status_code == 200
    data = response.json()
    assert data["status"] == "healthy"
    assert data["service"] == "pass-the-controller-signaling"
    assert "active_rooms" in data


def test_room_status_nonexistent():
    response = client.get("/rooms/NONEXISTENT")
    assert response.status_code == 200
    data = response.json()
    assert data["exists"] is False


def test_websocket_room_lifecycle():
    room_code = "TESTROOM"
    with client.websocket_connect(f"/ws/{room_code}/player_1?username=Alice&is_host=true") as ws1:
        # Check that room state message was broadcast
        state1 = ws1.receive_json()
        assert state1["type"] == "room_state"
        assert state1["room_code"] == room_code
        assert state1["host_id"] == "player_1"
        assert state1["controller_holder"] == "player_1"

        # Connect second player
        with client.websocket_connect(f"/ws/{room_code}/player_2?username=Bob") as ws2:
            state2 = ws2.receive_json()
            assert state2["type"] == "room_state"
            # ws1 receives the broadcast that player 2 joined
            join_notice = ws1.receive_json()
            assert join_notice["type"] == "room_state"

            # Player 2 requests the controller
            ws2.send_json({"type": "request_controller"})
            queue_update1 = ws1.receive_json()
            queue_update2 = ws2.receive_json()
            assert queue_update1["type"] == "controller_queue_updated"
            assert queue_update2["type"] == "controller_queue_updated"
            assert "player_2" in queue_update1["controller_queue"]

            # Player 1 (Host) passes the controller to Player 2
            ws1.send_json({"type": "pass_controller", "target_id": "player_2"})
            pass_event2 = ws2.receive_json()
            assert pass_event2["type"] == "controller_passed"
            assert pass_event2["controller_holder"] == "player_2"

            # Host reclaims the controller
            ws1.send_json({"type": "reclaim_controller"})
            reclaim_event = ws2.receive_json()
            assert reclaim_event["type"] == "controller_reclaimed"
            assert reclaim_event["controller_holder"] == "player_1"
