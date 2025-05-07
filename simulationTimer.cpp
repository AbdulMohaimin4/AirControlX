#include "simulationTimer.hpp"
#include <ctime>

SimulationTimer::SimulationTimer() : startTime(chrono::system_clock::now()) {

    // Setting simStartTime to midnight (00:00:00) of the current day
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&now_c);
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    simStartTime = chrono::system_clock::from_time_t(mktime(&local_tm));
}

void SimulationTimer::start() {

    startTime = chrono::system_clock::now();
}

chrono::seconds SimulationTimer::getRealTimeElapsed() const {

    auto elapsed = chrono::system_clock::now() - startTime;
    return chrono::duration_cast<chrono::seconds>(elapsed);
}

chrono::system_clock::time_point SimulationTimer::getSimulatedTime() const {

    auto elapsed = chrono::system_clock::now() - startTime;
    auto simulatedSeconds = chrono::duration_cast<chrono::seconds>(elapsed) * SIMULATION_SPEED;
    auto simulatedDuration = chrono::duration_cast<chrono::system_clock::duration>(simulatedSeconds);
    return simStartTime + simulatedDuration;
}

chrono::system_clock::time_point SimulationTimer::getStartTime() const {
    
    return startTime;
}