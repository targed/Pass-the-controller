#include <iostream>
#include "state_diagram_two.h"

Room_Status& Room_Status::getInstance() {
    static Room_Status instance;
    return instance;
}



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
            break;
    }
}