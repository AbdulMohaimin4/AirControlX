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
    info->runway_C = params->runway_C; // incase aircraft is high priority and A/B rwys are locked

    params->schedule->aircraft->updatePhase(params->schedule->isArrival ? AircraftPhase::Holding : AircraftPhase::AtGate);
    params->schedule->aircraft->checkRunway(info); // waiting for runway to be free

    // ASSIGINING RUNWAY HERE and calculating wait time
    int waitTimeSeconds;
    info->runway->assign(params->schedule->aircraft, info->queueStartTime, waitTimeSeconds);
    params->schedule->waitTimeSeconds = waitTimeSeconds;

    if (params->schedule->isArrival) {

        this->simulateArrival(params->schedule->aircraft, info->runway, waitTimeSeconds); // assignment of runway
    }
    else {

        this->simulateDeparture(params->schedule->aircraft, info->runway, waitTimeSeconds); // assignment of runway
    }
    //pthread_join(params->schedule->aircraft->aircraft_thread, NULL); // joining thread after done

    delete info;
    delete params;
}

void FlightManager::simulate(vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC) {
    
    SimulationTimer timer;
    timer.start();

    // sorting schedules by scheduled time and priority (using custom method)
    sort(schedules.begin(), schedules.end(), compareFlightSchedules);

    vector<thread> flightThreads;

    for (auto& schedule : schedules) {
        // Format times for output
        auto realTimeSec = timer.getRealTimeElapsed().count();
        int realMin = realTimeSec / 60;
        int realSec = realTimeSec % 60;
        auto simTime = timer.getSimulatedTime();
        time_t simTime_t = chrono::system_clock::to_time_t(simTime);
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
    while (timer.getRealTimeElapsed() < chrono::minutes(5)) {

        this_thread::sleep_for(chrono::milliseconds(100));

        // Exit loop if all scheduled flights are done
        bool flag = true;
        for (const auto& schedule : schedules) {

            if (!schedule.aircraft->isDone) flag = false;
        }
        if (flag) break;
    }

    cout << "\n[FINAL WAIT TIMES]\n";
    for (const auto& schedule : schedules) {

        cout << schedule.aircraft->id << " waited " << schedule.waitTimeSeconds << " seconds\n";
    }
}

void FlightManager::simulateArrival(Aircraft* ac, Runway* rw, int waitTimeSeconds) {

    SimulationTimer timer; // For time output in events

    cout << "\n[SIMULATION] Flight " << ac->id << " is ARRIVING (waited " 
    << waitTimeSeconds << " seconds).\n";

    // sleeping to simulate real time delay (edit later to match animation delay for each phase)
    // phase transitions
    ac->updatePhase(AircraftPhase::Holding); this_thread::sleep_for(chrono::milliseconds(1000));
    ac->updatePhase(AircraftPhase::Approach); this_thread::sleep_for(chrono::milliseconds(1000));
    ac->updatePhase(AircraftPhase::Landing); this_thread::sleep_for(chrono::milliseconds(1000));
    
    // runway assigned after landing
    ac->updatePhase(AircraftPhase::Taxi); 
    ac->checkGroundFault();
    this_thread::sleep_for(chrono::seconds(1));
    if (!ac->hasFault) {

        ac->updatePhase(AircraftPhase::AtGate);
        ac->isDone = true;
    }
    ac->isDone = true;
    rw->release();

    cout << "\n[STATUS] " << ac->id << " final status: " << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
    return;
}

void FlightManager::simulateDeparture(Aircraft* ac, Runway* rw, int waitTimeSeconds) {

    SimulationTimer timer; // For time output in events

    cout << "\n[SIMULATION] Flight " << ac->id 
    << " is DEPARTING (waited " << waitTimeSeconds << " seconds).\n";

    ac->updatePhase(AircraftPhase::Taxi);
    ac->checkGroundFault(); 
    this_thread::sleep_for(chrono::milliseconds(1000));
    if (ac->hasFault) {
        
        rw->release(); // terminate flight if faulty
        return;
    }

    ac->updatePhase(AircraftPhase::Takeoff); this_thread::sleep_for(chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Climb); this_thread::sleep_for(chrono::seconds(1));
    ac->updatePhase(AircraftPhase::Cruise); this_thread::sleep_for(chrono::seconds(1));
    ac->isDone = true;
    rw->release(); // release runway after departure successful

    cout << "\n[STATUS] " << ac->id << " final status: " 
    << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
    return;
}