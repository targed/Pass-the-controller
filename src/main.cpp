#include <iostream>
#include "state_diagram_one/state_diagram_one.h"
#include "state_diagram_two/state_diagram_two.h"
#include "class_model/user_classes.h"
#include "class_model/testdata.h"
#include <typeinfo>

int main() 
{
   // User *user = new SystemAdministrator temp_user;
    EventApprover approver;
    SystemAdministrator admin;
    User *user = &approver;
    int choice = 0;
    bool choice_made = false;
    std::vector<std::string> email_list = {"benny@gmail.com", "woody@gmail.com", 
        "larry@gmail.com", "terry@gmail.com"}; // Preloaded data

    std::string entered_email;
    
    email_list.reserve(10);

    while(!choice_made)
    {

        std::cout << "Please select an option" << std::endl;
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;


        std::cin >> choice;

        if (choice == 1) // User selects login
        {
            choice_made = true;
            bool authorized = false;
            while(!authorized)
            {
                std::cout << "Please enter your email: " << std::endl;
                std::getline(std::cin, entered_email);
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
            {
                int roomNum;
                std::cout << "Please enter room number: ";
                std::cin >> roomNum;
                Room *room = new Room;
                // TODO: get room object to reserve
                user->reserveRoom(*room);
                break;
            }

            case 2:
            {
                // Edit Request
                Request *req = new Request;
                user->editRoomRequest(*req);
                break;
            }
                
            case 3:
            {
                // Cancel Request
                Request *req1 = new Request;
                user->cancelRoomRequest(*req1);
                break;
            }
                
            case 4:
            {
                // Manage Event
                // approver.manageEvent(/*Request newRequest*/);
                break;
            }
                
            case 5:
            {
                // Generate reports
                admin.generateUsageReport();
                break;
            }
                
            default:
            {
                // none chosen/invalid choice
                std::cerr << "Error: Invalid option. Please try again.\n" << std::endl;
            }
                
        }
        
    } while(1);
   
    return 0;
}
