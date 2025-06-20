#ifndef AIRLINE_HPP
#define AIRLINE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include "enums.hpp"
using namespace std;

struct AVNRecord;

class Airline {
private:
    

public:
    Airline(const string& name, FlightType type, int total, int available);
    ~Airline();

    string name;
    int priority;
    int availableFlights;
    int totalFlights;
    ofstream log_file;
    FlightType type;

    void logViolation(const AVNRecord& avn);
    string getName() const { return name; }
    FlightType getType() const { return type; }
    int getPriority() const { return priority; }

};

#endif