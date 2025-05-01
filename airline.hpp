#ifndef AIRLINE_HPP
#define AIRLINE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include "enums.hpp"
using namespace std;

class Airline {

private:
    string fileName; // log file name
    ofstream logFile; // file stream for entering logs/bills
public:
    string name;
    FlightType type;
    int priority; // higher priority = higher value (for max-heap implementation)
    int aircraftCount;
    int activeFlights;

    Airline(string n, FlightType t, int total, int active);
    ~Airline();
    
    // logging Airspace Violation Notice (AVN) bills
    void logViolation(const string& message);
};

#endif
