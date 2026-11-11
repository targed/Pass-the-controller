#include <string>
using namespace std;

class User {
public:
    int userId;
    string name;
    string email;

    void reserveRoom();
    void editRoomRequest();
    void cancelRoomRequest();
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

    void manageEvent();
};

class FacilityCoordinator : public EventApprover {
public:
    int coordinatorId;

    bool approveSubmission();
    void manageFacility();
    bool manageFacilityAvailability();
};

class SystemAdministrator : public User {
public:   
    int adminId;

    bool approveReservation();
    bool denyReservation();
    void generateUsageReport();
};