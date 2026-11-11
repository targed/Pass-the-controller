#ifndef STATE_DIAGRAM_ONE_H
#define STATE_DIAGRAM_ONE_H
#include <string>

/*
    To use State Diagram #1 in main -
        Add:
        #include "state_diagram_one/state_diagram_two.h"

        
        Create a reservation request instance:
        Reservation_Request& request = Reservation_Request::getInstance();


        To input an event:
        request.newEvent(Event::<name_of_event>);
        For example: request.newEvent(Event::createDraft);


        To get the current state:
        request.getState();

*/

enum class State {
    Start,
    Draft,
    Submitted,
    Under_Review,
    Rejected,
    Security_Review,
    Cancelled,
    Approved,
    In_Progress,
    Completed
};

enum class Event {
    createDraft,
    submitRequest,
    routeToApprover,
    reject,
    evaluateEvent,
    approve,
    requestRevision,
    securityCleared,
    cancelRequest,
    eventStartTimeReached,
    eventConcluded
};

class Reservation_Request {
    State current_state;
    // initialize with state at start by default
    Reservation_Request() { current_state = State::Start; }
    public:
        // initialize singleton object
        static Reservation_Request & getInstance();

        // perform transition logic upon receiving event name
        void newEvent(Event event);

        // output current state as a string
        std::string getState();
};

#endif