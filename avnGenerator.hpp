#ifndef AVN_GENERATOR_HPP
#define AVN_GENERATOR_HPP

#include <string>
#include <vector>
#include "airline.hpp"
using namespace std;

struct AVNRecord {

    string avn_id;
    string airline_name;
    string aircraft_id;
    FlightType aircraft_type;
    double recorded_speed;
    time_t issue_date;
    double fine_amount;
    double service_fee;
    bool paid;
    time_t due_date;

    // Serialization to string for file storage
    string serialize() const;
    // Deserialization from string
    static AVNRecord deserialize(const string& data);
};

class AVNGenerator {
private:
    int read_fd;
    int write_portal_fd;
    vector<AVNRecord> active_avns;
    
    string generateAVNID();
    double calculateFine(FlightType type);
    double calculateServiceFee(double fine);

public:
    AVNGenerator();
    ~AVNGenerator();
    void processViolation(const string& violation_data, Airline* airline);
    void updatePaymentStatus(const string& avn_id, Airline* airline);
    void run();
};

#endif