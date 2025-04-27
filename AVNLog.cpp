// AVNLog.cpp

#include "AVNLog.hpp"
#include "utils.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

void AVNLog::issueAVN(std::string aircraftID, std::string reason, double speed, AircraftPhase phase, std::string timeStr) {
    std::cout << "[AVN] Violation: " << aircraftID << " | Phase: " << toString(phase)
              << " | Speed: " << speed << " km/h | Reason: " << reason << " | Time: " << timeStr << "\n";
}
