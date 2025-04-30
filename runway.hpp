#ifndef RUNWAY_HPP
#define RUNWAY_HPP

#include <queue>
#include <string>
#include <iostream>
#include "utils.hpp"
#include "enums.hpp"

// forward declaration to avoid circular dependency
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


#endif
