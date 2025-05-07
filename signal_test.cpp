#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include <unistd.h> // for sleep()
#include <chrono>
using namespace std;
using namespace chrono;

// Shared event queue + mutex
queue<string> eventQueue;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

// Global start time for timing
steady_clock::time_point startTime;

// Get time since simulation start
string getElapsedTime() {
    auto now = steady_clock::now();
    auto sec = chrono::duration_cast<chrono::seconds>(now - startTime).count();
    return "[" + to_string(sec) + "s]";
}

// Simulation thread function
void* SimulationThread(void* arg) {
    int counter = 1;
    while (counter <= 5) {
        sleep(1);  // simulate work

        string event = getElapsedTime() + " Aircraft PK00" + to_string(counter) + " took off";

        pthread_mutex_lock(&queueMutex);
        eventQueue.push(event);
        pthread_mutex_unlock(&queueMutex);

        cout << getElapsedTime() << " [Simulation] Generated: " << event << endl;

        counter++;
    }
    return NULL;
}

// Render thread function
void* RenderThread(void* arg) {
    while (true) {
        sleep(1); // simulate rendering

        pthread_mutex_lock(&queueMutex);
        while (!eventQueue.empty()) {
            string evt = eventQueue.front();
            eventQueue.pop();
            cout << getElapsedTime() << " [Renderer] Processing: " << evt << endl;
        }
        pthread_mutex_unlock(&queueMutex);
    }
    return NULL;
}

int main() {
    pthread_t simThread, renderThread;

    // Start timer
    startTime = steady_clock::now();

    // Create threads
    pthread_create(&simThread, NULL, SimulationThread, NULL);
    pthread_create(&renderThread, NULL, RenderThread, NULL);

    // Wait for simulation to finish
    pthread_join(simThread, NULL);

    // Stop render thread after simulation ends (for demo purposes)
    pthread_cancel(renderThread);
    pthread_join(renderThread, NULL);

    cout << "\n[Main] Simulation complete.\n";
    return 0;
}
