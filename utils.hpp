#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <pthread.h>
#include <chrono>
#include "enums.hpp"
using namespace std;

// forward declarations
class Runway;
class Aircraft;

// struct to pass into thread func so the func can decide runway locking itself (declared here to avoid circular dep.)
struct RunwayInfo {

    Runway* runway;
    Runway* runway_C;
    bool isArrival;
    int priority;
    Runway* assignedRunway; // Store the runway actually used
    chrono::system_clock::time_point queueStartTime; // to track when aircraft is enqueued
};


string toString(FlightType type);
string toString(AircraftPhase phase);


#endif
