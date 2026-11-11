#include <iostream>
#include <string>
#include "state_diagram_two.h"



// Creates the singleton instance
Room_Status& Room_Status::getInstance() {
    static Room_Status instance;
    return instance;
}




// Reads the current enum state and outputs a matching string
std::string Room_Status::getState()
{
    switch(current_state)
    {
        case RoomState::Start:
        {
            return "Start";
        }
        break;

        case RoomState::Avaliable:
        {
            return "Avaliable";
        }
        break;
           
        case RoomState::TenativlyHeld:
        {
            return "TenativlyHeld";
        }
        break;

        case RoomState::Reserved:
        {
            return "Reserved";
        }
        break;

        case RoomState::InUse:
        {
            return "InUse";
        }
        break;

        case RoomState::Maintenance:
        {
            return "Maintenance";
        }
        break;
        
        case RoomState::Blackout:
        {
            return "Blackout";
        }
        break;

        case RoomState::Final:
        {
            return "Final";
        }
        break;

        default:
            return "INVALID";
    }
}




// Performs the state/transition logic of the state machine
// If an event is impossible from the current state, the state machine will remain in the current state
// If an event is possible from the current state, current state is updated accordingly
void Room_Status::newEvent(RoomEvent event)
{
    switch (event)
    {
        case RoomEvent::Commission:
        {   if (current_state == RoomState::Start)
            {
                current_state = RoomState::Avaliable;
                std::clog << "SUCCESS: CURRENT_STATUS = AVALIABLE" << std::endl;
            }
            else
            {
                    
                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }

        case RoomEvent::Requested:
        {   if (current_state == RoomState::Avaliable)
            {
                current_state = RoomState::TenativlyHeld;
                std::clog << "SUCCESS: CURRENT_STATUS = TENATIVLY_HELD" << std::endl;
            }
            else
            {
                    
                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }




        case RoomEvent::Approved:
        {
            if (current_state == RoomState::TenativlyHeld)
            {
                current_state = RoomState::Reserved;
                std::clog << "SUCCESS: CURRENT_STATUS = RESERVED" << std::endl;
            }
            else
            {

                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }


        case RoomEvent::Timeout:
        {
            if (current_state == RoomState::TenativlyHeld)
            {
                current_state = RoomState::Avaliable;
                std::clog << "SUCCESS: CURRENT_STATUS = AVALIABLE" << std::endl;
            }
            else
            {

                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }


        case RoomEvent::Checkin:
        {
            if (current_state == RoomState::Reserved)
            {
                current_state = RoomState::InUse;
                std::clog << "SUCCESS: CURRENT_STATUS = IN_USE" << std::endl;
            }
            else
            {

                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }



        case RoomEvent::Flag:
        {
            if (current_state == RoomState::Reserved)
            {
                current_state = RoomState::Blackout;
                std::clog << "SUCCESS: CURRENT_STATUS = BLACKOUT" << std::endl;
            }
            else if (current_state == RoomState::Avaliable)
            {
                current_state = RoomState::Blackout;
                std::clog << "SUCCESS: CURRENT_STATUS = BLACKOUT" << std::endl;
            }
            else
            {
                 std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }


        case RoomEvent::Checkout:
        {
            if (current_state == RoomState::InUse)
            {
                current_state = RoomState::Maintenance;
                std::clog << "SUCCESS: CURRENT_STATUS = MAINTENANCE" << std::endl;
            }
            else
            {

                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }


        case RoomEvent::Repaired:
        {
            if (current_state == RoomState::Blackout)
            {
                current_state = RoomState::Avaliable;
                std::clog << "SUCCESS: CURRENT_STATUS = AVALIABLE" << std::endl;
            }
            else
            {

                std::clog << "ERROR: CURRENT STATE HAS NOT TRANSITION FOR THIS EVENT" << std::endl;
            }
            break;
        }

        
        default:
            std::clog << "ERROR: INVALID EVENT" << std::endl;
            break;
    }
}