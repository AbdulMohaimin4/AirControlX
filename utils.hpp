#ifndef UTILS_HPP
#define UTILS_HPP

#include "enums.hpp"
#include <string>

// forward declarations
class Runway;
class Aircraft;

// struct to pass into thread func so the func can decide runway locking itself
struct RunwayInfo {

    Runway* runway;
    Runway* runway_C;
    bool isArrival;
    int priority;
    Runway* assignedRunway; // Store the runway actually used
    Aircraft* aircraft; // aircraft attempting to use the runway
};

std::string toString(FlightType type);
std::string toString(AircraftPhase phase);


#endif
