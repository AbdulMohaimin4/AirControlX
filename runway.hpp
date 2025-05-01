#ifndef RUNWAY_HPP
#define RUNWAY_HPP

#include <queue>
#include <string>
#include <iostream>
#include "utils.hpp"
#include "enums.hpp"

// forward declaration to avoid circular dependency
class Aircraft;

// struct to handle flight priorities when enqueing 
struct AircraftQueueEntry {

    Aircraft* aircraft;
    int priority;
    chrono::system_clock::time_point entryTime; // for FCFS in priority queue

    // overloading the < operator to be used in priority queue STL
    bool operator<(const AircraftQueueEntry& other) const {
        if (priority != other.priority) return priority < other.priority;
        return entryTime > other.entryTime; // if not same prioirty, then FCFS applied
    }
};

class Runway {
    
public:
    RunwayID id;
    Direction direction;
    bool isAvailable;
    pthread_mutex_t runway_mutex; // to allow aircrafts to lock/unlock runways
    pthread_mutex_t queue_mutex;
    priority_queue<AircraftQueueEntry> waitQueue; // waiting priority queue
    Aircraft* inUseBy;

    Runway(RunwayID ID, Direction dir);
    ~Runway();

    string getID();
    void assign(Aircraft* ac, chrono::system_clock::time_point queueStartTime, int& waitTimeSeconds);
    void release();
};


#endif
