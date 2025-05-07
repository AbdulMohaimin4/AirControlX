// Runway.cpp

#include "runway.hpp"
#include "aircraft.hpp"

Runway::Runway(RunwayID ID, Direction dir) : id(ID), direction(dir), isAvailable(true), inUseBy(nullptr) {

    pthread_mutex_init(&this->runway_mutex, NULL);
    pthread_mutex_init(&this->queue_mutex, NULL);
}

Runway::~Runway() { 

    pthread_mutex_destroy(&this->runway_mutex); 
    pthread_mutex_destroy(&this->queue_mutex);
}

string Runway::getID() {

    switch (id) {

        case RunwayID::RWY_A: return "RWY-A";
        case RunwayID::RWY_B: return "RWY-B";
        case RunwayID::RWY_C: return "RWY-C";
    }
    return "Unknown";
}

void Runway::assign(Aircraft* ac, chrono::system_clock::time_point queueStartTime, int& waitTimeSeconds) {

    auto now = chrono::system_clock::now(); // starting timer at assignment (now)
    // calculating the time it took since aircraft's enquque till assignment (delay) | stored in inputted ref var
    waitTimeSeconds = chrono::duration_cast<chrono::seconds>(now - queueStartTime).count();

    // locking queue 
    pthread_mutex_lock(&this->queue_mutex);
    if (this->isAvailable) {

        this->inUseBy = ac;
        this->isAvailable = false;
        cout << "[RUNWAY] " << ac->id << " assigned to " << getID() << " after waiting "
        << waitTimeSeconds << " seconds\n";
    } 
    else {

        // initializing and passing 'AircraftQueueEntry' struct to queue simultaeneously
        this->waitQueue.push({ac, ac->airline->priority, now}); 
        cout << "[QUEUE] " << ac->id << " added to " << this->getID() << " queue.\n";
    }
    pthread_mutex_unlock(&this->queue_mutex); // unlocked for other aircrafts to be enqueued
}

void Runway::release() {

    pthread_mutex_lock(&this->queue_mutex);

    if (this->inUseBy != nullptr) {

        cout << "[RUNWAY] " << this->inUseBy->id << " cleared from runway" << this->getID() << "\n";
        this->inUseBy = nullptr;
    }
    this->isAvailable = true;

    if ( !(this->waitQueue.empty()) ) {
        
        AircraftQueueEntry next = this->waitQueue.top();
        this->waitQueue.pop();

        pthread_mutex_unlock(&this->queue_mutex);
        int waitTimeSeconds = 0; // this will be sent by refrence to assign method so that it can be used as output
        this->assign(next.aircraft, next.entryTime, waitTimeSeconds);
    }
    else 
        pthread_mutex_unlock(&this->queue_mutex);
}
