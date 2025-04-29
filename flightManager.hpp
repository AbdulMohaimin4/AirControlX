#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include <vector>
#include <chrono>
#include "aircraft.hpp"
#include "runway.hpp"

struct FlightSchedule {

    Aircraft* aircraft;
    bool isArrival;
    std::chrono::system_clock::time_point scheduledTime;
};

class FlightManager {

public:
    void simulate(std::vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC);
    void simulateArrival(Aircraft* ac, Runway* rw);
    void simulateDeparture(Aircraft* ac, Runway* rw);
};

#endif