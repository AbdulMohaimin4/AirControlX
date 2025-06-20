#ifndef ATC_CONTROLLER_HPP
#define ATC_CONTROLLER_HPP

#include <string>
#include <vector>
#include <map>
#include "enums.hpp"
using namespace std;

struct ViolationRecord {

    string aircraft_id;
    double speed;
    double altitude;
    string violation_type;
    bool active;
    time_t detected_time;
};

class ATCController {
    
private:
    int write_fd;
    map<string, ViolationRecord> violations;
    int active_violations;

public:
    ATCController();
    ~ATCController();
    void monitorAircraft(const string& aircraft_id, double speed, double altitude);
    void displayDashboard() const;
    void updateViolationStatus(const string& aircraft_id, bool status);
    void run();
};

#endif