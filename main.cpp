#include <ctime>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include "airline.hpp"
#include "runway.hpp"
#include "aircraft.hpp"
#include "flightManager.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "atcController.hpp"
#include "avnGenerator.hpp"
#include "airlinePortal.hpp"
#include "ipcPaths.hpp"
#include "stripePay.hpp"
using namespace std;

void createNamedPipes() {

    mkfifo(ATC_TO_AVN_PIPE, 0666);
    mkfifo(AVN_TO_PORTAL_PIPE, 0666);
    mkfifo(PORTAL_TO_STRIPE_PIPE, 0666);
    mkfifo(STRIPE_TO_AVN_PIPE, 0666);
    mkfifo(STRIPE_TO_PORTAL_PIPE, 0666);
}

void cleanupPipes() {
    
    unlink(ATC_TO_AVN_PIPE);
    unlink(AVN_TO_PORTAL_PIPE);
    unlink(PORTAL_TO_STRIPE_PIPE);
    unlink(STRIPE_TO_AVN_PIPE);
    unlink(STRIPE_TO_PORTAL_PIPE);
}

void signalHandler(int signum) {

    cleanupPipes();
    exit(signum);
}

int main() {

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    srand(time(0));

    // Initializing airlines
    Airline PIA("PIA", FlightType::Commercial, 6, 4);
    Airline AirBlue("AirBlue", FlightType::Commercial, 4, 4);
    Airline FedEx("FedEx", FlightType::Cargo, 3, 2);
    Airline Pakistan_Airforce("Pakistan_Airforce", FlightType::Military, 2, 1);
    Airline BlueDart("BlueDart", FlightType::Cargo, 2, 2);
    Airline AghaKhan_Air_Ambulance("AghaKhan_Air_Ambulance", FlightType::Medical, 2, 1);

    // Initializing runways
    Runway rwyA(RunwayID::RWY_A, Direction::North);
    Runway rwyB(RunwayID::RWY_B, Direction::East);
    Runway rwyC(RunwayID::RWY_C, Direction::None);

    // Initializing all available aircrafts at the start (pre-defined)
    vector<Aircraft*> availableAircrafts = {

        new Aircraft("PK001", &PIA),
        new Aircraft("PK002", &PIA),
        new Aircraft("PK003", &PIA),
        new Aircraft("PK004", &PIA),
        new Aircraft("AB001", &AirBlue),
        new Aircraft("AB002", &AirBlue),
        new Aircraft("AB003", &AirBlue),
        new Aircraft("AB004", &AirBlue),
        new Aircraft("FDX01", &FedEx),
        new Aircraft("FDX02", &FedEx),
        new Aircraft("PAF01", &Pakistan_Airforce),
        new Aircraft("BD001", &BlueDart),
        new Aircraft("BD002", &BlueDart),
        new Aircraft("AKA01", &AghaKhan_Air_Ambulance)
    };

    FlightManager manager;
    vector<FlightSchedule> schedules;

    createNamedPipes();
    
    cout << "\n1. Access Airline Portal\n2. Continue with Simulation\nChoice: ";
    int choice;
    cin >> choice;

    if (choice == 1) {

        AirlinePortal portal;
        string aircraft_id, date;
        cout << "Enter Aircraft ID: ";
        cin >> aircraft_id;
        
        if (portal.login(aircraft_id)) {
            portal.run();
        }
        cleanupPipes();
        return 0;
    }

    // Fork processes for each requirment 
    pid_t atc_pid = fork();
    if (atc_pid == 0) {

        ATCController atc;
        atc.run();
        exit(0);
    }

    pid_t avn_pid = fork();
    if (avn_pid == 0) {

        AVNGenerator avn;
        avn.run();
        exit(0);
    }

    pid_t portal_pid = fork();
    if (portal_pid == 0) {

        AirlinePortal portal;
        portal.run();
        exit(0);
    }

    pid_t stripe_pid = fork();
    if (stripe_pid == 0) {

        StripePay stripe;
        stripe.run();
        exit(0);
    }

    // User input for flight schedules
    cout << "\nAvailable Aircrafts:\n";
    for (int i = 0; i < availableAircrafts.size(); ++i) {

        cout << i + 1 << ". " << availableAircrafts[i]->id << " (" << availableAircrafts[i]->airline->getName() << ")\n";
    }

    int numFlights;
    cout << "\nEnter the number of flights to schedule: ";
    cin >> numFlights;

    if (numFlights < 0 || numFlights > availableAircrafts.size()) {

    
        cout << "Error: Invalid number of flights. Must be between 0 and " << availableAircrafts.size() << ".\n";
        kill(atc_pid, SIGTERM);
        kill(avn_pid, SIGTERM);
        kill(portal_pid, SIGTERM);
        kill(stripe_pid, SIGTERM);
        cleanupPipes();
        return 1;
    }

    for (int i = 0; i < numFlights; ++i) {

        int aircraftIndex;
        cout << "\nSelect aircraft for flight " << i + 1 << " (1-" << availableAircrafts.size() << "): ";
        cin >> aircraftIndex;

        if (aircraftIndex < 1 || aircraftIndex > availableAircrafts.size()) {

            cout << "Error: Invalid aircraft selection.\n";
            --i;
            continue;
        }

        bool isScheduled = false;
        for (const auto& schedule : schedules) {

            if (schedule.aircraft == availableAircrafts[aircraftIndex - 1]) {
                isScheduled = true;
                break;
            }
        }

        if (isScheduled) {

            cout << "Error: Aircraft already scheduled.\n";
            --i;
            continue;
        }

        bool isArrival;
        cout << "Is this an arrival (1) or departure (0)? ";
        int input;
        cin >> input;
        isArrival = (input == 1);

        string timeInput;
        cout << "Enter scheduled time (MM:SS format, within 5 minutes, e.g., 03:30): ";
        cin >> timeInput;

        int minutes, seconds;
        // using c function 'sscanf' to make sure time inputted is correct format
        if (sscanf(timeInput.c_str(), "%d:%d", &minutes, &seconds) != 2 || 
            minutes < 0 || minutes >= 5 || seconds < 0 || seconds > 59 ||
            (minutes == 5 && seconds > 0)) {

            cout << "Error: Invalid time format. Use MM:SS (within 5 minutes).\n";
            --i;
            continue;
        }

        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm local_tm = *localtime(&now_c);
        local_tm.tm_hour = 0;
        local_tm.tm_min = minutes;
        local_tm.tm_sec = seconds;
        auto scheduledTime = chrono::system_clock::from_time_t(mktime(&local_tm));

        schedules.push_back({availableAircrafts[aircraftIndex - 1], isArrival, scheduledTime, availableAircrafts[aircraftIndex - 1]->airline->getPriority()});
    }

    cout << "\n\n\t\t*** Simulation Initializing ***\n\n";

    manager.simulate(schedules, rwyA, rwyB, rwyC);

    // Freeing memory
    for (auto aircraft : availableAircrafts) {
        delete aircraft;
    }

    cout << "\n\n\t\t*** Simulation Complete ***\n";

    // ceeling up / freeing memory
    kill(atc_pid, SIGTERM);
    kill(avn_pid, SIGTERM);
    kill(portal_pid, SIGTERM);
    kill(stripe_pid, SIGTERM);
    
    cleanupPipes();
    return 0;
}