#include <ctime>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "airline.hpp"
#include "runway.hpp"
#include "aircraft.hpp"
#include "flightManager.hpp"
using namespace std;

int main() {

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



    // User input for flight schedules
    cout << "\nAvailable Aircrafts:\n";
    for (int i = 0; i < availableAircrafts.size(); ++i) {

        cout << i + 1 << ". " << availableAircrafts[i]->id << " (" << availableAircrafts[i]->airline->name << ")\n";
    }

    int numFlights;
    cout << "\nEnter the number of flights to schedule: ";
    cin >> numFlights;

    if (numFlights < 0 || numFlights > availableAircrafts.size()) {

        cout << "Error: Invalid number of flights. Must be between 0 and " << availableAircrafts.size() << ".\n";
        return 1;
    }

    // loop for user input of aircraft selection
    for (int i = 0; i < numFlights; ++i) {

        int aircraftIndex;
        cout << "\nSelect aircraft for flight " << i + 1 << " (1-" << availableAircrafts.size() << "): ";
        cin >> aircraftIndex;

        if (aircraftIndex < 1 || aircraftIndex > availableAircrafts.size()) {

            cout << "Error: Invalid aircraft selection.\n";
            --i;
            continue;
        }

        // check if aircraft is already scheduled
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
        isArrival = (input == 1); // to make sure 1 or 0 (error handling)

        string timeInput;
        cout << "Enter scheduled time (HH:MM, 24-hour format, e.g., 14:30): ";
        cin >> timeInput;

        // Parsing time input and handling error using 'sscanf'
        int hours, minutes;
        if (sscanf(timeInput.c_str(), "%d:%d", &hours, &minutes) != 2 || hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {

            cout << "Error: Invalid time format. Use HH:MM (24-hour).\n";
            --i;
            continue;
        }

        // Creating a time_point for scheduled time (assuming today's date) beyond my understanding
        auto now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);
        tm local_tm = *localtime(&now_c);
        local_tm.tm_hour = hours;
        local_tm.tm_min = minutes;
        local_tm.tm_sec = 0;
        auto scheduledTime = chrono::system_clock::from_time_t(mktime(&local_tm));

        schedules.push_back({availableAircrafts[aircraftIndex - 1], isArrival, scheduledTime}); // pushing aircraft into schedule vector
    }

    cout << "\n\n\t\t*** Simulation Initializing ***\n\n";

    manager.simulate(schedules, rwyA, rwyB, rwyC);

    // Freeing memory
    for (auto aircraft : availableAircrafts) {

        delete aircraft;
    }

    cout << "\n\n\t\t*** Simulation Complete ***\n";


    return 0;
}