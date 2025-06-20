#ifndef AIRCRAFT_HPP
#define AIRCRAFT_HPP

#include <iostream>
#include <string>
#include <pthread.h>
#include "enums.hpp"
#include "utils.hpp"
#include "airline.hpp"
#include "AVNLog.hpp"
#include "avnGenerator.hpp"
using namespace std;

class Aircraft {
    
public:
    string id;
    pthread_t aircraft_thread; // aircraft's personal thread for mutex management
    Airline* airline;
    AircraftPhase phase;
    double speed;
    bool hasFault;
    bool avnIssued;
    int violationCount;
    bool violationStatus; // module 3
    string faultType;
    bool isDone; // sucessfully at gate/cruise
    int waitTimeSeconds; // for storing wait time

    double current_speed;
    double current_altitude;
    bool has_active_violation;

    Aircraft(string ID, Airline* al);

    //void checkRunway(RunwayInfo* runway); // locks/unlocks/waits for runway mutex
    void updatePhase(AircraftPhase newPhase);
    void assignSpeed();
    void checkForViolation();
    void triggerAVN(string reason);
    void checkGroundFault();

    void updateFlightParameters(double speed, double altitude);
    bool hasActiveViolation() const { return has_active_violation; }
    void setViolationStatus(bool status) { has_active_violation = status; }
    double getCurrentSpeed() const { return current_speed; }
    double getCurrentAltitude() const { return current_altitude; }
};


#endif
