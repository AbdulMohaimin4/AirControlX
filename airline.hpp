#ifndef AIRLINE_HPP
#define AIRLINE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include "enums.hpp"

class Airline {

private:
    std::string fileName; // log file name
    std::ofstream logFile; // file stream for entering logs/bills
public:
    std::string name;
    FlightType type;
    int priority; // higher priority = higher value (for max-heap implementation)
    int aircraftCount;
    int activeFlights;

    Airline(std::string n, FlightType t, int total, int active);
    ~Airline();
    
    // logging Airspace Violation Notice (AVN) bills
    void logViolation(const std::string& message);
};

#endif
