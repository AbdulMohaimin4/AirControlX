#ifndef SIMULATIONTIMER_HPP
#define SIMULATIONTIMER_HPP

#include <chrono>

class SimulationTimer {
    
private:
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point simStartTime;
    const double SIMULATION_SPEED = 60.0; // 1 hour in one min

public:
    SimulationTimer();
    void start();
    std::chrono::seconds getRealTimeElapsed() const;
    std::chrono::system_clock::time_point getSimulatedTime() const;
    std::chrono::system_clock::time_point getStartTime() const;
};

#endif