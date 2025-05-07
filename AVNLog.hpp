#ifndef AVNLOG_HPP
#define AVNLOG_HPP

#include <string>
#include "enums.hpp"
using namespace std;

class AVNLog {
public:
    static void issueAVN(string aircraftID, string reason, double speed, AircraftPhase phase);
};

#endif