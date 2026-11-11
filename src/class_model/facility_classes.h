#pragma once

#include <iostream>
#include <string>
#include <vector>

// Stores a small list of upcoming events.
class Event_List {
public:
    int Request_ID = 0;
    std::vector<std::string> events;

    void add_event(const std::string& eventName) {
        events.push_back(eventName);
    }

    void display_upcoming_events() const {
        std::cout << "Upcoming events:\n";
        for (const std::string& event : events) {
            std::cout << "- " << event << '\n';
        }
    }
};

// Represents a room and stores its booked time slots.
class Room {
public:
    std::string Room_Name;
    int Room_Number = 0;
    int Capacity = 0;
    bool Room_Available = true;
    std::vector<std::string> Available_Times;
    std::vector<std::string> Available_Dates;
    std::vector<std::string> bookedSlots;

    bool getRoomAvailability() const {
        return Room_Available;
    }

    void setRoomAvailability(bool available) {
        Room_Available = available;
    }

    void set_available_times(const std::string& date, const std::string& time) {
        Available_Dates.push_back(date);
        Available_Times.push_back(time);
    }

    bool check_conflicts(const std::string& date, const std::string& time) const {
        for (const std::string& slot : bookedSlots) {
            if (slot == date + " " + time) {
                return true;
            }
        }
        return false;
    }

    bool reserveRoom(const std::string& date, const std::string& time) {
        if (!Room_Available || check_conflicts(date, time)) {
            return false;
        }
        bookedSlots.push_back(date + " " + time);
        return true;
    }
};

// Represents a building containing rooms.
class Facility {
public:
    std::string Facility_Name;
    std::string Address;
    std::string Special_Details;
    bool Is_Available = true;
    std::vector<Room> rooms;

    void addRoom(const Room& room) {
        rooms.push_back(room);
    }

    void setAvailability(bool available) {
        Is_Available = available;
    }

    void setDescription(const std::string& description) {
        Special_Details = description;
    }

    bool findRooms(int roomNumber) const {
        for (const Room& room : rooms) {
            if (room.Room_Number == roomNumber) {
                return true;
            }
        }
        return false;
    }

    void listRooms() const {
        std::cout << "Rooms in " << Facility_Name << ":\n";
        for (const Room& room : rooms) {
            std::cout << "- " << room.Room_Number << ": " << room.Room_Name << '\n';
        }
    }

    void listDetails() const {
        std::cout << Facility_Name << " at " << Address << '\n';
        std::cout << Special_Details << '\n';
    }
};

// Prints simulated notifications.
class NotificationService {
public:
    std::string Reminder_Time;
    std::string Reminder_Message;

    void sendNotification() const {
        std::cout << "Notification: " << Reminder_Message << '\n';
    }

    void cancelNotification() const {
        std::cout << "Cancelled notification: " << Reminder_Message << '\n';
    }

    void sendConfirmation() const {
        std::cout << "Confirmation: " << Reminder_Message << '\n';
    }
};

// Simulates checking if a security officer is available at a given time.
class CampusSecurity {
public:
    std::string name;
    int user_id = 0;
    std::string available_time;

    bool check_availability(const std::string& time) const {
        return time == available_time;
    }
};
