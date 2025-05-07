#include "AVNLog.hpp"
#include "utils.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>

void AVNLog::issueAVN(string aircraftID, string reason, double speed, AircraftPhase phase) {
    cout << "[AVN] Violation: " << aircraftID << " | Phase: " << toString(phase)
              << " | Speed: " << speed << " km/h | Reason: " << reason << "\n";
}
