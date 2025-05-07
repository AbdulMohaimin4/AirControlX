#include "flightManager.hpp"
#include <iostream>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <vector>
#include <chrono>

// Comparison function for sorting FlightSchedule objects
bool compareFlightSchedules(const FlightSchedule& a, const FlightSchedule& b) {

    if (a.scheduledTime == b.scheduledTime) {

        return a.priority > b.priority; 
    }
    return a.scheduledTime < b.scheduledTime;
}

Runway* FlightManager::getRunwayForFlight(const FlightSchedule& schedule, Runway& rwyA, Runway& rwyB, Runway& rwyC) {

    if (schedule.isArrival) {

        return (schedule.aircraft->airline->type == FlightType::Commercial || 
                schedule.aircraft->airline->type == FlightType::Medical ||
                schedule.aircraft->airline->type == FlightType::Military) ? &rwyA : &rwyC;
    } 
    else {

        return (schedule.aircraft->airline->type == FlightType::Commercial || 
                schedule.aircraft->airline->type == FlightType::Medical ||
                schedule.aircraft->airline->type == FlightType::Military) ? &rwyB : &rwyC;
    }
}

void simulateArrival(Aircraft* ac, Runway* rw, int waitTimeSeconds) {

    cout << "\n[SIMULATION] Flight " << ac->id << " is ARRIVING (waited " 
              << waitTimeSeconds << " seconds).\n";
    ac->updatePhase(AircraftPhase::Holding); this_thread::sleep_for(chrono::milliseconds(5000));
    ac->updatePhase(AircraftPhase::Approach); this_thread::sleep_for(chrono::milliseconds(5000));
    ac->updatePhase(AircraftPhase::Landing); this_thread::sleep_for(chrono::milliseconds(5000));
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

void simulateDeparture(Aircraft* ac, Runway* rw, int waitTimeSeconds) {

    cout << "\n[SIMULATION] Flight " << ac->id 
              << " is DEPARTING (waited " << waitTimeSeconds << " seconds).\n";
    ac->updatePhase(AircraftPhase::Taxi);
    ac->checkGroundFault(); 
    this_thread::sleep_for(chrono::milliseconds(1000));
    if (ac->hasFault) {
        rw->release();
        return;
    }
    ac->updatePhase(AircraftPhase::Takeoff); this_thread::sleep_for(chrono::seconds(5));
    ac->updatePhase(AircraftPhase::Climb); this_thread::sleep_for(chrono::seconds(5));
    ac->updatePhase(AircraftPhase::Cruise); this_thread::sleep_for(chrono::seconds(5));
    ac->isDone = true;
    rw->release();
    cout << "\n[STATUS] " << ac->id << " final status: " 
              << toString(ac->phase) << " | Speed: " << ac->speed << " km/h\n\n";
    return;
}

static void* thread_fun(void* arg) {

    RunwayInfo* runwayInfo = (RunwayInfo*)arg;
    cout << runwayInfo->runway->getID() << "...\n";
    this_thread::sleep_for(chrono::milliseconds(1000));
    runwayInfo->queueStartTime = chrono::system_clock::now();

    // Non-priority flights (priority != 4)
    if (runwayInfo->priority != 4) {

        pthread_mutex_lock(&runwayInfo->runway->runway_mutex);

        runwayInfo->assignedRunway = runwayInfo->runway;
        int waitTimeSeconds;
        runwayInfo->runway->assign(runwayInfo->aircraft, runwayInfo->queueStartTime, waitTimeSeconds);
        if (runwayInfo->isArrival) {

            simulateArrival(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
        } else {

            simulateDeparture(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
        }
        runwayInfo->waitTimeSeconds = waitTimeSeconds;

        pthread_mutex_unlock(&runwayInfo->runway->runway_mutex);    
    } 
    else {
        // Priority 4: Try primary runway
        if (pthread_mutex_trylock(&runwayInfo->runway->runway_mutex) == 0) {
            runwayInfo->assignedRunway = runwayInfo->runway;
            int waitTimeSeconds;
            runwayInfo->runway->assign(runwayInfo->aircraft, runwayInfo->queueStartTime, waitTimeSeconds);
            if (runwayInfo->isArrival) {

                simulateArrival(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
            } else {

                simulateDeparture(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
            }
            runwayInfo->waitTimeSeconds = waitTimeSeconds;

            pthread_mutex_unlock(&runwayInfo->runway->runway_mutex);
        } 
        else {

            // Try Runway C
            if (pthread_mutex_trylock(&runwayInfo->runway_C->runway_mutex) == 0) {
                runwayInfo->assignedRunway = runwayInfo->runway_C;
                int waitTimeSeconds;
                runwayInfo->runway_C->assign(runwayInfo->aircraft, runwayInfo->queueStartTime, waitTimeSeconds);
                if (runwayInfo->isArrival) {

                    simulateArrival(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
                } else {

                    simulateDeparture(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
                }
                runwayInfo->waitTimeSeconds = waitTimeSeconds;

                pthread_mutex_unlock(&runwayInfo->runway_C->runway_mutex);
            } 
            else {
                // Both runways locked: queue on primary runway
                pthread_mutex_lock(&runwayInfo->runway->runway_mutex);

                runwayInfo->assignedRunway = runwayInfo->runway;
                int waitTimeSeconds;
                runwayInfo->runway->assign(runwayInfo->aircraft, runwayInfo->queueStartTime, waitTimeSeconds);
                if (runwayInfo->isArrival) {

                    simulateArrival(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
                } else {

                    simulateDeparture(runwayInfo->aircraft, runwayInfo->assignedRunway, waitTimeSeconds);
                }
                runwayInfo->waitTimeSeconds = waitTimeSeconds;

                pthread_mutex_unlock(&runwayInfo->runway->runway_mutex);
            }
        }
    }
    cout << "\nAircraft cleared to " << (runwayInfo->assignedRunway->getID() == "RWY-B" ? "depart" :
        runwayInfo->assignedRunway->getID() == "RWY-A" ? "land" : "proceed on Runway C") << "\n";

    pthread_exit(NULL);
}

void FlightManager::checkRunway(RunwayInfo* runwayInfo) {

    cout << "\n\n" << runwayInfo->aircraft->airline->name << " flight "<< runwayInfo->aircraft->id << " requesting runway ";
    pthread_create(&runwayInfo->aircraft->aircraft_thread, NULL, thread_fun, (void*)runwayInfo);
    pthread_join(runwayInfo->aircraft->aircraft_thread, NULL);
    runwayInfo->runway = runwayInfo->assignedRunway;
}

void FlightManager::processFlight(FlightProcessParams* params) {

    RunwayInfo* info = new RunwayInfo;
    info->isArrival = params->schedule->isArrival;
    info->priority = params->schedule->aircraft->airline->priority;
    info->runway = params->runway;
    info->runway_C = params->runway_C;
    info->aircraft = params->schedule->aircraft;
    params->schedule->aircraft->updatePhase(params->schedule->isArrival ? AircraftPhase::Holding : AircraftPhase::AtGate);

    this->checkRunway(info);
    params->schedule->aircraft->waitTimeSeconds = info->waitTimeSeconds;
    delete info;
}

void FlightManager::simulate(vector<FlightSchedule>& schedules, Runway& rwyA, Runway& rwyB, Runway& rwyC) {

    auto startTime = chrono::steady_clock::now();
    sort(schedules.begin(), schedules.end(), compareFlightSchedules);
    vector<thread> flightThreads;
    vector<bool> processed(schedules.size(), false);

    while (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count() < 300) {

        auto elapsedReal = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count();
        int simSeconds = elapsedReal; // 1 real second = 1 simulated second
        int realMin = elapsedReal / 60;
        int realSec = elapsedReal % 60;
        int simMin = simSeconds / 60;
        int simSec = simSeconds % 60;
        ostringstream oss;
        oss << setfill('0') << setw(2) << realMin << ":" << setw(2) << realSec
            << " | Sim: " << setw(2) << simMin << ":" << setw(2) << simSec;
        //cout << "[TIME] Real: " << oss.str() << "\n";
        bool allDone = true;

        for (int i = 0; i < schedules.size(); ++i) {

            if (!processed[i]) {

                allDone = false;
                time_t schedTime_t = chrono::system_clock::to_time_t(schedules[i].scheduledTime);
                tm* sched_tm = localtime(&schedTime_t);
                int schedSeconds = sched_tm->tm_min * 60 + sched_tm->tm_sec;
                if (simSeconds >= schedSeconds) {

                    Runway* rw = getRunwayForFlight(schedules[i], rwyA, rwyB, rwyC);
                    FlightProcessParams* params = new FlightProcessParams;
                    params->schedule = &schedules[i];
                    params->runway = rw;
                    params->runway_C = &rwyC;
                    params->timeOutput = oss.str();
                    flightThreads.emplace_back(&FlightManager::processFlight, this, params);
                    processed[i] = true;
                    cout << "[SCHEDULE] Processing flight " << schedules[i].aircraft->id 
                              << " at sim time " << setw(2) << simMin << ":" << setw(2) << simSec << "\n";
                }
            }
            if (!schedules[i].aircraft->isDone) {

                allDone = false;
            }
        }
        if (allDone) {

            break;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    for (auto& thread : flightThreads) {

        thread.join();
    }

    cout << "\n[FINAL WAIT TIMES]\n";
    for (const auto& schedule : schedules) {

        cout << schedule.aircraft->id << " waited " << schedule.aircraft->waitTimeSeconds << " seconds\n";
    }
}