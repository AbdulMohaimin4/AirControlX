#ifndef RUNWAY_HPP
#define RUNWAY_HPP

#include <queue>
#include <string>
#include <iostream>
//#include "aircraft.hpp"
#include "enums.hpp"

class Aircraft;

class Runway {
    
public:
    RunwayID id;
    Direction direction;
    bool isAvailable;
    pthread_mutex_t runway_mutex; // to allow aircrafts to lock/unlock runways
    std::queue<Aircraft*> waitQueue;
    Aircraft* inUseBy;

    Runway(RunwayID ID, Direction dir);
    ~Runway();

    std::string getID();
    void assign(Aircraft* ac);
    void release();
};

// structure to pass into thread func so the func can decide runway locking itself
struct RunwayInfo {

    Runway* runway;
    Runway* runway_C;
    bool isArrival;
    int priority;

};

#endif
