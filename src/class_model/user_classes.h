#pragma once

#include <iostream>
#include <string>
#include "facility_classes.h"
#include <vector>
using namespace std;

// There's a lot of errors because I'm basing my code off of some unwritten code
// I'll fix this later
class User {
public:
    int userId;
    string name;
    string email;

    vector<string> email_list;

    User() {
        email_list.reserve(10);
    }

    void reserveRoom(Room& room)
    {
        if (room.getRoomAvailability() == true)
        { 
            room.setRoomAvailability(false);
            // Code/function to then create event request
        }
    }
    void editRoomRequest(Request& request)
    {
        cout << "What would you like to edit about your request or event? Enter number in terminal. \n";
        cout << "1. Event Name \n";
        cout << "2. Event Time \n";
        cout << "3. Event Date \n";
        int selection = 0;
        cin >> selection;

        switch (selection) {
            case 1: {
                cout << "Enter your new event name. \n";
                string eventName = "";
                cin >> eventName;
                request.setEventName(eventName);
                break;
            }
            
            case 2: {
                cout << "Enter your new event time. \n";
                string time = "";
                cin >> time;
                request.setEventTime(time);
                break;
            }

            case 3: {
                cout << "Enter your new event date. \n";
                string date = "";
                cin >> date;
                request.setEventDate(date);
                break;
            }
            
            default:
                cout << "Invalid option. \n";
                break;
        }
    }
    void cancelRoomRequest(Request& request) 
    {
        request.setStatus(false);
    }
    void login()
    {
       
        bool authorized = false;
        while(!authorized)
        {
            cout << "Please enter your email: ";
            cin >> email;

            for (int i = 0; i < email_list.size(); i++)
            {
                if (email == email_list[i])
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


    void user_register()
    {
        string input;
        cout << "Please enter your name: " << endl;
        cin >> input;
        email_list.push_back(input);
    }
};

class StaffMember : public User {
public:
    int staffId;
    bool isStudent;
    string organization;
    string jobTitle;
};

class EventApprover : public User {
public:
    int approverId;

    void manageEvent(Request& request)
    {
        cout << "What would you like to do with this request? Enter a number. \n";
        cout << "1. Approve request \n";
        cout << "2. Deny request \n";
        int selection = 0;
        cin >> selection;

        switch (selection) {
            case 1:
                request.setStatus(true);
                break;
            
            case 2:
                request.setStatus(false);
                break;
        }
    }
};

class FacilityCoordinator : public EventApprover {
public:
    int coordinatorId;

    void manageFacility(vector<Facility>& facilityList)
    {
        // Code to pull up facility list
        cout << "Which facility would you like to manage? Enter a number. \n";
        int selection = -1;
        cin >> selection;

        if (selection > -1 && selection < facilityList.size())
        {
            cout << "What would you like to do with this facility? \n";
            cout << "1. Open facility \n";
            cout << "2. Close facility \n";
            cout << "3. Edit facility description \n";
            cout << "4. Open facility room \n";
            cout << "5. Close facility room \n";
            int option = 0;
            cin >> option;

            switch (option) {
                case 1:
                    facilityList[selection].setAvailability(true);
                    break;
                
                case 2:
                    facilityList[selection].setAvailability(false);
                    break;
                
                case 3: {
                    cout << "Enter new facility description. \n";
                    string desc = "";
                    cin >> desc;
                    facilityList[selection].setDescription(desc);
                    break;
                }
                
                case 4:
                    // Code to print facility rooms
                    // Code to open room
                    break;
                
                case 5:
                    // Code to print facility rooms then close
                    break;
                
                default:
                    cout << "Invalid option. \n";
                    break;
            }
        }

        else
        {
            cout << "Invalid facility. \n";
        }
    }
};

class SystemAdministrator : public EventApprover {
public:   
    int adminId;

    void generateUsageReport() 
    {

        cout << "Generating usage report... \n";
        cout << "Total booked facilities: " <<"\n";
        cout << "Total booked rooms: " << "\n";
        cout << "Total active events: ";
        cout << "Total declined events: ";
        cout << "Total users: ";
    }
};

class Request {
public:
    string roomName;
    int roomNumber;
    string eventName;
    string eventTime;
    string eventDate;
    bool status;
    
    void setEventName(string newName) {
        eventName = newName;
    }

    void setEventTime(string newTime) {
        eventName = newTime;
    }

    void setEventDate(string newDate) {
        eventDate = newDate;
    }

    void setStatus(bool newStatus) 
    {
        status = newStatus;
    }

    void requestSpecialEquipment() {
        cout << "Equipment requested!" << endl;
    }

    void requestSecurity() 
    {
        cout << "Security requested!" << endl;
    }

    void requestEventSupport() 
    {
        cout << "Support requested!" << endl;
    }

};