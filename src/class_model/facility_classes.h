#pragma once

#include <iostream>
#include <string>
#include <vector>

// Stores a small list of upcoming events.
class EventList {
public:
    int requestId = 0;
    std::vector<std::string> events;

    void addEvent(const std::string& eventName) {
        events.push_back(eventName);
    }

    void displayUpcomingEvents() const {
        std::cout << "Upcoming events:\n";
        for (const std::string& event : events) {
            std::cout << "- " << event << '\n';
        }
    }
};

// Represents a room and stores its booked time slots.
class Room {
public:
    std::string roomName;
    int roomNumber = 0;
    int capacity = 0;
    bool roomAvailable = true;
    std::vector<std::string> availableTimes;
    std::vector<std::string> availableDates;
    std::vector<std::string> bookedSlots;

    bool getRoomAvailability() const {
        return roomAvailable;
    }

    void setRoomAvailability(bool available) {
        roomAvailable = available;
    }

    void setAvailableTimes(const std::string& date, const std::string& time) {
        availableDates.push_back(date);
        availableTimes.push_back(time);
    }

    bool checkConflicts(const std::string& date, const std::string& time) const {
        for (const std::string& slot : bookedSlots) {
            if (slot == date + " " + time) {
                return true;
            }
        }
        return false;
    }

    bool reserveRoom(const std::string& date, const std::string& time) {
        if (!roomAvailable || checkConflicts(date, time)) {
            return false;
        }
        bookedSlots.push_back(date + " " + time);
        return true;
    }
};

// Represents a building containing rooms.
class Facility {
public:
    std::string facilityName;
    std::string address;
    std::string specialDetails;
    bool isAvailable = true;
    std::vector<Room> rooms;

    void addRoom(const Room& room) {
        rooms.push_back(room);
    }

    void setAvailability(bool available) {
        isAvailable = available;
    }

    void setDescription(const std::string& description) {
        specialDetails = description;
    }

    bool findRooms(int roomNumber) const {
        for (const Room& room : rooms) {
            if (room.roomNumber == roomNumber) {
                return true;
            }
        }
        return false;
    }

    void listRooms() const {
        std::cout << "Rooms in " << facilityName << ":\n";
        for (const Room& room : rooms) {
            std::cout << "- " << room.roomNumber << ": " << room.roomName << '\n';
        }
    }

    void listDetails() const {
        std::cout << facilityName << " at " << address << '\n';
        std::cout << specialDetails << '\n';
    }
};

// Prints simulated notifications.
class NotificationService {
public:
    std::string reminderTime;
    std::string reminderMessage;

    void sendNotification() const {
        std::cout << "Notification: " << reminderMessage << '\n';
    }

    void cancelNotification() const {
        std::cout << "Cancelled notification: " << reminderMessage << '\n';
    }

    void sendConfirmation() const {
        std::cout << "Confirmation: " << reminderMessage << '\n';
    }
};

// Simulates checking if a security officer is available at a given time.
class CampusSecurity {
public:
    std::string name;
    int userId = 0;
    std::string availableTime;

    bool checkAvailability(const std::string& time) const {
        return time == availableTime;
    }
};

