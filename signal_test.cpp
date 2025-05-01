#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
using namespace std;

void my_handler(int signum) {
    cout << "Parent received signal " << signum << ". Ignoring it." << endl;
}

int main () {
    signal(SIGUSR1, my_handler); // Parent sets handler before fork

    pid_t pid = fork();

    if (pid == 0) { // Child process
        cout << "Child process PID: " << getpid() << endl;
        pid_t parent_pid = getppid(); // Get parent's PID
        while (true) {
            sleep(1);
            kill(parent_pid, SIGUSR1); // Send signal to parent
        }
    } else { // Parent process
        cout << "Parent process PID: " << getpid() << endl;
        while (true) {
            pause(); // Wait for signals
        }
    }

    return 0;
}
