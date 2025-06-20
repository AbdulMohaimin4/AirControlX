#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include "aircraft.hpp"
#include "runway.hpp"
#include "utils.hpp"
#include "atcController.hpp"
using namespace std;

struct FlightSchedule {

    Aircraft* aircraft;
    bool isArrival;
    chrono::system_clock::time_point scheduledTime;
    int priority;
};

struct FlightProcessParams {

    FlightSchedule* schedule;
    Runway* runway;
    Runway* runway_C;
    string timeOutput;
    int waitTimeSeconds; // to store the waiting time of aircraft for final times output
};

class FlightManager {

private:
    ATCController* atc;

public: 
    FlightManager() : atc(new ATCController()) {}
    ~FlightManager() { delete atc; }
    void simulate(std::vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC);
    //void simulateArrival(Aircraft* ac, Runway* rw, int waitTimeSeconds);
    //void simulateDeparture(Aircraft* ac, Runway* rw, int waitTimeSeconds);
    Runway* getRunwayForFlight(const FlightSchedule& schedule, Runway& rwyA, Runway& rwyB, Runway& rwyC);
    void processFlight(FlightProcessParams* params);
    void checkRunway(RunwayInfo* runway);

    void monitorFlight(Aircraft* aircraft) {
        atc->monitorAircraft(aircraft->id, aircraft->getCurrentSpeed(), aircraft->getCurrentAltitude());
    }
};

#endif