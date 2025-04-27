#ifndef SIMULATIONTIMER_HPP
#define SIMULATIONTIMER_HPP

#include <chrono>

class SimulationTimer {
private:
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point simStartTime;
    const double SIMULATION_SPEED = 8640.0; // 12 hours in 5 minutes (12*60*60 / 5*60)

public:
    SimulationTimer();
    void start();
    std::chrono::seconds getRealTimeElapsed() const;
    std::chrono::system_clock::time_point getSimulatedTime() const;
    std::chrono::system_clock::time_point getStartTime() const;
};

#endif