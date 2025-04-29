// Airline.cpp

#include "airline.hpp"
#include "utils.hpp"

Airline::Airline(std::string n, FlightType t, int total, int active)
    : name(n), type(t), aircraftCount(total), activeFlights(active) {

        this->fileName = n + "_logFile";
        this->logFile.open(this->fileName, std::ios::app); // opening in append mode

        if (!this->logFile.is_open()) 
            std::cout << "Error: could not create/open file '" << this->fileName << "'\n";

        switch (this->type) {

            case FlightType::Cargo:
                this->priority = 2;
                break;
            case FlightType::Commercial:
                this->priority = 1;
                break;
            case FlightType::Medical:
                this->priority = 4;     // priority = 4 is maximum (emergency) and can be set for other flights allowing usage of RWY-C
                break;
            case FlightType::Military:
                this->priority = 3;
                break;
        }

    }

Airline::~Airline() {

    if (this->logFile.is_open()) this->logFile.close();
}

void Airline::logViolation(const std::string& message) {

    if (this->logFile.is_open()) this->logFile << message << "\n";
    else std::cout << "Error: Log file not open\n";
}