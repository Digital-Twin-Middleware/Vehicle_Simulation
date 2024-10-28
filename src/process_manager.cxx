#include <algorithm>

#include "process_manager.h"

namespace digital_twin {

	void process_manager::add_pid(pid_t pid) {
    	std::lock_guard<std::mutex> lock(mutex_);
    	pids_.push_back(pid);
	}

	std::vector<pid_t> process_manager::get_pids() const {
    	std::lock_guard<std::mutex> lock(mutex_);
    	return pids_;
	}

	void process_manager::remove_pid(pid_t pid) {
    	std::lock_guard<std::mutex> lock(mutex_);
    	pids_.erase(std::remove(pids_.begin(), pids_.end(), pid), pids_.end());
	}

	size_t process_manager::get_process_count() const {
    	std::lock_guard<std::mutex> lock(mutex_);
    	return pids_.size();
	}

	void process_manager::clear() {
    	std::lock_guard<std::mutex> lock(mutex_);
    	pids_.clear();
	}

}
