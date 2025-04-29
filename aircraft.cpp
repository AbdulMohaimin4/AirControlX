#include "aircraft.hpp"
#include "runway.hpp"
#include <cstdlib>
#include <iostream>
#include "simulationTimer.hpp"
#include <iomanip>
#include <chrono>
#include <thread>
#include <ctime>

Aircraft::Aircraft(std::string ID, Airline* al)
    : id(ID), airline(al), phase(AircraftPhase::AtGate), speed(0.0), hasFault(false), avnIssued(false), violationCount(0), faultType(""), isDone(0) {}


// thread function allowing aircraft to lock/unlock runways
// static as implicit this* pointer isn't allowed in thread_fun
static void* thread_fun(void* arg) {

    Runway* runway = (Runway*)arg;

    cout << runway->getID() << "...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    pthread_mutex_lock(&runway->runway_mutex);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    pthread_mutex_unlock(&runway->runway_mutex);
    runway->getID() == "RWY-B" ? 
    cout << "\nAircraft cleared to depart\n" :
    runway->getID() == "RWY-A" ? 
        cout << "\nAircraft cleared to land\n" : 
        cout << "\nRunway C clear to proceed\n";

    pthread_exit(NULL);
}

// called in FlightManager to wait until runway is unoccupied
void Aircraft::checkRunway(Runway* runway) {

    cout << "\n\n" <<this->airline->name << " flight "<< this->id << " requesting runway ";
    pthread_create(&this->aircraft_thread, NULL, thread_fun, (void*)runway);
}

void Aircraft::updatePhase(AircraftPhase newPhase) {

    phase = newPhase;
    this->assignSpeed();
    this->checkForViolation();
}

void Aircraft::assignSpeed() {

    int roll = rand() % 100;

    // 10% chance of violation set in each phase
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

    switch (phase) {

        case AircraftPhase::Holding: if (speed > 600) triggerAVN("Speed exceeds 600 (too fast in holding)"); break;
        case AircraftPhase::Approach: if (speed < 240 || speed > 290) triggerAVN("Approach speed out of range (240–290)"); break;
        case AircraftPhase::Landing: if (speed > 240 || speed < 30) triggerAVN("Landing speed not decelerated properly"); break;
        case AircraftPhase::Taxi: if (speed > 30) triggerAVN("Taxi speed exceeds 30"); break;
        case AircraftPhase::AtGate: if (speed > 10) triggerAVN("Gate speed exceeds 10 (should be stationary)"); break;
        case AircraftPhase::Takeoff: if (speed > 290) triggerAVN("Takeoff speed exceeds 290 before lift"); break;
        case AircraftPhase::Climb: if (speed > 463) triggerAVN("Climb speed exceeds 463 (max under 10,000 ft)"); break;
        case AircraftPhase::Cruise: if (speed < 800 || speed > 900) triggerAVN("Cruise speed out of bounds (800–900)"); break;
    }
}

void Aircraft::triggerAVN(std::string reason) {

    SimulationTimer timer;

    // Formatting times for logging
    auto realTimeSec = timer.getRealTimeElapsed().count();
    int realMin = realTimeSec / 60;
    int realSec = realTimeSec % 60;
    auto simTime = timer.getSimulatedTime();
    time_t simTime_t = std::chrono::system_clock::to_time_t(simTime);
    tm* sim_tm = localtime(&simTime_t);

    std::ostringstream oss; // to store time information for logging
    oss << std::setfill('0') << std::setw(2) << realMin << ":" << std::setw(2) << realSec
        << " | Sim: " << std::setw(2) << sim_tm->tm_hour << ":" << std::setw(2) << sim_tm->tm_min << ":" << std::setw(2) << sim_tm->tm_sec;

    const std::string violation = "[AVN] Violation: " + id + " | Phase: " + toString(phase) + " | Speed: " + std::to_string(speed) + " km/h | Reason: " + reason + " | Time: " + oss.str() + "\n";
    this->airline->logViolation(violation);
    AVNLog::issueAVN(id, reason, speed, phase, oss.str());
    avnIssued = true;
    violationCount++;
}

void Aircraft::checkGroundFault() {

    if ((phase == AircraftPhase::AtGate || phase == AircraftPhase::Taxi) && (rand() % 100 < 15)) {

        hasFault = true;
        faultType = (rand() % 2 == 0) ? "Brake Failure" : "Hydraulic Leak";
        std::cout << "[FAULT] " << id << " encountered: " << faultType << "\n";
    }
}