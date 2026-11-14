#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "domain/User.hpp"
#include "domain/Facility.hpp"
#include "domain/Reservation.hpp"
#include "domain/NotificationService.hpp"
#include "state/ReservationStateMachine.hpp"
#include "state/FacilityStateMachine.hpp"
#include "service/BookingSystem.hpp"
#include "service/ScenarioRunner.hpp"
#include "test/TestSuite.hpp"
#include "web/HttpServer.hpp"
#include "web/WebDashboard.hpp"

namespace smartcampus {
    namespace domain = ::domain;
    namespace state = ::state;
    namespace service = ::service;
}

// Global pointer for graceful signal termination of web server
static std::atomic<smartcampus::web::HttpServer*> g_activeServer{nullptr};

extern "C" void handleSignal(int sig) {
    if (auto* s = g_activeServer.load()) {
        std::cout << "\n[Server] Shutdown signal (" << sig << ") received. Gracefully stopping...\n";
        s->stop();
    }
}

namespace {

void printHelp(const char* progName) {
    std::cout << "\n"
              << "================================================================================\n"
              << "   SMART CAMPUS EVENT & FACILITY BOOKING SYSTEM - C++20 AUTONOMOUS ENGINE       \n"
              << "================================================================================\n"
              << "\n"
              << "Usage:\n"
              << "  " << progName << " [options]\n\n"
              << "Modes & Options:\n"
              << "  --web [port]          Start the embedded HTTP server and visual web dashboard.\n"
              << "                        Optional port argument (defaults to 8080).\n"
              << "                        Examples:\n"
              << "                          " << progName << " --web\n"
              << "                          " << progName << " --web 8080\n"
              << "                          " << progName << " --web=9000\n\n"
              << "  --cli                 Launch the interactive terminal management console.\n"
              << "                        Enables inspecting campus facilities, viewing reservations,\n"
              << "                        advancing the simulation clock, triggering emergency holds,\n"
              << "                        and running demonstration scenarios interactively.\n\n"
              << "  --scenario <1-6|all>  Execute an automated demonstration scenario and display\n"
              << "                        formatted ANSI state transitions and verification steps.\n"
              << "                        Scenarios:\n"
              << "                          1   Standard Happy Path Booking Lifecycle\n"
              << "                          2   Large Event Security Review & Officer Clearance\n"
              << "                          3   Approver Revision Request & Resubmission Loop\n"
              << "                          4   Facility Logistics, Equipment Staging & Turnover\n"
              << "                          5   Campus Emergency Lockdown & Restoration\n"
              << "                          6   Cancellation Policy & 24-Hour Lead Time Guard\n"
              << "                          all Run all 6 scenarios in sequence with summary table\n\n"
              << "  --test                Run the complete 14-suite automated test harness\n"
              << "                        (595+ assertions verifying domain, state engines, services,\n"
              << "                        and web dashboard). Returns exit code 0 on pass, 1 on fail.\n\n"
              << "  --help, -h            Display this comprehensive help message and exit.\n\n"
              << "Default Behavior:\n"
              << "  Running without arguments launches the embedded web server on port 8080.\n"
              << "================================================================================\n\n";
}

int runWebServer(int port) {
    if (port <= 0 || port > 65535) {
        std::cerr << "Error: Invalid port number " << port << ". Port must be between 1 and 65535.\n";
        return 1;
    }

    auto system = std::make_shared<smartcampus::service::BookingSystem>();
    smartcampus::web::HttpServer server(system, port);
    g_activeServer.store(&server);

    std::signal(SIGINT, handleSignal);
#if defined(SIGTERM)
    std::signal(SIGTERM, handleSignal);
#endif

    std::cout << "\n"
              << "╔══════════════════════════════════════════════════════════════════════════════╗\n"
              << "║       SMART CAMPUS EVENT & FACILITY BOOKING SYSTEM - WEB DASHBOARD           ║\n"
              << "╚══════════════════════════════════════════════════════════════════════════════╝\n"
              << "  Server running on: http://localhost:" << port << "\n"
              << "  Single-page Web Dashboard: http://localhost:" << port << "/\n"
              << "  REST API State Endpoint:  http://localhost:" << port << "/api/state\n"
              << "  Press Ctrl+C to stop the server.\n"
              << "════════════════════════════════════════════════════════════════════════════════\n\n";

    server.run();

    g_activeServer.store(nullptr);
    std::cout << "[Server] Web server terminated cleanly.\n";
    return 0;
}

int runScenarioMode(const std::string& choice) {
    smartcampus::service::ScenarioRunner runner(std::cout, true);

    if (choice == "all") {
        bool success = runner.runAll();
        return success ? 0 : 1;
    }

    try {
        int scenarioNum = std::stoi(choice);
        bool success = false;
        switch (scenarioNum) {
            case 1:
                success = runner.runHappyPath();
                break;
            case 2:
                success = runner.runLargeEventSecurityScenario();
                break;
            case 3:
                success = runner.runRevisionLoopScenario();
                break;
            case 4:
                success = runner.runFacilityLogisticsScenario();
                break;
            case 5:
                success = runner.runEmergencyLockdownScenario();
                break;
            case 6:
                success = runner.runCancellationRulesScenario();
                break;
            default:
                std::cerr << "Error: Unknown scenario number '" << scenarioNum
                          << "'. Please specify 1 to 6, or 'all'.\n";
                return 1;
        }
        return success ? 0 : 1;
    } catch (...) {
        std::cerr << "Error: Invalid scenario parameter '" << choice
                  << "'. Please specify 1, 2, 3, 4, 5, 6, or 'all'.\n";
        return 1;
    }
}

int runTestMode() {
    std::cout << "\nExecuting Smart Campus automated verification test suites...\n\n";
    int result = smartcampus::test::TestSuite::run(true);
    return (result == 0) ? 0 : 1;
}

void cliInspectFacilities(const smartcampus::service::BookingSystem& system) {
    auto facilities = system.getFacilities();
    std::cout << "\n============================== CAMPUS FACILITIES ==============================\n";
    std::cout << std::left << std::setw(10) << "ID"
              << std::setw(30) << "Name"
              << std::setw(10) << "Capacity"
              << std::setw(12) << "Rate/hr"
              << std::setw(16) << "Status"
              << std::setw(14) << "Doors Locked"
              << "\n";
    std::cout << std::string(92, '-') << "\n";
    for (const auto& f : facilities) {
        std::ostringstream rateStr;
        rateStr << "$" << std::fixed << std::setprecision(2) << f.getHourlyRate();
        std::cout << std::left << std::setw(10) << f.getFacilityId()
                  << std::setw(30) << f.getName()
                  << std::setw(10) << f.getCapacity()
                  << std::setw(12) << rateStr.str()
                  << std::setw(16) << smartcampus::domain::roomStatusToString(f.getStatus())
                  << std::setw(14) << (f.areDoorsLocked() ? "LOCKED" : "UNLOCKED")
                  << "\n";
    }
    std::cout << "Total facilities: " << facilities.size() << "\n\n";
}

void cliListReservations(const smartcampus::service::BookingSystem& system) {
    auto reservations = system.getReservations();
    std::cout << "\n================================ RESERVATIONS ================================\n";
    if (reservations.empty()) {
        std::cout << "No reservations currently registered.\n\n";
        return;
    }
    std::cout << std::left << std::setw(16) << "ID"
              << std::setw(12) << "Requester"
              << std::setw(10) << "Facility"
              << std::setw(26) << "Title"
              << std::setw(16) << "Status"
              << std::setw(10) << "Attendees"
              << std::setw(22) << "Start Time"
              << "\n";
    std::cout << std::string(112, '-') << "\n";
    for (const auto& r : reservations) {
        std::string title = r.getEventTitle();
        if (title.length() > 24) {
            title = title.substr(0, 23) + "…";
        }
        std::cout << std::left << std::setw(16) << r.getReservationId()
                  << std::setw(12) << r.getRequesterId()
                  << std::setw(10) << r.getFacilityId()
                  << std::setw(26) << title
                  << std::setw(16) << smartcampus::domain::reservationStatusToString(r.getStatus())
                  << std::setw(10) << r.getExpectedAttendance()
                  << std::setw(22) << smartcampus::service::BookingSystem::formatTime(r.getStartTime())
                  << "\n";
    }
    std::cout << "Total reservations: " << reservations.size() << "\n\n";
}

void cliCreateReservation(smartcampus::service::BookingSystem& system) {
    std::cout << "\n--- Create New Reservation ---\n";
    std::string userId, facilityId, title, eventType;
    int attendance = 25;
    int hoursFromNow = 24;
    int durationHours = 2;

    std::cout << "Enter Requester User ID [default: USR-001]: ";
    std::string line;
    std::getline(std::cin, line);
    userId = line.empty() ? "USR-001" : line;

    std::cout << "Enter Facility ID [default: FAC-101]: ";
    std::getline(std::cin, line);
    facilityId = line.empty() ? "FAC-101" : line;

    std::cout << "Enter Event Title [default: Robotics Demonstration]: ";
    std::getline(std::cin, line);
    title = line.empty() ? "Robotics Demonstration" : line;

    std::cout << "Enter Event Type [default: Workshop]: ";
    std::getline(std::cin, line);
    eventType = line.empty() ? "Workshop" : line;

    std::cout << "Enter Expected Attendees [default: 25]: ";
    std::getline(std::cin, line);
    if (!line.empty()) {
        try { attendance = std::stoi(line); } catch (...) { attendance = 25; }
    }

    std::cout << "Enter Hours from Current Time to Start [default: 24]: ";
    std::getline(std::cin, line);
    if (!line.empty()) {
        try { hoursFromNow = std::stoi(line); } catch (...) { hoursFromNow = 24; }
    }

    std::cout << "Enter Duration in Hours [default: 2]: ";
    std::getline(std::cin, line);
    if (!line.empty()) {
        try { durationHours = std::stoi(line); } catch (...) { durationHours = 2; }
    }

    std::time_t cur = system.getCurrentTime();
    std::time_t start = cur + static_cast<std::time_t>(hoursFromNow) * 3600;
    std::time_t end = start + static_cast<std::time_t>(durationHours) * 3600;

    std::string resId = system.createReservation(userId, facilityId, title, eventType, attendance, start, end);
    if (resId.empty()) {
        std::cerr << "Failed to create reservation: facility not found or invalid inputs.\n\n";
        return;
    }

    std::cout << "Reservation drafted successfully! Reservation ID: " << resId << "\n";
    std::cout << "Submit reservation now to initiate soft hold? (y/n) [default: y]: ";
    std::getline(std::cin, line);
    if (line.empty() || line == "y" || line == "Y") {
        auto res = system.submitReservation(resId);
        if (res.success) {
            std::cout << "Reservation submitted! " << res.message << "\n";
        } else {
            std::cout << "Submission rejected: " << res.message << "\n";
        }
    }
    std::cout << "\n";
}

void cliAdvanceTime(smartcampus::service::BookingSystem& system) {
    std::cout << "\n--- Advance Simulation Time ---\n";
    std::cout << "Current Simulated Time: "
              << smartcampus::service::BookingSystem::formatTime(system.getCurrentTime()) << "\n";
    std::cout << "Options:\n"
              << "  1. Advance +15 Minutes (900 seconds) - tests soft hold expiration\n"
              << "  2. Advance +1 Hour (3600 seconds)\n"
              << "  3. Advance +24 Hours (86400 seconds) - tests day transitions & lead time\n"
              << "  4. Custom minutes\n"
              << "Select option [1-4]: ";

    std::string line;
    std::getline(std::cin, line);
    std::time_t seconds = 0;
    if (line == "1") {
        seconds = 900;
    } else if (line == "2") {
        seconds = 3600;
    } else if (line == "3") {
        seconds = 86400;
    } else if (line == "4") {
        std::cout << "Enter minutes to advance: ";
        std::getline(std::cin, line);
        try {
            int mins = std::stoi(line);
            seconds = static_cast<std::time_t>(mins) * 60;
        } catch (...) {
            std::cout << "Invalid minutes entered.\n\n";
            return;
        }
    } else {
        std::cout << "Invalid option selected.\n\n";
        return;
    }

    if (seconds > 0) {
        system.advanceTime(seconds);
        std::cout << "Advanced clock by " << seconds << " seconds.\n";
        std::cout << "New Simulation Time: "
                  << smartcampus::service::BookingSystem::formatTime(system.getCurrentTime()) << "\n\n";
    }
}

void cliEmergencyControls(smartcampus::service::BookingSystem& system) {
    std::cout << "\n--- Campus Security Controls ---\n";
    std::cout << "  1. Declare Emergency Lockdown (locks doors, sets EmergencyHold)\n"
              << "  2. Declare All-Clear (restores venues to Available)\n"
              << "Select action [1-2]: ";

    std::string line;
    std::getline(std::cin, line);
    if (line == "1") {
        std::cout << "Enter lockdown alert reason [default: Campus Security Alert]: ";
        std::getline(std::cin, line);
        std::string reason = line.empty() ? "Campus Security Alert" : line;
        system.declareCampusEmergency(reason);
        std::cout << "EMERGENCY LOCKDOWN DECLARED. All venue doors locked.\n\n";
    } else if (line == "2") {
        system.clearCampusEmergency();
        std::cout << "ALL-CLEAR ISSUED. Facilities restored to operational status.\n\n";
    } else {
        std::cout << "Invalid action selected.\n\n";
    }
}

int runCliMode() {
    auto system = std::make_shared<smartcampus::service::BookingSystem>();

    std::cout << "\n"
              << "╔══════════════════════════════════════════════════════════════════════════════╗\n"
              << "║       SMART CAMPUS EVENT & FACILITY BOOKING SYSTEM - CLI CONSOLE             ║\n"
              << "╚══════════════════════════════════════════════════════════════════════════════╝\n";

    while (true) {
        std::cout << "Current Simulation Time: "
                  << smartcampus::service::BookingSystem::formatTime(system->getCurrentTime()) << "\n\n"
                  << "Menu:\n"
                  << "  1. Inspect Campus Facilities\n"
                  << "  2. List All Reservations\n"
                  << "  3. Create New Reservation\n"
                  << "  4. Advance Simulation Time\n"
                  << "  5. Run Demonstration Scenario (1-6 or all)\n"
                  << "  6. Campus Emergency Controls\n"
                  << "  7. Reset Simulation State\n"
                  << "  8. Exit CLI\n\n"
                  << "Enter choice [1-8]: ";

        std::string input;
        if (!std::getline(std::cin, input)) {
            std::cout << "\nEnd of input detected. Exiting CLI.\n";
            break;
        }

        if (input == "1") {
            cliInspectFacilities(*system);
        } else if (input == "2") {
            cliListReservations(*system);
        } else if (input == "3") {
            cliCreateReservation(*system);
        } else if (input == "4") {
            cliAdvanceTime(*system);
        } else if (input == "5") {
            std::cout << "\nSelect Scenario to execute [1-6, or 'all']: ";
            std::string scChoice;
            if (std::getline(std::cin, scChoice)) {
                if (scChoice.empty()) scChoice = "all";
                runScenarioMode(scChoice);
            }
        } else if (input == "6") {
            cliEmergencyControls(*system);
        } else if (input == "7") {
            system->reset();
            std::cout << "\nSimulation state reset to default seeded facilities and accounts.\n\n";
        } else if (input == "8" || input == "exit" || input == "quit" || input == "q") {
            std::cout << "\nExiting Smart Campus CLI Console. Goodbye!\n";
            break;
        } else {
            std::cout << "\nInvalid choice '" << input << "'. Please enter a number from 1 to 8.\n\n";
        }
    }

    return 0;
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    // If no arguments provided, launch the embedded web server on port 8080 by default
    if (argc <= 1) {
        std::cout << "================================================================================\n"
                  << "  SMART CAMPUS EVENT & FACILITY BOOKING SYSTEM\n"
                  << "================================================================================\n"
                  << "  No mode specified. Launching embedded web dashboard on default port 8080.\n"
                  << "  Tip: Run 'app --help' to see all available CLI options and scenario flags.\n\n";
        return runWebServer(8080);
    }

    std::string firstArg = argv[1];

    if (firstArg == "--help" || firstArg == "-h" || firstArg == "/?") {
        printHelp(argv[0]);
        return 0;
    }

    if (firstArg == "--test") {
        return runTestMode();
    }

    if (firstArg == "--cli") {
        return runCliMode();
    }

    if (firstArg == "--scenario") {
        if (argc < 3) {
            std::cerr << "Error: --scenario requires an argument: 1, 2, 3, 4, 5, 6, or 'all'.\n";
            std::cerr << "Example: " << argv[0] << " --scenario all\n";
            return 1;
        }
        return runScenarioMode(argv[2]);
    }

    if (firstArg.rfind("--scenario=", 0) == 0) {
        std::string choice = firstArg.substr(11);
        if (choice.empty()) {
            std::cerr << "Error: --scenario requires an argument: 1, 2, 3, 4, 5, 6, or 'all'.\n";
            return 1;
        }
        return runScenarioMode(choice);
    }

    if (firstArg == "--web") {
        int port = 8080;
        if (argc >= 3) {
            std::string portStr = argv[2];
            try {
                port = std::stoi(portStr);
            } catch (...) {
                std::cerr << "Error: Invalid port number '" << portStr << "'.\n";
                return 1;
            }
        }
        return runWebServer(port);
    }

    if (firstArg.rfind("--web=", 0) == 0) {
        std::string portStr = firstArg.substr(6);
        int port = 8080;
        try {
            port = std::stoi(portStr);
        } catch (...) {
            std::cerr << "Error: Invalid port number '" << portStr << "'.\n";
            return 1;
        }
        return runWebServer(port);
    }

    std::cerr << "Error: Unknown argument '" << firstArg << "'.\n";
    printHelp(argv[0]);
    return 1;
}
