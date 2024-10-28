#pragma once

#include <vector>
#include <mutex>
#include <sys/types.h>

namespace digital_twin {

	class process_manager {
		private:
    		mutable std::mutex mutex_;
    		std::vector<pid_t> pids_;

	
		public:
    		process_manager() = default;
    		~process_manager() = default;
    
    		// Delete copy constructor and assignment operator
    		process_manager(const process_manager&) = delete;
    		process_manager& operator=(const process_manager&) = delete;
    
    		// Add a new process ID to the manager
    		void add_pid(pid_t pid);
    
    		// Get a copy of all managed PIDs
    		std::vector<pid_t> get_pids() const;
    
    		// Remove a specific PID from management
    		void remove_pid(pid_t pid);
    
    		// Get the current count of managed processes
    		size_t get_process_count() const;
    
    		// Clear all PIDs (useful for cleanup)
    		void clear();
	};
}

