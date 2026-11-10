#ifndef STATE_DIAGRAM_H
#define STATE_DIAGRAM_H

/*
    To use state diagram two in main -
        Add:
        #include "state_diagram_two/state_diagram_two.h"

        Create a room status instance:
        Room_Status& status = Room_Status::getInstance();

        To input an event:
        status.newEvent(RoomEvent::<name_of_event>);

        For example: status.newEvent(RoomEvent::Commision);

*/



 enum class RoomState {
        Start,
        Avaliable,
        TenativlyHeld,
        Reserved,
        InUse,
        Maintenance,
        Blackout,
        Final
    };


    // If the event names cause some confusion, please consult State Diagram #2
    enum class RoomEvent {
        Commission,
        Requested,
        Approved,
        Timeout,
        Checkin,
        Checkout,
        Flag,
        Repaired
    };



class Room_Status
{
    private:
        RoomState current_state;
        Room_Status() {current_state = RoomState::Start;};

    public:
        static Room_Status& getInstance();
        void newEvent(RoomEvent event);
};

#endif