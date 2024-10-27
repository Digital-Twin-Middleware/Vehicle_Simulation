#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <atomic>
#include <thread>

#include "digital_twin_client.h"
#include "car_simulation.h"
#include "json_helper.h"

using namespace digital_twin;

std::atomic<bool> running(true);

void run_simulation(int id) {
    digital_twin_client client(id);

    client.connect();
    
    car_simulation car(id);
    
    car.register_data(client);
        
    car.run(client, running);
    
    client.disconnect();
}

void signal_handler(int signal) {
	running = false;
    std::cout << "Terminating simulation..." << std::endl;
}

void cleanup_children() {
    while (wait(NULL) > 0);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int max_car = json_helper<int>::get_value_or_default("simulation_max", 0);
    
    pid_t pid;
    for (int i = 0; i < max_car; i++) {
        if ((pid = fork()) == 0) {
            run_simulation(i + 1);
            exit(0);
        }
      	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    //while (true) {
        
    //}

    cleanup_children();

    return 0;
}

