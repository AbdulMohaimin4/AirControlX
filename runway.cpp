// Runway.cpp

#include "runway.hpp"
#include "aircraft.hpp"

Runway::Runway(RunwayID ID, Direction dir) : id(ID), direction(dir), isAvailable(true), inUseBy(nullptr) {

    pthread_mutex_init(&this->runway_mutex, NULL);
}

Runway::~Runway() { pthread_mutex_destroy(&this->runway_mutex); }

std::string Runway::getID() {

    switch (id) {

        case RunwayID::RWY_A: return "RWY-A";
        case RunwayID::RWY_B: return "RWY-B";
        case RunwayID::RWY_C: return "RWY-C";
    }
    return "Unknown";
}

void Runway::assign(Aircraft* ac) {

    if (this->isAvailable) {

        this->inUseBy = ac;
        this->isAvailable = false;
        std::cout << "[RUNWAY] " << ac->id << " assigned to " << getID() << "\n";
    } 
    else {

        this->waitQueue.push(ac);
        std::cout << "[QUEUE] " << ac->id << " added to " << getID() << " queue.\n";
    }
}

void Runway::release() {

    if (this->inUseBy != nullptr) {

        std::cout << "[RUNWAY] " << this->inUseBy->id << " cleared from " << getID() << "\n";
        this->inUseBy = nullptr;
    }
    this->isAvailable = true;

    if (!this->waitQueue.empty()) {
        
        Aircraft* next = this->waitQueue.front();
        this->waitQueue.pop();
        assign(next);
    }
}
