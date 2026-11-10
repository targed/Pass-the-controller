#include <iostream>
#include <string_view>

enum class ControllerState {
    Disconnected,
    Connected,
    Active
};

std::string_view stateToString(ControllerState state) {
    switch (state) {
        case ControllerState::Disconnected: return "Disconnected";
        case ControllerState::Connected:    return "Connected";
        case ControllerState::Active:       return "Active";
    }
    return "Unknown";
}

int main() {
    ControllerState current = ControllerState::Connected;
    std::cout << "Project ready. Controller State: " << stateToString(current) << '\n';
    return 0;
}
