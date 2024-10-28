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
#include "process_manager.h"

using namespace digital_twin;

namespace {
    std::atomic<bool> running(true);
    std::atomic<int> active_children(0);
    std::atomic<int> index_counter(0);
    process_manager manager;
}

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
    
    // Get a copy of PIDs under lock
    auto pids = manager.get_pids();
    
    // Terminate all child processes
    for (pid_t pid : pids) {
        kill(pid, SIGTERM);
    }
}

bool launch_simulation() {
    int id = ++index_counter;
    pid_t pid = fork();
    
    if (pid == -1) {
        std::cerr << "Fork failed" << std::endl;
        return false;
    }
    
    if (pid == 0) {
        // Child process
        signal(SIGINT, SIG_IGN);
        signal(SIGTERM, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        
        run_simulation(id);
        _exit(0);  // Use _exit() in forked child
    } else {
        // Parent process
        manager.add_pid(pid);
        active_children++;
        return true;
    }
}

void cleanup_children() {
    int status;
    pid_t pid;
    
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        manager.remove_pid(pid);
        active_children--;
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            std::cerr << "Child " << pid << " exited with status " << WEXITSTATUS(status) << std::endl;
        }
        if (WIFSIGNALED(status)) {
            std::cerr << "Child " << pid << " killed by signal " << WTERMSIG(status) << std::endl;
        }
    }
}

void sigchld_handler(int signal) {
    // Minimize work in signal handler
    // Just set a flag that main loop will check
    running = running.load() && (active_children.load() > 0);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGINT, &sa, nullptr) == -1 ||
        sigaction(SIGTERM, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up signal handlers" << std::endl;
        return 1;
    }
    
    sa.sa_handler = sigchld_handler;
    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up SIGCHLD handler" << std::endl;
        return 1;
    }
    
    const int max_car = json_helper<int>::get_value_or_default("simulation_max", 0);
    if (max_car <= 0) {
        std::cerr << "Invalid max_car value" << std::endl;
        return 1;
    }
    
    // Initial launch of simulations
    for (int i = 0; i < max_car && running; i++) {
        if (!launch_simulation()) {
            std::cerr << "Failed to launch simulation " << i << std::endl;
            continue;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Main loop
    while (running) {
        cleanup_children();
        
        // Launch new simulations if needed
        while (running && active_children < max_car) {
            if (!launch_simulation()) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Cleanup phase
    auto pids = manager.get_pids();
    for (pid_t pid : pids) {
        kill(pid, SIGTERM);
    }
    
    // Wait for all children to terminate
    while (active_children > 0) {
        cleanup_children();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}
