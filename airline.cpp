// Airline.cpp

#include "airline.hpp"
#include "utils.hpp"

Airline::Airline(std::string n, FlightType t, int total, int active)
    : name(n), type(t), aircraftCount(total), activeFlights(active) {

        this->fileName = n + "_logFile";
        this->logFile.open(this->fileName, std::ios::app); // opening in append mode

        if (!this->logFile.is_open()) 
            std::cout << "Error: could not create/open file '" << this->fileName << "'\n";

    }

Airline::~Airline() {

    if (this->logFile.is_open()) this->logFile.close();
}

void Airline::logViolation(const std::string& message) {

    if (this->logFile.is_open()) this->logFile << message << "\n";
    else std::cout << "Error: Log file not open\n";
}