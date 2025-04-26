// Runway.cpp

#include "runway.hpp"

Runway::Runway(RunwayID ID, Direction dir) : id(ID), direction(dir), isAvailable(true), inUseBy(nullptr) {}

std::string Runway::getID() {
    switch (id) {
        case RunwayID::RWY_A: return "RWY-A";
        case RunwayID::RWY_B: return "RWY-B";
        case RunwayID::RWY_C: return "RWY-C";
    }
    return "Unknown";
}

void Runway::assign(Aircraft* ac) {
    if (isAvailable) {
        inUseBy = ac;
        isAvailable = false;
        std::cout << "[RUNWAY] " << ac->id << " assigned to " << getID() << "\n";
    } else {
        waitQueue.push(ac);
        std::cout << "[QUEUE] " << ac->id << " added to " << getID() << " queue.\n";
    }
}

void Runway::release() {
    if (inUseBy != nullptr) {
        std::cout << "[RUNWAY] " << inUseBy->id << " cleared from " << getID() << "\n";
        inUseBy = nullptr;
    }
    isAvailable = true;
    if (!waitQueue.empty()) {
        Aircraft* next = waitQueue.front();
        waitQueue.pop();
        assign(next);
    }
}
