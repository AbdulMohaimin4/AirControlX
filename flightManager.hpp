#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include <vector>
#include <chrono>
#include "aircraft.hpp"
#include "runway.hpp"
#include "utils.hpp"
using namespace std;

struct FlightSchedule {

    Aircraft* aircraft;
    bool isArrival;
    std::chrono::system_clock::time_point scheduledTime;
    int priority;
};

struct FlightProcessParams {

    FlightSchedule* schedule;
    Runway* runway;
    Runway* runway_C;
    string timeOutput;
    Aircraft* aircraft; // aircraft attempting to use the runway
};

class FlightManager {

public: 
    void simulate(std::vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC);
    void simulateArrival(Aircraft* ac, Runway* rw);
    void simulateDeparture(Aircraft* ac, Runway* rw);
    Runway* getRunwayForFlight(const FlightSchedule& schedule, Runway& rwyA, Runway& rwyB, Runway& rwyC);
    void processFlight(FlightProcessParams* params);
};

#endif