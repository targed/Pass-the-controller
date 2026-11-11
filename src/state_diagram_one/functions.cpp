#include <iostream>
#include <string>
#include "state_diagram_one.h"

Reservation_Request & Reservation_Request::getInstance() {
    static Reservation_Request instance;
    return instance;
}

std::string Reservation_Request::getState() {
    switch(current_state) {
        case State::Start:
            return "Start";
        break;
        
        case State::Draft:
            return "Draft";
        break;

        case State::Submitted:
            return "Submitted";
        break;

        case State::Under_Review:
            return "Under Review";
        break;

        case State::Rejected:
            return "Rejected";
        break;

        case State::Security_Review:
            return "Security Review";
        break;

        case State::Cancelled:
            return "Cancelled";
        break;

        case State::Approved:
            return "Approved";
        break;
        
        case State::In_Progress:
            return "In Progress";
        break;

        case State::Completed:
            return "Completed";
        break;

        default:
            return "INVALID";
    }
}