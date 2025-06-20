#include "airline.hpp"
#include "avnGenerator.hpp"
#include <sstream>
#include <iomanip>

Airline::Airline(const std::string& name, FlightType type, int total, int available)
    : name(name), type(type), availableFlights(available), totalFlights(total) {
    priority = (type == FlightType::Medical) ? 4 :
               (type == FlightType::Military) ? 3 :
               (type == FlightType::Commercial) ? 2 : 1;

    log_file.open(name + "_avns.log", std::ios::app);
    if (!log_file.is_open()) {

        std::cerr << "Error: Could not open " << name << "_avns.log for writing\n";
    }
}

Airline::~Airline() {

    if (log_file.is_open()) {

        log_file.close();
    }
}

void Airline::logViolation(const AVNRecord& avn) {
    
    if (log_file.is_open()) {

        std::stringstream ss;
        ss << avn.avn_id << "|"
           << avn.airline_name << "|"
           << avn.aircraft_id << "|"
           << static_cast<int>(avn.aircraft_type) << "|"
           << avn.recorded_speed << "|"
           << avn.issue_date << "|"
           << avn.fine_amount << "|"
           << avn.service_fee << "|"
           << (avn.paid ? "1" : "0") << "|"
           << avn.due_date;
        log_file << ss.str() << "\n";
        log_file.flush();
    } 
    else {
        
        std::cerr << "Error: " << name << "_avns.log not open for writing\n";
    }
}