#ifndef AVNLOG_HPP
#define AVNLOG_HPP

#include <string>
#include "enums.hpp"

class AVNLog {
public:
    static void issueAVN(std::string aircraftID, std::string reason, double speed, AircraftPhase phase, std::string timeStr);
};

#endif