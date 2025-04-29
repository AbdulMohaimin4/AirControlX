#include "flightManager.hpp"
#include "simulationTimer.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>

// Comparison function for sorting FlightSchedule objects
bool compareFlightSchedules(const FlightSchedule& a, const FlightSchedule& b) {

    return a.scheduledTime < b.scheduledTime;
}

void FlightManager::simulate(std::vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC) {
    
    SimulationTimer timer;
    timer.start();

    // Sorting schedules by scheduled time using comparison function
    std::sort(schedules.begin(), schedules.end(), compareFlightSchedules);

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

        ostringstream oss;
        oss << setfill('0') << setw(2) << realMin << ":" << setw(2) << realSec
            << " | Sim: " << setw(2) << sim_tm->tm_hour << ":" << setw(2) << sim_tm->tm_min << ":" << setw(2) << sim_tm->tm_sec;


        Runway* rw = nullptr; // points to already instantiated Runway objects in main

        // choosing runway based on whether flight is arrival/departure
        if (schedule.isArrival) {

            // by default RWY-C only accomodates Cargo flights (can be used as backup for multiple high-priority arrivals/departures)
            rw = (schedule.aircraft->airline->type == FlightType::Commercial || 
                schedule.aircraft->airline->type == FlightType::Medical ||
                schedule.aircraft->airline->type == FlightType::Military) ? &rwyA : &rwyC;

            cout << "[TIME] Real: " << oss.str() << "\n";

            // using runway info struct to pass info into thread func
            RunwayInfo* info = new RunwayInfo;
            info->isArrival = schedule.isArrival;
            info->priority = schedule.aircraft->airline->priority;
            info->runway = rw;
            info->runway_C = &rwyC; // incase high priority and A/B are locked

            schedule.aircraft->checkRunway(info); // waiting for runway to be free
            simulateArrival(schedule.aircraft, rw); // assignment of runway
            pthread_join(schedule.aircraft->aircraft_thread, NULL); // joining thread after done

            cout << "[TIME] Real: " << oss.str() << "\n";

        } 
        else {

            rw = (schedule.aircraft->airline->type == FlightType::Commercial || 
                schedule.aircraft->airline->type == FlightType::Medical ||
                schedule.aircraft->airline->type == FlightType::Military) ? &rwyB : &rwyC;

            cout << "[TIME] Real: " << oss.str() << "\n";

            // using runway info struct to pass info into thread func
            RunwayInfo* info = new RunwayInfo;
            info->isArrival = schedule.isArrival;
            info->priority = schedule.aircraft->airline->priority;
            info->runway = rw;
            info->runway_C = &rwyC; // incase high priority and A/B are locked

            schedule.aircraft->checkRunway(info); // waiting for runway to be free
            simulateDeparture(schedule.aircraft, rw); // assignment of runway
            pthread_join(schedule.aircraft->aircraft_thread, NULL); // joining thread after done

            cout << "[TIME] Real: " << oss.str() << "\n";

        }
    }

    // Continue running until 5 minutes real-time
    while (timer.getRealTimeElapsed() < std::chrono::minutes(5)) {

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // exiting loop if all scheduled flights are done
        bool flag = 1;
        for (const auto& schedule : schedules) {

            if (!schedule.aircraft->isDone) flag = 0;
        }
        if (flag) break;
    }
}

void FlightManager::simulateArrival(Aircraft* ac, Runway* rw) {

    SimulationTimer timer; // For time output in events
    cout << "\n[SIMULATION] Flight " << ac->id << " is ARRIVING.\n";

    // sleeping to simulate real time delay (edit later to match animation delay for each phase)
    ac->updatePhase(AircraftPhase::Holding); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Approach); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Landing); std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // runway assigned after landing
    rw->assign(ac); std::this_thread::sleep_for(std::chrono::seconds(1));
    rw->release();
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!ac->hasFault) {

        ac->updatePhase(AircraftPhase::AtGate);
        ac->isDone = true;
    }
    ac->isDone = true;
    cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}

void FlightManager::simulateDeparture(Aircraft* ac, Runway* rw) {

    SimulationTimer timer; // For time output in events
    cout << "\n[SIMULATION] Flight " << ac->id << " is DEPARTING.\n";
    ac->updatePhase(AircraftPhase::AtGate); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); std::this_thread::sleep_for(std::chrono::seconds(1));
    if (ac->hasFault) return;

    rw->assign(ac); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Takeoff); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Climb); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Cruise); std::this_thread::sleep_for(std::chrono::seconds(1));
    ac->isDone = true;
    rw->release();
    cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}