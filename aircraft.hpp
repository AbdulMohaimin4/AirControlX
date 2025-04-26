#ifndef AIRCRAFT_HPP
#define AIRCRAFT_HPP

#include <string>
#include "enums.hpp"
#include "utils.hpp"
#include "airline.hpp"
#include "AVNLog.hpp"

class Aircraft {
    
public:
    std::string id;
    Airline* airline;
    AircraftPhase phase;
    double speed;
    bool hasFault;
    bool avnIssued;
    int violationCount;
    std::string faultType;

    Aircraft(std::string ID, Airline* al);

    void updatePhase(AircraftPhase newPhase);
    void assignSpeed();
    void checkForViolation();
    void triggerAVN(std::string reason);
    void checkGroundFault();
};

#endif
