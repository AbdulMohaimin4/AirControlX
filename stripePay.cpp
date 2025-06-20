#include "stripePay.hpp"
#include "ipcPaths.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void signalHandler(int signum) {
    keep_running = 0;
}

StripePay::StripePay() {

    read_fd = open(PORTAL_TO_STRIPE_PIPE, O_RDONLY | O_NONBLOCK);
    write_avn_fd = open(STRIPE_TO_AVN_PIPE, O_WRONLY | O_NONBLOCK);
    write_portal_fd = open(STRIPE_TO_PORTAL_PIPE, O_WRONLY | O_NONBLOCK);
}

StripePay::~StripePay() {

    close(read_fd);
    close(write_avn_fd);
    close(write_portal_fd);
}

bool StripePay::validatePayment(const string& avn_id, double amount) {

    if (amount <= 0) return false;
    pending_payments[avn_id] = amount;
    return true;
}

void StripePay::notifyPaymentStatus(const string& avn_id, bool success) {

    stringstream ss;
    ss << avn_id << "," << (success ? "SUCCESS" : "FAILED");
    string msg = ss.str();
    
    write(write_avn_fd, msg.c_str(), msg.length());
    write(write_portal_fd, msg.c_str(), msg.length());
}

void StripePay::processPayment(const string& payment_data) {

    stringstream ss(payment_data);
    string avn_id, aircraft_id;
    double amount;

    getline(ss, avn_id, ',');
    getline(ss, aircraft_id, ',');
    ss >> amount;

    cout << "\n=== Processing Payment ===\n"
              << "AVN ID: " << avn_id << "\n"
              << "Aircraft: " << aircraft_id << "\n"
              << "Amount: PKR " << fixed << setprecision(2) << amount << "\n";

    bool success = validatePayment(avn_id, amount);
    notifyPaymentStatus(avn_id, success);
}

void StripePay::run() {

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    char buffer[1024];
    while (keep_running) {
        
        int n = read(read_fd, buffer, sizeof(buffer)-1);
        if (n > 0) {
            buffer[n] = '\0';
            processPayment(buffer);
        }
        usleep(100000); // 100ms
    }
}