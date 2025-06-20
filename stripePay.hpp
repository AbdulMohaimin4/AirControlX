#ifndef STRIPE_PAY_HPP
#define STRIPE_PAY_HPP

#include <string>
#include <map>
using namespace std;

class StripePay {
    
private:
    int read_fd;
    int write_avn_fd;
    int write_portal_fd;
    std::map<std::string, double> pending_payments;

public:
    StripePay();
    ~StripePay();
    void processPayment(const std::string& payment_data);
    bool validatePayment(const std::string& avn_id, double amount);
    void notifyPaymentStatus(const std::string& avn_id, bool success);
    void run();
};

#endif