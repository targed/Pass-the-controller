#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <ctime>
#include <algorithm>

namespace domain {

/**
 * @brief Seven continuous operational states of a physical room/space on campus.
 */
enum class RoomStatus {
    Available,     ///< Clean, unlocked for walk-in inspections, bookable
    Reserved,      ///< Confirmed booking holds calendar slot; awaiting staging window
    Setup,         ///< Facility support configuring AV, furniture, equipment
    Occupied,      ///< Event actively occurring, smart doors unlocked
    Turnover,      ///< Event concluded, room locked, custodial staff cleaning/resetting
    OutOfService,  ///< Maintenance blackout or reported damage blocking bookings
    EmergencyHold  ///< Security lockdown, doors locked, active reservations suspended
};

/**
 * @brief Convert RoomStatus enum to human-readable string representation.
 */
inline std::string roomStatusToString(RoomStatus status) {
    switch (status) {
        case RoomStatus::Available:     return "Available";
        case RoomStatus::Reserved:      return "Reserved";
        case RoomStatus::Setup:         return "Setup";
        case RoomStatus::Occupied:      return "Occupied";
        case RoomStatus::Turnover:      return "Turnover";
        case RoomStatus::OutOfService:  return "OutOfService";
        case RoomStatus::EmergencyHold: return "EmergencyHold";
    }
    return "Unknown";
}

/**
 * @brief Convert string representation back to RoomStatus enum.
 */
inline RoomStatus stringToRoomStatus(std::string_view str) {
    if (str == "Available")     return RoomStatus::Available;
    if (str == "Reserved")      return RoomStatus::Reserved;
    if (str == "Setup")         return RoomStatus::Setup;
    if (str == "Occupied")      return RoomStatus::Occupied;
    if (str == "Turnover")      return RoomStatus::Turnover;
    if (str == "OutOfService")  return RoomStatus::OutOfService;
    if (str == "EmergencyHold") return RoomStatus::EmergencyHold;
    return RoomStatus::Available;
}

/**
 * @brief Time interval during which a facility is unavailable for reservations.
 */
struct BlackoutWindow {
    std::time_t start{0};
    std::time_t end{0};
    std::string reason{"Maintenance"};

    bool overlaps(std::time_t queryStart, std::time_t queryEnd) const {
        return (start < queryEnd) && (queryStart < end);
    }
};

/**
 * @brief Models a physical campus venue (auditorium, classroom, athletic center).
 */
class Facility {
public:
    Facility(std::string facilityId,
             std::string name,
             std::string building,
             int capacity,
             double hourlyRate = 0.0,
             RoomStatus status = RoomStatus::Available,
             std::vector<std::string> supportedEquipment = {})
        : facilityId_(std::move(facilityId)),
          name_(std::move(name)),
          building_(std::move(building)),
          capacity_(capacity),
          hourlyRate_(hourlyRate),
          status_(status),
          supportedEquipment_(std::move(supportedEquipment)) {}

    // Availability and schedule evaluation
    bool checkAvailability(std::time_t start, std::time_t end) const {
        // Physical spaces in emergency hold or out of service cannot be booked
        if (status_ == RoomStatus::OutOfService || status_ == RoomStatus::EmergencyHold) {
            return false;
        }
        for (const auto& window : blackoutWindows_) {
            if (window.overlaps(start, end)) {
                return false;
            }
        }
        return true;
    }

    void setBlackout(std::time_t start, std::time_t end, std::string reason = "Maintenance Blackout") {
        blackoutWindows_.push_back({start, end, std::move(reason)});
    }

    void clearBlackouts() {
        blackoutWindows_.clear();
    }

    // Status management
    void setStatus(RoomStatus newStatus) {
        status_ = newStatus;
    }

    void updateStatus(RoomStatus newStatus) {
        setStatus(newStatus);
    }

    RoomStatus getStatus() const {
        return status_;
    }

    // Equipment capabilities
    bool hasEquipment(std::string_view equipment) const {
        return std::find(supportedEquipment_.begin(), supportedEquipment_.end(), equipment) != supportedEquipment_.end();
    }

    void addSupportedEquipment(std::string equipment) {
        if (!hasEquipment(equipment)) {
            supportedEquipment_.push_back(std::move(equipment));
        }
    }

    // Accessors
    const std::string& getFacilityId() const { return facilityId_; }
    const std::string& getName() const { return name_; }
    const std::string& getBuilding() const { return building_; }
    int getCapacity() const { return capacity_; }
    double getHourlyRate() const { return hourlyRate_; }
    const std::vector<std::string>& getSupportedEquipment() const { return supportedEquipment_; }
    const std::vector<BlackoutWindow>& getBlackoutWindows() const { return blackoutWindows_; }

    void setName(std::string name) { name_ = std::move(name); }
    void setBuilding(std::string building) { building_ = std::move(building); }
    void setCapacity(int capacity) { capacity_ = capacity; }
    void setHourlyRate(double rate) { hourlyRate_ = rate; }
    void setSupportedEquipment(std::vector<std::string> equipment) { supportedEquipment_ = std::move(equipment); }

private:
    std::string facilityId_;
    std::string name_;
    std::string building_;
    int capacity_{0};
    double hourlyRate_{0.0};
    RoomStatus status_{RoomStatus::Available};
    std::vector<std::string> supportedEquipment_;
    std::vector<BlackoutWindow> blackoutWindows_;
};

} // namespace domain
