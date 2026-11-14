#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <memory>

namespace domain {

/**
 * @brief System-wide user roles governing permissions and access tiers.
 */
enum class UserRole {
    StudentOrgRep,
    FacultyStaff,
    FacilityCoord,
    EventAppr,
    SecurityOfficer,
    Admin
};

/**
 * @brief Convert UserRole enum to human-readable string representation.
 */
inline std::string userRoleToString(UserRole role) {
    switch (role) {
        case UserRole::StudentOrgRep:   return "StudentOrgRep";
        case UserRole::FacultyStaff:    return "FacultyStaff";
        case UserRole::FacilityCoord:   return "FacilityCoord";
        case UserRole::EventAppr:       return "EventAppr";
        case UserRole::SecurityOfficer: return "SecurityOfficer";
        case UserRole::Admin:           return "Admin";
    }
    return "Unknown";
}

/**
 * @brief Convert string representation back to UserRole enum.
 */
inline UserRole stringToUserRole(std::string_view str) {
    if (str == "StudentOrgRep" || str == "StudentOrgRepresentative") return UserRole::StudentOrgRep;
    if (str == "FacultyStaff" || str == "FacultyStaffMember")       return UserRole::FacultyStaff;
    if (str == "FacilityCoord" || str == "FacilityCoordinator")     return UserRole::FacilityCoord;
    if (str == "EventAppr" || str == "EventApprover")               return UserRole::EventAppr;
    if (str == "SecurityOfficer")                                   return UserRole::SecurityOfficer;
    if (str == "Admin" || str == "Administrator")                   return UserRole::Admin;
    return UserRole::StudentOrgRep;
}

/**
 * @brief Abstract base class for all authenticated users in the booking system.
 */
class User {
public:
    User(std::string userId,
         std::string fullName,
         std::string email,
         std::string department,
         std::string passwordHash = "")
        : userId_(std::move(userId)),
          fullName_(std::move(fullName)),
          email_(std::move(email)),
          department_(std::move(department)),
          passwordHash_(std::move(passwordHash)) {}

    virtual ~User() = default;

    // Pure virtual role interrogation
    virtual UserRole getRole() const = 0;
    virtual std::string getRoleName() const = 0;

    // Virtual capability / permission checks
    virtual bool canSubmit() const { return false; }
    virtual bool canApprove() const { return false; }
    virtual bool canCoordinate() const { return false; }
    virtual bool isExpeditedEligible() const { return false; }
    virtual bool isSecurityOfficer() const { return false; }
    virtual bool isAdmin() const { return false; }

    // Authentication helper
    bool verifyPassword(std::string_view password) const {
        return passwordHash_.empty() || passwordHash_ == password;
    }

    // Accessors
    const std::string& getUserId() const { return userId_; }
    const std::string& getFullName() const { return fullName_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getDepartment() const { return department_; }
    const std::string& getPasswordHash() const { return passwordHash_; }

    void setFullName(std::string fullName) { fullName_ = std::move(fullName); }
    void setEmail(std::string email) { email_ = std::move(email); }
    void setDepartment(std::string department) { department_ = std::move(department); }
    void setPasswordHash(std::string passwordHash) { passwordHash_ = std::move(passwordHash); }

protected:
    std::string userId_;
    std::string fullName_;
    std::string email_;
    std::string department_;
    std::string passwordHash_;
};

/**
 * @brief Models registered student club/organization officers.
 */
class StudentOrgRepresentative : public User {
public:
    StudentOrgRepresentative(std::string userId,
                             std::string fullName,
                             std::string email,
                             std::string department,
                             std::string organizationName,
                             std::string passwordHash = "")
        : User(std::move(userId), std::move(fullName), std::move(email),
               std::move(department), std::move(passwordHash)),
          organizationName_(std::move(organizationName)) {}

    UserRole getRole() const override { return UserRole::StudentOrgRep; }
    std::string getRoleName() const override { return "Student Organization Representative"; }

    bool canSubmit() const override { return true; }

    const std::string& getOrganizationName() const { return organizationName_; }
    void setOrganizationName(std::string organizationName) { organizationName_ = std::move(organizationName); }

private:
    std::string organizationName_;
};

/**
 * @brief Models academic and administrative faculty/staff members.
 * Eligible for expedited booking review for curricular/official university events.
 */
class FacultyStaffMember : public User {
public:
    FacultyStaffMember(std::string userId,
                       std::string fullName,
                       std::string email,
                       std::string department,
                       std::string employeeId,
                       std::string passwordHash = "")
        : User(std::move(userId), std::move(fullName), std::move(email),
               std::move(department), std::move(passwordHash)),
          employeeId_(std::move(employeeId)) {}

