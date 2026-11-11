#ifndef STATE_DIAGRAM_ONE_H
#define STATE_DIAGRAM_ONE_H
#include <string>

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
    Reservation_Request() { current_state = State::Start; }
    public:
        static Reservation_Request & getInstance();

        void newEvent(Event event);

        std::string getState();
};

#endif