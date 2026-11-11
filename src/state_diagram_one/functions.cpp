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

void Reservation_Request::newEvent(Event event) {
    switch(event) {
        case Event::createDraft:
        {   if (current_state == State::Start)
            {
                current_state = State::Draft;
                std::clog << "SUCCESS: CURRENT_STATUS = DRAFT" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::submitRequest:
        {   if (current_state == State::Draft)
            {
                current_state = State::Submitted;
                std::clog << "SUCCESS: CURRENT_STATUS = SUBMITTED" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::routeToApprover:
        {   if (current_state == State::Submitted)
            {
                current_state = State::Under_Review;
                std::clog << "SUCCESS: CURRENT_STATUS = UNDER_REVIEW" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::reject:
        {   if (current_state == State::Under_Review)
            {
                current_state = State::Rejected;
                std::clog << "SUCCESS: CURRENT_STATUS = REJECTED" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::evaluateEvent:
        {   if (current_state == State::Under_Review)
            {
                current_state = State::Security_Review;
                std::clog << "SUCCESS: CURRENT_STATUS = SECURITY_REVIEW" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::approve:
        {   if (current_state == State::Under_Review)
            {
                current_state = State::Approved;
                std::clog << "SUCCESS: CURRENT_STATUS = APPROVED" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::requestRevision:
        {   if (current_state == State::Security_Review)
            {
                current_state = State::Draft;
                std::clog << "SUCCESS: CURRENT_STATUS = DRAFT" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::securityCleared:
        {   if (current_state == State::Security_Review)
            {
                current_state = State::Approved;
                std::clog << "SUCCESS: CURRENT_STATUS = APPROVED" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::cancelRequest:
        {   if (current_state == State::Submitted || current_state == State::Approved)
            {
                current_state = State::Cancelled;
                std::clog << "SUCCESS: CURRENT_STATUS = CANCELLED" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::eventStartTimeReached:
        {   if (current_state == State::Approved)
            {
                current_state = State::In_Progress;
                std::clog << "SUCCESS: CURRENT_STATUS = IN_PROGRESS" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        case Event::eventConcluded:
        {   if (current_state == State::In_Progress)
            {
                current_state = State::Completed;
                std::clog << "SUCCESS: CURRENT_STATUS = COMPLETED" << std::endl;
            }
            else
            {
                std::clog << "ERROR: CURRENT STATE HAS NO TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }
        default:
            std::clog << "ERROR: INVALID EVENT" << std::endl;
            break;
    }
}