    UserRole getRole() const override { return UserRole::FacultyStaff; }
    std::string getRoleName() const override { return "Faculty/Staff Member"; }

    bool canSubmit() const override { return true; }
    bool isExpeditedEligible() const override { return true; }

    const std::string& getEmployeeId() const { return employeeId_; }
    void setEmployeeId(std::string employeeId) { employeeId_ = std::move(employeeId); }

private:
    std::string employeeId_;
};

/**
 * @brief Responsible for building schedules, maintenance blackout flags,
 * equipment staging, and custodial turnover.
 */
class FacilityCoordinator : public User {
public:
    FacilityCoordinator(std::string userId,
                        std::string fullName,
                        std::string email,
                        std::string department,
                        std::vector<std::string> assignedBuildings = {},
                        std::string passwordHash = "")
        : User(std::move(userId), std::move(fullName), std::move(email),
               std::move(department), std::move(passwordHash)),
          assignedBuildings_(std::move(assignedBuildings)) {}

    UserRole getRole() const override { return UserRole::FacilityCoord; }
    std::string getRoleName() const override { return "Facility Coordinator"; }

    bool canCoordinate() const override { return true; }

    const std::vector<std::string>& getAssignedBuildings() const { return assignedBuildings_; }
    void setAssignedBuildings(std::vector<std::string> buildings) { assignedBuildings_ = std::move(buildings); }
    void addAssignedBuilding(std::string building) { assignedBuildings_.push_back(std::move(building)); }

    bool managesBuilding(std::string_view building) const {
        if (assignedBuildings_.empty()) return true; // Unrestricted coordinator
        return std::find(assignedBuildings_.begin(), assignedBuildings_.end(), building) != assignedBuildings_.end();
    }

private:
    std::vector<std::string> assignedBuildings_;
};

/**
 * @brief Responsible for evaluating reservation requests against university policies,
 * approving, rejecting with justification, or requesting revisions.
 */
class EventApprover : public User {
public:
    EventApprover(std::string userId,
                  std::string fullName,
                  std::string email,
                  std::string department,
                  int approvalTier = 1,
                  std::string passwordHash = "")
        : User(std::move(userId), std::move(fullName), std::move(email),
               std::move(department), std::move(passwordHash)),
          approvalTier_(approvalTier) {}

    UserRole getRole() const override { return UserRole::EventAppr; }
    std::string getRoleName() const override { return "Event Approver"; }

    bool canApprove() const override { return true; }

    int getApprovalTier() const { return approvalTier_; }
    void setApprovalTier(int tier) { approvalTier_ = tier; }

private:
    int approvalTier_{1};
};

/**
 * @brief Models campus police or security personnel tasked with high-risk event review.
 */
class SecurityOfficer : public User {
public:
    SecurityOfficer(std::string userId,
                    std::string fullName,
                    std::string email,
                    std::string department = "Campus Security",
                    std::string badgeNumber = "",
                    std::string passwordHash = "")
        : User(std::move(userId), std::move(fullName), std::move(email),
               std::move(department), std::move(passwordHash)),
          badgeNumber_(std::move(badgeNumber)) {}

    UserRole getRole() const override { return UserRole::SecurityOfficer; }
    std::string getRoleName() const override { return "Campus Security Officer"; }

    bool isSecurityOfficer() const override { return true; }

    const std::string& getBadgeNumber() const { return badgeNumber_; }
    void setBadgeNumber(std::string badge) { badgeNumber_ = std::move(badge); }

private:
    std::string badgeNumber_;
};

/**
 * @brief System administrator with comprehensive oversight privileges.
 */
class Administrator : public User {
public:
    Administrator(std::string userId,
                  std::string fullName,
                  std::string email,
                  std::string department = "IT Administration",
                  std::string passwordHash = "")
        : User(std::move(userId), std::move(fullName), std::move(email),
               std::move(department), std::move(passwordHash)) {}

    UserRole getRole() const override { return UserRole::Admin; }
    std::string getRoleName() const override { return "System Administrator"; }

    bool canSubmit() const override { return true; }
    bool canApprove() const override { return true; }
    bool canCoordinate() const override { return true; }
    bool isAdmin() const override { return true; }
};

} // namespace domain
