#include <iostream>
#include "state_diagram_one/state_diagram_one.h"
#include "state_diagram_two/state_diagram_two.h"
#include "user_classes.h"
#include "testdata.h"

int main() 
{
    User user;
    EventApprover approver;
    SystemAdministrator admin;
    int choice = 0;
    bool choice_made = false;
    std::vector<std::string> email_list = {"benny@gmail.com", "woody@gamil.com", 
        "larry@gmail.com", "terry@gmail.com"}; // Preloaded data

    std::string entered_email;
    
    email_list.reserve(10);

    while(!choice_made)
    {

        std::cout << "Please select an option" << std::endl;
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;


        cin >> choice;

        if (choice == 1) // User selects login
        {
            choice_made = true;
            bool authorized = false;
            while(!authorized)
            {
                cout << "Please enter your email: ";
                cin >> entered_email;

                for (int i = 0; i < email_list.size(); i++)
                {
                    if (entered_email == email_list[i])
                    {
                        authorized = true;
                        break;
                    }
                }

                if (!authorized)
                {
                    cout << "Invalid Email, please try again" << endl;
                }
            }
        }
        else if (choice == 2) // User selects regiser, upon creaing their account they will sent back to the login menu and will be required to login with their account
        {
            string input;
            cout << "Please enter your email: " << endl;
            cin >> input;
            email_list.push_back(input);
        }
        else // User selects an invalid option
        {
            cout << "Invalid, please try again";
        }

    }

    std::vector<Facility> facilityList = createTestData();
    for (auto facility: facilityList) {
        facility.listDetails();
        facility.listRooms();
    }

    // initialize singleton state machines
    Reservation_Request & request = Reservation_Request::getInstance();
    Room_Status & status = Room_Status::getInstance(); 

    // Example app flow loop
    // We can switch to an interrupt system later, just trying to get the logic order down
    do { 
        std::cout << "Please select an option" << std::endl;
        std::cout << "1. Reserve a room" << std::endl;
        std::cout << "2. Edit a request" << std::endl;
        std::cout << "3. Cancel a request" << std::endl;
        std::cout << "4. Manage an event" << std::endl;
        std::cout << "5. Generate usage reports" << std::endl;
        std::cin >> choice;

        switch(choice) {
            case 1:
                // Reserve a room
                int roomNum;
                std::cout << "Please enter room number: ";
                std::cin >> roomNum;
                // TODO: get reference of (pointer to) room object to reserve
                user.reserveRoom(/* Insert reference to room object here */);
                break;
            case 2:
                // Edit Request                
                user.editRoomRequest(Request newRequest);
                break;
            case 3:
                // Cancel Request
                user.cancelRoomRequest(/* Insert reference to request object */);
                break;
            case 4:
                if (type(EventApprover))
                {
                    approver.manageEvent(Request newRequest);
                }
                else
                {
                    std::cout << "Error: Not authorized for this action"
                }            
                    
                break;
            case 5:
                if (type(SystemAdministrator))
                {
                    admin.generateUsageReport();
                }
                else
                {
                    std::cout << "Error: Not authorized for this action"
                }
                
                break;
            default:
                // none chosen/invalid choice
                std::cerr << "Error: Invalid option. Please try again.\n" << std::endl;
        }
        
    } while(1);
   
    return 0;
}
