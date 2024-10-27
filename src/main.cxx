#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#include "digital_twin_client.h"
#include "car_simulation.h"
#include "json_helper.h"

using namespace digital_twin;

std::atomic<bool> running(true);
std::vector<pid_t> children_pids;
std::atomic<int> active_children(0);
std::atomic<int> index_counter(0);

void run_simulation(int id) {
    digital_twin_client client(id);
    client.connect();
    
    car_simulation car(id);
    car.register_data(client);
    car.run(client, running);
    
    client.disconnect();
}

void signal_handler(int signal) {
    std::cout << "Terminating simulation..." << std::endl;
    
    for (pid_t pid : children_pids) {
        kill(pid, SIGTERM);
    }
    
    exit(1);
}

void launch_simulation() {
    int id = ++index_counter;
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_IGN); 
        run_simulation(id);
        exit(0);
    } else if (pid > 0) {
        children_pids.push_back(pid);
        active_children++;
    }
}

void cleanup_children() {
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {
        active_children--;
    }
}

void sigchld_handler(int signal) {
    cleanup_children();
    
    if (running) launch_simulation();
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, sigchld_handler); 

    int max_car = json_helper<int>::get_value_or_default("simulation_max", 0);

    for (int i = 0; i < max_car; i++) {
        launch_simulation();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    while(running) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
    }

    cleanup_children();

    return 0;
}

