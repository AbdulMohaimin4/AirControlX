#ifndef RUNWAY_HPP
#define RUNWAY_HPP

#include <queue>
#include <string>
#include <iostream>
#include "aircraft.hpp"
#include "enums.hpp"

class Runway {
    
public:
    RunwayID id;
    Direction direction;
    bool isAvailable;
    std::queue<Aircraft*> waitQueue;
    Aircraft* inUseBy;

    Runway(RunwayID ID, Direction dir);
    std::string getID();
    void assign(Aircraft* ac);
    void release();
};

#endif
