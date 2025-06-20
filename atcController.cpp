#include "atcController.hpp"
#include "ipcPaths.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void signalHandler(int signum) {

    keep_running = 0;
}

ATCController::ATCController() {

    write_fd = open(ATC_TO_AVN_PIPE, O_WRONLY | O_NONBLOCK);
    active_violations = 0;
}

ATCController::~ATCController() {

    close(write_fd);
}

void ATCController::monitorAircraft(const string& aircraft_id, double speed, double altitude) {

    const double SPEED_LIMIT = 900.0; // km/h
    const double MAX_ALTITUDE = 41000.0; // feet

    bool has_violation = false;
    string violation_type;

    if (speed > SPEED_LIMIT) {

        has_violation = true;
        violation_type = "Speed";
    }
    if (altitude > MAX_ALTITUDE) {

        has_violation = true;
        violation_type += has_violation ? ", Altitude" : "Altitude";
    }

    if (has_violation) {

        ViolationRecord record {aircraft_id, speed, altitude, violation_type, true, time(nullptr)};
        
        if (violations.find(aircraft_id) == violations.end()) {

            active_violations++;
        }
        violations[aircraft_id] = record;

        stringstream ss;
        ss << aircraft_id << "," << speed << "," << altitude << "," << violation_type;
        write(write_fd, ss.str().c_str(), ss.str().length());
    }
}

void ATCController::displayDashboard() const {
    //cout << "\n=== ATC Dashboard ===\n";
    //cout << "Active Violations: " << active_violations << "\n\n";
    
    for (const auto& [id, record] : violations) {

        if (record.active) {

            cout << "Aircraft: " << id << "\n"
                     << "Violation: " << record.violation_type << "\n"
                     << "Speed: " << record.speed << " km/h\n"
                     << "Altitude: " << record.altitude << " feet\n"
                     << "Detected: " << put_time(localtime(&record.detected_time), "%H:%M:%S")
                     << "\n-------------------\n";
        }
    }
}

void ATCController::run() {

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    while (keep_running) {
        
        displayDashboard();
        usleep(5000000); // 5 seconds
    }
}