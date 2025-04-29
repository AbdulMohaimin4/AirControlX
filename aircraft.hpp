#ifndef AIRCRAFT_HPP
#define AIRCRAFT_HPP

#include <string>
#include "enums.hpp"
#include "utils.hpp"
#include "airline.hpp"
#include "AVNLog.hpp"
using namespace std;

class Aircraft {
    
public:
    string id;
    Airline* airline;
    AircraftPhase phase;
    double speed;
    bool hasFault;
    bool avnIssued;
    int violationCount;
    string faultType;
    bool isDone; // sucessfully at gate/cruise

    Aircraft(std::string ID, Airline* al);

    void updatePhase(AircraftPhase newPhase);
    void assignSpeed();
    void checkForViolation();
    void triggerAVN(std::string reason);
    void checkGroundFault();
};

#endif
