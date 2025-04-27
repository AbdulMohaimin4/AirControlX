#include "flightManager.hpp"
#include "simulationTimer.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <ctime>

void FlightManager::simulate(std::vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC) {
    SimulationTimer timer;
    timer.start();

    // Sort schedules by scheduled time
    std::sort(schedules.begin(), schedules.end(), 
        [](const FlightSchedule& a, const FlightSchedule& b) {
            return a.scheduledTime < b.scheduledTime;
        });

    for (const auto& schedule : schedules) {
        // Wait until simulated time reaches scheduled time
        while (timer.getSimulatedTime() < schedule.scheduledTime) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Format times for output
        auto realTimeSec = timer.getRealTimeElapsed().count();
        int realMin = realTimeSec / 60;
        int realSec = realTimeSec % 60;
        auto simTime = timer.getSimulatedTime();
        time_t simTime_t = std::chrono::system_clock::to_time_t(simTime);
        tm* sim_tm = localtime(&simTime_t);
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << realMin << ":" << std::setw(2) << realSec
            << " | Sim: " << std::setw(2) << sim_tm->tm_hour << ":" << std::setw(2) << sim_tm->tm_min << ":" << std::setw(2) << sim_tm->tm_sec;

        Runway* rw = nullptr;
        if (schedule.isArrival) {
            rw = (schedule.aircraft->airline->type == FlightType::Commercial) ? &rwyA : &rwyC;
            std::cout << "[TIME] Real: " << oss.str() << "\n";
            simulateArrival(schedule.aircraft, rw);
        } else {
            rw = (schedule.aircraft->airline->type == FlightType::Commercial) ? &rwyB : &rwyC;
            std::cout << "[TIME] Real: " << oss.str() << "\n";
            simulateDeparture(schedule.aircraft, rw);
        }
    }

    // Continue running until 5 minutes real-time
    while (timer.getRealTimeElapsed() < std::chrono::minutes(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void FlightManager::simulateArrival(Aircraft* ac, Runway* rw) {
    SimulationTimer timer; // For time output in events
    std::cout << "\n[SIMULATION] Flight " << ac->id << " is ARRIVING.\n";
    ac->updatePhase(AircraftPhase::Holding); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Approach); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Landing); std::this_thread::sleep_for(std::chrono::seconds(1));
    
    rw->assign(ac); std::this_thread::sleep_for(std::chrono::seconds(1));
    rw->release();
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!ac->hasFault) ac->updatePhase(AircraftPhase::AtGate);
    std::cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}

void FlightManager::simulateDeparture(Aircraft* ac, Runway* rw) {
    SimulationTimer timer; // For time output in events
    std::cout << "\n[SIMULATION] Flight " << ac->id << " is DEPARTING.\n";
    ac->updatePhase(AircraftPhase::AtGate); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); std::this_thread::sleep_for(std::chrono::seconds(1));
    if (ac->hasFault) return;

    rw->assign(ac); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Takeoff); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Climb); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Cruise); std::this_thread::sleep_for(std::chrono::seconds(1));
    rw->release();
    std::cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}