#include "aircraft.hpp"
#include "runway.hpp"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <ctime>

Aircraft::Aircraft(string ID, Airline* al)
    : id(ID), airline(al), phase(AircraftPhase::AtGate),
      speed(0.0), hasFault(false), avnIssued(false),
      violationCount(0), violationStatus(false), faultType(""), isDone(0) {}

// update method the set new phase for arrival/departure and check violations
void Aircraft::updatePhase(AircraftPhase newPhase) {

    phase = newPhase;
    this->assignSpeed();
    this->checkForViolation();
    cout << "[STATUS] " << this->id << " updated to " << toString(phase) << " | Speed: " << this->speed << " km/h\n";
}


void Aircraft::assignSpeed() {

    // 10% chance set of violation occuring using roll variable
    int roll = rand() % 100;
    switch (phase) { 
        case AircraftPhase::Holding:
            speed = (roll < 90) ? 400 + rand() % 201 : 601 + rand() % 50;
            break;
        case AircraftPhase::Approach:
            speed = (roll < 90) ? 240 + rand() % 51 : (rand() % 2 ? 230 : 295);
            break;
        case AircraftPhase::Landing:
            speed = (roll < 90) ? 30 + rand() % 211 : (rand() % 2 ? 25 : 260);
            break;
        case AircraftPhase::Taxi:
            speed = (roll < 90) ? 15 + rand() % 16 : 35 + rand() % 20;
            break;
        case AircraftPhase::AtGate:
            speed = (roll < 90) ? 0 : 11 + rand() % 5;
            break;
        case AircraftPhase::Takeoff:
            speed = (roll < 90) ? 0 + rand() % 291 : 291 + rand() % 50;
            break;
        case AircraftPhase::Climb:
            speed = (roll < 90) ? 250 + rand() % 214 : 464 + rand() % 30;
            break;
        case AircraftPhase::Cruise:
            speed = (roll < 90) ? 800 + rand() % 101 : (rand() % 2 ? 790 : 910);
            break;
    }
}

void Aircraft::checkForViolation() {

    // triggering AVN for violations
    switch (phase) {

        case AircraftPhase::Holding: if (speed > 600) triggerAVN("Speed exceeds 600 (too fast in holding)"); break;
        case AircraftPhase::Approach: if (speed < 240 || speed > 290) triggerAVN("Approach speed out of range (240-290)"); break;
        case AircraftPhase::Landing: if (speed > 240 || speed < 30) triggerAVN("Landing speed not decelerated properly"); break;
        case AircraftPhase::Taxi: if (speed > 30) triggerAVN("Taxi speed exceeds 30"); break;
        case AircraftPhase::AtGate: if (speed > 10) triggerAVN("Gate speed exceeds 10 (should be stationary)"); break;
        case AircraftPhase::Takeoff: if (speed > 290) triggerAVN("Takeoff speed exceeds 290 before lift"); break;
        case AircraftPhase::Climb: if (speed > 463) triggerAVN("Climb speed exceeds 463 (max under 10,000 ft)"); break;
        case AircraftPhase::Cruise: if (speed < 800 || speed > 900) triggerAVN("Cruise speed out of bounds (800-900)"); break;
    }
}

void Aircraft::triggerAVN(string reason) {

    if (!this->violationStatus) this->violationStatus = true;

    auto now = chrono::system_clock::now();
    time_t now_t = chrono::system_clock::to_time_t(now);

    // Create AVN record
    AVNRecord avn;
    avn.avn_id = "AVN" + to_string(violationCount + 1);
    avn.airline_name = airline->getName();
    avn.aircraft_id = id;
    avn.aircraft_type = airline->getType();
    avn.recorded_speed = speed;
    avn.issue_date = now_t;
    avn.due_date = now_t + (3 * 24 * 60 * 60); // due date 3 days from now

    if (airline->getType() == FlightType::Commercial) avn.fine_amount = 500000.0;
    else avn.fine_amount = 700000.0;
    avn.service_fee = avn.fine_amount * 0.15;
    avn.paid = false;

    // Log the violation through airline
    this->airline->logViolation(avn);

    // Also log to AVN system
    AVNLog::issueAVN(this->id, reason, this->speed, this->phase);
    
    this->avnIssued = true;
    this->violationCount++;
}

void Aircraft::checkGroundFault() {

    if ((this->phase == AircraftPhase::AtGate || this->phase == AircraftPhase::Taxi) && (rand() % 100 < 15)) {
        this->hasFault = true;
        this->faultType = (rand() % 2 == 0) ? "Brake Failure" : "Hydraulic Leak";
        cout << "[FAULT] " << id << " encountered: " << this->faultType << "\n";
    }
}

void Aircraft::updateFlightParameters(double speed, double altitude) {
    
    current_speed = speed;
    current_altitude = altitude;
    
    // Random speed/altitude variations for simulation
    if (rand() % 100 < 10) {  // 10% chance of speed violation
        current_speed += 100 + (rand() % 200);
    }
    if (rand() % 100 < 5) {   // 5% chance of altitude violation
        current_altitude += 1000 + (rand() % 2000);
    }
}