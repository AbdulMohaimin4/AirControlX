#ifndef AIRCRAFT_HPP
#define AIRCRAFT_HPP

#include <string>
#include <pthread.h>
#include "enums.hpp"
#include "utils.hpp"
#include "airline.hpp"
#include "AVNLog.hpp"
using namespace std;

class Aircraft {
    
public:
    string id;
    pthread_t aircraft_thread; // aircraft's personal thread
    Airline* airline;
    AircraftPhase phase;
    double speed;
    bool hasFault;
    bool avnIssued;
    int violationCount;
    string faultType;
    bool isDone; // sucessfully at gate/cruise

    Aircraft(std::string ID, Airline* al);

    void checkRunway(RunwayInfo* runway); // locks/unlocks/waits for runway mutex
    void updatePhase(AircraftPhase newPhase);
    void assignSpeed();
    void checkForViolation();
    void triggerAVN(std::string reason);
    void checkGroundFault();
};


#endif
