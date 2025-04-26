#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include "aircraft.hpp"
#include "runway.hpp"
#include <unistd.h>
#include <iostream>

class FlightManager {

public:
    
    Runway* rw;
    void simulateArrival(Aircraft* ac);
    void simulateDeparture(Aircraft* ac);
};

#endif
