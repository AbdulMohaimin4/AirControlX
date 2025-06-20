#ifndef AIRLINE_PORTAL_HPP
#define AIRLINE_PORTAL_HPP

#include <string>
#include <vector>
#include "avnGenerator.hpp"
using namespace std;

class AirlinePortal {
private:
    // file descriptors
    int read_fd; 
    int read_stripe_fd;
    int write_stripe_fd;

    vector<AVNRecord> avns; // to store all the avns for airlines generated
    bool authenticated;
    string current_airline;
    
    void loadAVNsFromFile(const string& airline_name);

public:
    AirlinePortal();
    ~AirlinePortal();
    
    bool login(const string& aircraft_id);
    void displayAVNs() const;
    void processPayment(const string& avn_id);
    void run();
};

#endif