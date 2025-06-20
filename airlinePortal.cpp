#include "airlinePortal.hpp"
#include "ipcPaths.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void signalHandler(int signum) {

    keep_running = 0;
}

AirlinePortal::AirlinePortal() : authenticated(false) {

    read_fd = open(AVN_TO_PORTAL_PIPE, O_RDONLY | O_NONBLOCK);
    read_stripe_fd = open(STRIPE_TO_PORTAL_PIPE, O_RDONLY | O_NONBLOCK);
    write_stripe_fd = open(PORTAL_TO_STRIPE_PIPE, O_WRONLY | O_NONBLOCK);
}

AirlinePortal::~AirlinePortal() {

    close(read_fd);
    close(read_stripe_fd);
    close(write_stripe_fd);
}

void AirlinePortal::loadAVNsFromFile(const string& airline_name) {

    ifstream avn_file(airline_name + "_avns.log");
    if (!avn_file.is_open()) {

        cerr << "Warning: Could not open " << airline_name << "_avns.log for reading\n";
        return;
    }

    string line;
    while (getline(avn_file, line)) {

        if (!line.empty()) {

            AVNRecord avn = AVNRecord::deserialize(line);
            bool exists = false;
            for (const auto& existing : avns) {

                if (existing.avn_id == avn.avn_id) {

                    exists = true;
                    break;
                }
            }
            if (!exists) {

                avns.push_back(avn);
            }
        }
    }
    avn_file.close();
}

bool AirlinePortal::login(const string& aircraft_id) {

    if (aircraft_id.length() < 2) {

        cout << "Invalid aircraft ID\n";
        return false;
    }

    current_airline = aircraft_id.substr(0, 2);
    string airline_name;

    if (current_airline == "PK") {

        airline_name = "PIA";
    } 
    else if (current_airline == "AB") {

        airline_name = "AirBlue";
    } 
    else if (current_airline == "FDX") {

        airline_name = "FedEx";
    } 
    else if (current_airline == "PAF") {

        airline_name = "Pakistan_Airforce";
    } 
    else if (current_airline == "BD") {

        airline_name = "BlueDart";
    } 
    else if (current_airline == "AKA") {

        airline_name = "AghaKhan_Air_Ambulance";
    } 
    else {

        cout << "Unknown airline for aircraft ID\n";
        return false;
    }
    authenticated = true;
    loadAVNsFromFile(airline_name);

    return true;
}

void AirlinePortal::displayAVNs() const {

    cout << "\n=== AVN Records for " << current_airline << " ===\n";
    bool found = false;
    for (const auto& avn : avns) {

        if (avn.aircraft_id.substr(0, 2) == current_airline) {

            found = true;
            cout << "\nAVN ID: " << avn.avn_id
                      << "\nAircraft: " << avn.aircraft_id
                      << "\nType: ";
            switch (avn.aircraft_type) {

                case FlightType::Commercial: cout << "Commercial"; break;
                case FlightType::Cargo: cout << "Cargo"; break;
                case FlightType::Military: cout << "Military"; break;
                case FlightType::Medical: cout << "Medical"; break;
            }

            cout << "\nSpeed: " << avn.recorded_speed << " km/h"
                      << "\nIssue Date: " << put_time(localtime(&avn.issue_date), "%Y-%m-%d %H:%M:%S")
                      << "\nFine Amount: PKR " << fixed << setprecision(2) << avn.fine_amount
                      << "\nService Fee: PKR " << avn.service_fee
                      << "\nTotal: PKR " << (avn.fine_amount + avn.service_fee)
                      << "\nStatus: " << (avn.paid ? "Paid" : "Unpaid")
                      << "\nDue Date: " << put_time(localtime(&avn.due_date), "%Y-%m-%d")
                      << "\n-----------------\n";
        }
    }
    if (!found) {

        cout << "No AVNs found for this airline.\n";
    }
}

void AirlinePortal::processPayment(const string& avn_id) {

    stringstream ss;
    bool found = false;
    for (auto& avn : avns) {
        if (avn.avn_id == avn_id && avn.aircraft_id.substr(0, 2) == current_airline && !avn.paid) {
            ss << avn_id << "," 
               << avn.aircraft_id << "," 
               << (avn.fine_amount + avn.service_fee);
            write(write_stripe_fd, ss.str().c_str(), ss.str().length());
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "AVN ID not found, not owned by this airline, or already paid.\n";
    }
}

void AirlinePortal::run() {

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    char buffer[1024];
    while (keep_running && authenticated) {
        // Read new AVNs from pipe
        int n = read(read_fd, buffer, sizeof(buffer)-1);
        if (n > 0) {

            buffer[n] = '\0';
            stringstream ss(buffer);
            AVNRecord avn;
            string paid_str;
            getline(ss, avn.avn_id, ',');
            getline(ss, avn.aircraft_id, ',');
            ss >> avn.fine_amount;
            ss.ignore();
            ss >> avn.service_fee;
            ss.ignore();
            ss >> avn.issue_date;
            ss.ignore();
            getline(ss, paid_str);
            avn.paid = (paid_str == "1");

            // Map aircraft ID to airline and type
            if (avn.aircraft_id.find("PK") == 0) {

                avn.airline_name = "PIA";
                avn.aircraft_type = FlightType::Commercial;
            } 
            else if (avn.aircraft_id.find("AB") == 0) {

                avn.airline_name = "AirBlue";
                avn.aircraft_type = FlightType::Commercial;
            } 
            else if (avn.aircraft_id.find("FDX") == 0) {

                avn.airline_name = "FedEx";
                avn.aircraft_type = FlightType::Cargo;
            } 
            else if (avn.aircraft_id.find("PAF") == 0) {

                avn.airline_name = "Pakistan_Airforce";
                avn.aircraft_type = FlightType::Military;
            } 
            else if (avn.aircraft_id.find("BD") == 0) {

                avn.airline_name = "BlueDart";
                avn.aircraft_type = FlightType::Cargo;
            } 
            else if (avn.aircraft_id.find("AKA") == 0) {

                avn.airline_name = "AghaKhan_Air_Ambulance";
                avn.aircraft_type = FlightType::Medical;
            }
            avn.due_date = avn.issue_date + (3 * 24 * 60 * 60);

            bool exists = false;
            for (auto& existing : avns) {

                if (existing.avn_id == avn.avn_id) {

                    existing = avn; // Update if exists
                    exists = true;
                    break;
                }
            }
            if (!exists) {

                avns.push_back(avn);
            }
        }

        // Read payment status updates from StripePay
        n = read(read_stripe_fd, buffer, sizeof(buffer)-1);
        if (n > 0) {

            buffer[n] = '\0';
            stringstream ss(buffer);
            string avn_id, status;
            getline(ss, avn_id, ',');
            getline(ss, status);
            for (auto& avn : avns) {

                if (avn.avn_id == avn_id) {

                    avn.paid = (status == "SUCCESS");
                    break;
                }
            }
        }

        cout << "\n=== Airline Portal Menu ===\n"
                  << "1. View AVNs\n"
                  << "2. Process Payment\n"
                  << "3. Logout\n"
                  << "Choice: ";
        
        int choice;
        cin >> choice;

        switch (choice) {
            
            case 1:
                displayAVNs();
                break;
            case 2: {

                string avn_id;
                cout << "Enter AVN ID: ";
                cin >> avn_id;
                processPayment(avn_id);
                break;
            }
            case 3:

                authenticated = false;
                break;
                
            default:

                cout << "Invalid choice\n";
        }
        usleep(100000); // 100ms
    }
}