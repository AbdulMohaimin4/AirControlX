// FlightManager.cpp

#include "flightManager.hpp"

Runway rwyA(RunwayID::RWY_A, Direction::North);
Runway rwyB(RunwayID::RWY_B, Direction::East);
Runway rwyC(RunwayID::RWY_C, Direction::None);

void FlightManager::simulateArrival(Aircraft* ac) {

    std::cout << "\n[SIMULATION] Flight " << ac->id << " is ARRIVING.\n";
    ac->updatePhase(AircraftPhase::Holding); sleep(1);
    ac->updatePhase(AircraftPhase::Approach); sleep(1);
    ac->updatePhase(AircraftPhase::Landing); sleep(1);

    if (ac->airline->type == FlightType::Commercial) this->rw = &rwyA;
    else this->rw = &rwyC;
    
    rw->assign(ac); sleep(1);
    rw->release();
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); sleep(1);
    if (!ac->hasFault) ac->updatePhase(AircraftPhase::AtGate);
    std::cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}

void FlightManager::simulateDeparture(Aircraft* ac) {

    std::cout << "\n[SIMULATION] Flight " << ac->id << " is DEPARTING.\n";
    ac->updatePhase(AircraftPhase::AtGate); sleep(1);
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); sleep(1);
    if (ac->hasFault) return;

    if (ac->airline->type == FlightType::Commercial) this->rw = &rwyB;
    else this->rw = &rwyC;

    rw->assign(ac); sleep(1);
    ac->updatePhase(AircraftPhase::Takeoff); sleep(1);
    ac->updatePhase(AircraftPhase::Climb); sleep(1);
    ac->updatePhase(AircraftPhase::Cruise); sleep(1);
    rw->release();
    std::cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}
