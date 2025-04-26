#ifndef AVNLOG_HPP
#define AVNLOG_HPP

#include <string>
#include <iostream>
#include "enums.hpp"
#include "utils.hpp"

class AVNLog {
    
public:
    static void issueAVN(std::string aircraftID, std::string reason, double speed, AircraftPhase phase);
};

#endif
