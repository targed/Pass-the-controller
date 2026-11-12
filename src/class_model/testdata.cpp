#include "user_classes.h"
#include "facility_classes.h"
#include "testdata.h"

std::vector<Facility> createTestData() {
    // Test data
    Room bigRoom;
    bigRoom.roomName = "Big Room";
    bigRoom.roomNumber = 1;
    bigRoom.capacity = 100;

    Room biggerRoom;
    biggerRoom.roomName = "Bigger Room";
    biggerRoom.roomNumber = 2;
    biggerRoom.capacity = 150;

    Room biggestRoom;
    biggestRoom.roomName = "Biggest Room";
    biggestRoom.roomNumber = 3;
    biggestRoom.capacity = 200;

    Room smallRoom;
    smallRoom.roomName = "Small Room";
    smallRoom.roomNumber = 4;
    smallRoom.capacity = 40;

    Facility bigFacility;
    bigFacility.facilityName = "Big Facility";
    bigFacility.address = "123 Main St.";
    bigFacility.specialDetails = "This is a really big facility.";
    bigFacility.addRoom(bigRoom);
    bigFacility.addRoom(biggerRoom);
    bigFacility.addRoom(biggestRoom);

    Facility smallFacility;
    smallFacility.facilityName = "Small Facility";
    smallFacility.address = "456 West Blvd.";
    smallFacility.specialDetails = "This is a really small facility.";
    smallFacility.addRoom(smallRoom);


    std::vector<Facility> listOfFacilities = {bigFacility, smallFacility};
    return listOfFacilities;

    StaffMember user;
    user.name = "Benny";
    user.userId = 1;
    user.email = "benny@gmail.com";
    user.jobTitle = "Janitor";
    user.isStudent = false;
    user.organization = "None";


    SystemAdministrator admin;
    admin.name = "Woody";
    admin.adminId = 1;
    admin.email = "woody@gamil.com";

    EventApprover approver;
    approver.name = "Larry";
    approver.approverId = 1;
    approver.email = "larry@gmail.com";

    FacilityCoordinator coordinator;
    coordinator.name = "Terry";
    coordinator.userId = 2;
    coordinator.approverId = 2;
    coordinator.email = "terry@gmail.com";
}


std::string roomName;
    int roomNumber = 0;
    int capacity = 0;
    bool roomAvailable = true;
    std::vector<std::string> availableTimes;
    std::vector<std::string> availableDates;
    std::vector<std::string> bookedSlots;


std::string facilityName;
    std::string address;
    std::string specialDetails;
    bool isAvailable = true;
    std::vector<Room> rooms;