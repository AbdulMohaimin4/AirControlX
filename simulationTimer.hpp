#ifndef SIMULATIONTIMER_HPP
#define SIMULATIONTIMER_HPP

#include <chrono>
using namespace std;

class SimulationTimer {
    
private:
    chrono::system_clock::time_point startTime;
    chrono::system_clock::time_point simStartTime;
    const double SIMULATION_SPEED = 60.0; // 1 hour in one min (will change later)

public:
    SimulationTimer();
    void start();
    chrono::seconds getRealTimeElapsed() const;
    chrono::system_clock::time_point getSimulatedTime() const;
    chrono::system_clock::time_point getStartTime() const;
};

#endif