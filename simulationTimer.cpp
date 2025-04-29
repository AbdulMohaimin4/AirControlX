#include "simulationTimer.hpp"
#include <ctime>

SimulationTimer::SimulationTimer() : startTime(std::chrono::system_clock::now()) {

    // Setting simStartTime to midnight (00:00:00) of the current day
    auto now = std::chrono::system_clock::now();
    time_t now_c = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *localtime(&now_c);
    local_tm.tm_hour = 0;
    local_tm.tm_min = 0;
    local_tm.tm_sec = 0;
    simStartTime = std::chrono::system_clock::from_time_t(mktime(&local_tm));
}

void SimulationTimer::start() {

    startTime = std::chrono::system_clock::now();
}

std::chrono::seconds SimulationTimer::getRealTimeElapsed() const {

    auto elapsed = std::chrono::system_clock::now() - startTime;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsed);
}

std::chrono::system_clock::time_point SimulationTimer::getSimulatedTime() const {

    auto elapsed = std::chrono::system_clock::now() - startTime;
    auto simulatedSeconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed) * SIMULATION_SPEED;
    auto simulatedDuration = std::chrono::duration_cast<std::chrono::system_clock::duration>(simulatedSeconds);
    return simStartTime + simulatedDuration;
}

std::chrono::system_clock::time_point SimulationTimer::getStartTime() const {
    
    return startTime;
}