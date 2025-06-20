#include "avnGenerator.hpp"
#include "ipcPaths.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void signalHandler(int signum) {

    keep_running = 0;
}

string AVNRecord::serialize() const {

    stringstream ss;
    ss << avn_id << "|"
       << airline_name << "|"
       << aircraft_id << "|"
       << static_cast<int>(aircraft_type) << "|"
       << recorded_speed << "|"
       << issue_date << "|"
       << fine_amount << "|"
       << service_fee << "|"
       << (paid ? "1" : "0") << "|"
       << due_date;
    return ss.str();
}

AVNRecord AVNRecord::deserialize(const string& data) {

    AVNRecord avn;
    stringstream ss(data);
    string paid_str;
    int type_int;
    getline(ss, avn.avn_id, '|');
    getline(ss, avn.airline_name, '|');
    getline(ss, avn.aircraft_id, '|');
    ss >> type_int;
    avn.aircraft_type = static_cast<FlightType>(type_int);
    ss.ignore();
    ss >> avn.recorded_speed;
    ss.ignore();
    ss >> avn.issue_date;
    ss.ignore();
    ss >> avn.fine_amount;
    ss.ignore();
    ss >> avn.service_fee;
    ss.ignore();
    getline(ss, paid_str, '|');
    avn.paid = (paid_str == "1");
    ss >> avn.due_date;
    return avn;
}

AVNGenerator::AVNGenerator() {

    read_fd = open(ATC_TO_AVN_PIPE, O_RDONLY | O_NONBLOCK);
    write_portal_fd = open(AVN_TO_PORTAL_PIPE, O_WRONLY | O_NONBLOCK);
}

AVNGenerator::~AVNGenerator() {

    close(read_fd);
    close(write_portal_fd);
}

string AVNGenerator::generateAVNID() {

    static int counter = 1;
    stringstream ss;
    ss << "AVN" << setw(6) << setfill('0') << counter++;
    return ss.str();
}

double AVNGenerator::calculateFine(FlightType type) {

    switch (type) {

        case FlightType::Commercial:
            return 500000.0;
        case FlightType::Cargo:
            return 700000.0;
        default:
            return 500000.0;
    }
}

double AVNGenerator::calculateServiceFee(double fine) {

    return fine * 0.15;
}

void AVNGenerator::processViolation(const string& violation_data, Airline* airline) {

    stringstream ss(violation_data);
    string aircraft_id, violation_type;
    double speed, altitude;
    
    getline(ss, aircraft_id, ',');
    ss >> speed;
    ss.ignore();
    ss >> altitude;
    ss.ignore();
    getline(ss, violation_type);

    AVNRecord avn;
    avn.avn_id = generateAVNID();
    avn.airline_name = airline->getName();
    avn.aircraft_id = aircraft_id;
    avn.aircraft_type = airline->getType();
    avn.recorded_speed = speed;
    avn.issue_date = time(nullptr);
    avn.due_date = avn.issue_date + (3 * 24 * 60 * 60); // 3 days
    avn.paid = false;
    avn.fine_amount = calculateFine(airline->getType());
    avn.service_fee = calculateServiceFee(avn.fine_amount);

    active_avns.push_back(avn);
    airline->logViolation(avn);

    // Forward to Airline Portal
    stringstream avn_ss;
    avn_ss << avn.avn_id << ","
           << avn.aircraft_id << ","
           << avn.fine_amount << ","
           << avn.service_fee << ","
           << avn.issue_date << ","
           << (avn.paid ? "1" : "0");
    
    write(write_portal_fd, avn_ss.str().c_str(), avn_ss.str().length());
}

void AVNGenerator::updatePaymentStatus(const string& avn_id, Airline* airline) {

    for (auto& avn : active_avns) {
        if (avn.avn_id == avn_id) {
            avn.paid = true;
            // Update airline log by rewriting
            ofstream temp_file(airline->getName() + "_avns_temp.log");
            for (const auto& a : active_avns) {

                if (a.airline_name == airline->getName()) {
                    temp_file << a.serialize() << "\n";
                }
            }
            temp_file.close();
            rename((airline->getName() + "_avns_temp.log").c_str(), (airline->getName() + "_avns.log").c_str());
            break;
        }
    }
}

void AVNGenerator::run() {

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    char buffer[1024];
    while (keep_running) {

        int n = read(read_fd, buffer, sizeof(buffer)-1);
        if (n > 0) {
            
            buffer[n] = '\0';
            string aircraft_id = string(buffer).substr(0, string(buffer).find(','));
            Airline* airline = nullptr;

            if (aircraft_id.find("PK") == 0) {

                static Airline pia("PIA", FlightType::Commercial, 6, 4);
                airline = &pia;
            } 
            else if (aircraft_id.find("AB") == 0) {

                static Airline airblue("AirBlue", FlightType::Commercial, 4, 4);
                airline = &airblue;
            } 
            else if (aircraft_id.find("FDX") == 0) {

                static Airline fedex("FedEx", FlightType::Cargo, 3, 2);
                airline = &fedex;
            } 
            else if (aircraft_id.find("PAF") == 0) {

                static Airline paf("Pakistan_Airforce", FlightType::Military, 2, 1);
                airline = &paf;
            } 
            else if (aircraft_id.find("BD") == 0) {

                static Airline bluedart("BlueDart", FlightType::Cargo, 2, 2);
                airline = &bluedart;
            } 
            else if (aircraft_id.find("AKA") == 0) {

                static Airline aghakhan("AghaKhan_Air_Ambulance", FlightType::Medical, 2, 1);
                airline = &aghakhan;
            }
            if (airline) {
                
                processViolation(buffer, airline);
            }
        }
        usleep(100000); // 100ms
    }
}