// AVNLog.cpp

#include "AVNLog.hpp"

void AVNLog::issueAVN(std::string aircraftID, std::string reason, double speed, AircraftPhase phase) {
    
    std::cout << "[AVN] Violation: " << aircraftID << " | Phase: " << toString(phase)
              << " | Speed: " << speed << " km/h | Reason: " << reason << "\n";
}
