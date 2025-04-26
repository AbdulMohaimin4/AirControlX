#include <ctime>
#include <cstdlib>
#include <iostream>
#include "airline.hpp"
#include "runway.hpp"
#include "aircraft.hpp"
#include "flightManager.hpp"

int main() {

    srand(time(0));

    Airline PIA("PIA", FlightType::Commercial, 6, 4);
    Airline FedEx("FedEx", FlightType::Cargo, 3, 2);
    Airline PAF("PAF", FlightType::Military, 2, 1);

    Runway rwyA(RunwayID::RWY_A, Direction::North);
    Runway rwyB(RunwayID::RWY_B, Direction::East);
    Runway rwyC(RunwayID::RWY_C, Direction::None);

    Aircraft ac1("PK001", &PIA);
    Aircraft ac2("FDX01", &FedEx);
    Aircraft ac3("PAF01", &PAF);
    Aircraft ac4("PK002", &PIA);

    FlightManager manager;

    std::cout << "\n\n\t\t*** Simulation initializing ***\n\n";
    manager.simulateArrival(&ac1);
    manager.simulateArrival(&ac2);
    manager.simulateDeparture(&ac3);
    manager.simulateDeparture(&ac4);
    std::cout << "\n\n\t\t*** Simulation Complete ***\n";

    return 0;
}
