#include "flightManager.hpp"
#include "simulationTimer.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <vector>

// Comparison function for sorting FlightSchedule objects
bool compareFlightSchedules(const FlightSchedule& a, const FlightSchedule& b) {

    if (a.scheduledTime == b.scheduledTime) {
        // If scheduled times are equal, sort by priority
        return a.priority > b.priority; 
    }
    return a.scheduledTime < b.scheduledTime; // Earlier scheduled time first
}

Runway* FlightManager::getRunwayForFlight(const FlightSchedule& schedule, Runway& rwyA, Runway& rwyB, Runway& rwyC) {

    // determines which runway to use based on flight type and availability
    if (schedule.isArrival) {

        return (schedule.aircraft->airline->type == FlightType::Commercial || 
                schedule.aircraft->airline->type == FlightType::Medical ||
                schedule.aircraft->airline->type == FlightType::Military) ? &rwyA : &rwyC;
    } else {

        return (schedule.aircraft->airline->type == FlightType::Commercial || 
                schedule.aircraft->airline->type == FlightType::Medical ||
                schedule.aircraft->airline->type == FlightType::Military) ? &rwyB : &rwyC;
    }
}

void FlightManager::processFlight(FlightProcessParams* params) {

    RunwayInfo* info = new RunwayInfo;
    info->isArrival = params->schedule->isArrival;
    info->priority = params->schedule->aircraft->airline->priority;
    info->runway = params->runway;
    info->runway_C = params->runway_C; // incase high priority and A/B are locked

    params->schedule->aircraft->checkRunway(info); // waiting for runway to be free
    if (params->schedule->isArrival) {

        this->simulateArrival(params->schedule->aircraft, info->runway); // assignment of runway
    }
    else {

        this->simulateDeparture(params->schedule->aircraft, info->runway);
    }
    pthread_join(params->schedule->aircraft->aircraft_thread, NULL); // joining thread after done

    cout << "[TIME] Real: " << params->timeOutput << "\n";

    delete info;
    delete params;
}

void FlightManager::simulate(std::vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC) {
    
    SimulationTimer timer;
    timer.start();

    // Sorting schedules by scheduled time and priority
    std::sort(schedules.begin(), schedules.end(), compareFlightSchedules);

    std::vector<std::thread> flightThreads;

    for (auto& schedule : schedules) {
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

        // calling method to assign runway to local pointer
        Runway* rw = getRunwayForFlight(schedule, rwyA, rwyB, rwyC);

        cout << "[TIME] Real: " << oss.str() << "\n";

        // Create parameters for flight processing
        FlightProcessParams* params = new FlightProcessParams;
        params->schedule = &schedule;
        params->runway = rw;
        params->runway_C = &rwyC;
        params->timeOutput = oss.str();

        // Create a thread for each flight using processFlight
        flightThreads.emplace_back(&FlightManager::processFlight, this, params);
    }

    // Join all flight threads
    for (auto& thread : flightThreads) {
        thread.join();
    }

    // Continue running until 5 minutes real-time
    while (timer.getRealTimeElapsed() < std::chrono::minutes(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Exit loop if all scheduled flights are done
        bool flag = true;
        for (const auto& schedule : schedules) {
            if (!schedule.aircraft->isDone) flag = false;
        }
        if (flag) break;
    }
}

void FlightManager::simulateArrival(Aircraft* ac, Runway* rw) {

    SimulationTimer timer; // For time output in events
    cout << "\n[SIMULATION] Flight " << ac->id << " is ARRIVING.\n";

    // sleeping to simulate real time delay (edit later to match animation delay for each phase)
    ac->updatePhase(AircraftPhase::Holding); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->updatePhase(AircraftPhase::Approach); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->updatePhase(AircraftPhase::Landing); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // runway assigned after landing
    rw->assign(ac); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    rw->release();
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
    ac->updatePhase(AircraftPhase::AtGate); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->updatePhase(AircraftPhase::Taxi); ac->checkGroundFault(); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (ac->hasFault) return;

    rw->assign(ac); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->updatePhase(AircraftPhase::Takeoff); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->updatePhase(AircraftPhase::Climb); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->updatePhase(AircraftPhase::Cruise); std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ac->isDone = true;
    rw->release();
    cout << "[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
}