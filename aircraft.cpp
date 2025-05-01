#include "aircraft.hpp"
#include "runway.hpp"
#include <cstdlib>
#include <iostream>
#include "simulationTimer.hpp"
#include <iomanip>
#include <chrono>
#include <thread>
#include <ctime>

Aircraft::Aircraft(string ID, Airline* al)
    : id(ID), airline(al), phase(AircraftPhase::AtGate),
     speed(0.0), hasFault(false), avnIssued(false),
      violationCount(0), faultType(""), isDone(0) {}


// thread function allowing aircraft to lock/unlock runways
// static as implicit this* pointer isn't allowed in thread_fun
static void* thread_fun(void* arg) {

    RunwayInfo* runwayInfo = (RunwayInfo*)arg;

    //pthread_mutex_lock(&cout_mutex);
    cout << runwayInfo->runway->getID() << "...\n";
    //pthread_mutex_unlock(&cout_mutex);

    this_thread::sleep_for(chrono::milliseconds(1000));

    // recording queue start time
    runwayInfo->queueStartTime = chrono::system_clock::now();

    // Normal/low priority (not priority = 4)
    if (runwayInfo->priority != 4) {

        pthread_mutex_lock(&runwayInfo->runway->runway_mutex);
        runwayInfo->assignedRunway = runwayInfo->runway; // Assign the primary runway
        this_thread::sleep_for(chrono::milliseconds(1000));
        pthread_mutex_unlock(&runwayInfo->runway->runway_mutex);    
    }

    // Top priority: attempt checking RWY-C if A/B not available
    else {
        if (pthread_mutex_trylock(&runwayInfo->runway->runway_mutex) == 0) {

            runwayInfo->assignedRunway = runwayInfo->runway; // Assign primary runway
            this_thread::sleep_for(chrono::milliseconds(1000));
            pthread_mutex_unlock(&runwayInfo->runway->runway_mutex);
        } 
        else {

            if (pthread_mutex_trylock(&runwayInfo->runway_C->runway_mutex) == 0) {

                runwayInfo->assignedRunway = runwayInfo->runway_C; // Assign Runway C
                this_thread::sleep_for(chrono::milliseconds(1000));
                pthread_mutex_unlock(&runwayInfo->runway_C->runway_mutex);
            } 
            else {

                // If both runways are locked, wait for the primary runway
                // pthread_mutex_lock(&runwayInfo->runway->runway_mutex);
                // runwayInfo->assignedRunway = runwayInfo->runway;
                // this_thread::sleep_for(chrono::milliseconds(1000));
                // pthread_mutex_unlock(&runwayInfo->runway->runway_mutex);
            }
        }
    }

    // thread safe outputting
    cout << "\nAircraft cleared to " << (runwayInfo->assignedRunway->getID() == "RWY-B" ? "depart" :
        runwayInfo->assignedRunway->getID() == "RWY-A" ? "land" : "proceed on Runway C") << "\n";

    pthread_exit(NULL);
}


// called in FlightManager to wait until runway is unoccupied
void Aircraft::checkRunway(RunwayInfo* runwayInfo) {

    cout << "\n\n" <<this->airline->name << " flight "<< this->id << " requesting runway ";

    pthread_create(&this->aircraft_thread, NULL, thread_fun, (void*)runwayInfo);
    pthread_join(this->aircraft_thread, NULL);

    runwayInfo->runway = runwayInfo->assignedRunway; // assigning locked runway to aircraft
}

void Aircraft::updatePhase(AircraftPhase newPhase) {

    phase = newPhase;
    this->assignSpeed();
    this->checkForViolation();

    // Thread safe status update
    cout << "[STATUS] " << id << " updated to " << toString(phase) << " | Speed: " << speed << " km/h\n";
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

void Aircraft::triggerAVN(string reason) {

    SimulationTimer timer;

    // Formatting times for logging
    auto realTimeSec = timer.getRealTimeElapsed().count();
    int realMin = realTimeSec / 60;
    int realSec = realTimeSec % 60;
    auto simTime = timer.getSimulatedTime();
    time_t simTime_t = chrono::system_clock::to_time_t(simTime);
    tm* sim_tm = localtime(&simTime_t);

    ostringstream oss; // to store time information for logging
    oss << setfill('0') << setw(2) << realMin << ":" << setw(2) << realSec
        << " | Sim: " << setw(2) << sim_tm->tm_hour << ":" << setw(2) << sim_tm->tm_min << ":" << setw(2) << sim_tm->tm_sec;

    const string violation = "[AVN] Violation: " + id + " | Phase: " + toString(phase) + " | Speed: " + to_string(speed) + " km/h | Reason: " + reason + " | Time: " + oss.str() + "\n";
    this->airline->logViolation(violation);
    AVNLog::issueAVN(id, reason, speed, phase, oss.str());
    avnIssued = true;
    violationCount++; // for later usage
}

void Aircraft::checkGroundFault() {

    // 15% chance of fault during taxi or at gate
    if ((this->phase == AircraftPhase::AtGate || this->phase == AircraftPhase::Taxi) && (rand() % 100 < 15)) {

        this->hasFault = true;
        this->faultType = (rand() % 2 == 0) ? "Brake Failure" : "Hydraulic Leak"; // example options
        
        cout << "[FAULT] " << id << " encountered: " << this->faultType << "\n";
    }
}