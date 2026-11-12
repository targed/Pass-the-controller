#include <iostream>
#include "state_diagram_one/state_diagram_one.h"
#include "state_diagram_two/state_diagram_two.h"
#include "user_classes.h"
#include "testdata.h"

int main() 
{
    

    User user;
    int choice = 0;
    bool choice_made = false;


   
    
   
    
    while(!choice_made)
    {

        std::cout << "Please select an option" << std::endl;
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;


        cin >> choice;

        if (choice == 1) // User selects login
        {
            choice_made = true;
            user.login();
        }
        else if (choice == 2) // User selects regiser, upon creaing their account they will sent back to the login menu and will be required to login with their account
        {
            user.user_register();
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

    // Example app flow loop
    // We can switch to an interrupt system later, just trying to get the logic order down
    do { 
        std::cout << "Please select an option" << std::endl;
        std::cout << "1. Reserve a room" << std::endl;
        std::cout << "2. Edit a request" << std::endl;
        std::cout << "3. Cancel a rquest" << std::endl;
        std::cout << "4. Manage an event" << std::endl;
        std::cout << "5. Generate usage reports" << std::endl;
        std::cin >> choice;

        switch(choice) {
            case 1:
                // opt 1
                break;
            case 2:
                // opt 2
                break;
            case 3:
                // opt 3
                break;
            case 4:
                // opt 4
                break;
            case 5:
                // opt 5
                break;
            default:
                // none chosen/invalid choice
        }
        
    } while(1);
   
    return 0;
}
