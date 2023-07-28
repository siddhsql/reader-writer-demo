#include <iostream>
#include <shared_mutex>
#include <condition_variable>
#include <thread>

enum class LockMode { EXCLUSIVE, SHARED };

/**
 * this class is not thread-safe meaning an instance of the class should not be shared between multiple threads
 * which would call its methods concurrently. create a different instance of the class for each different thread to
 * simulate multiple readers/writers running concurrently.
 */
class Common {
    public:

        Common(const std::string& label, const std::string& action, int id, std::shared_mutex& rw_mutex, LockMode lock_mode)
            : rw_mutex_(rw_mutex), label_(label), action_(action), id_(id), lock_mode_(lock_mode) {
            is_started = false;
            is_finished = false;
        }
        
        void start() {
            std::thread t1([this]() { start_(); });
            t1.detach();
        }        

        void finish() {
            finish_();
        }
    
    private:
        std::condition_variable cv;
        std::mutex mx;
        std::shared_mutex& rw_mutex_;
        bool is_started;
        bool is_finished;        
        const std::string& label_;
        const std::string& action_;
        int id_;
        LockMode lock_mode_;

        void start_() {
            start_pre_lock();
            if (lock_mode_ == LockMode::EXCLUSIVE) {
                std::unique_lock<std::shared_mutex> lock(rw_mutex_); // lock will get released automatically when it goes out of scope
                start_post_lock();
            } else if (lock_mode_ == LockMode::SHARED) {
                std::shared_lock<std::shared_mutex> lock(rw_mutex_); // lock will get released automatically when it goes out of scope
                start_post_lock();
            }            
        }

        void start_pre_lock() {
            if (is_started) {
                throw std::runtime_error("duplicate call to start() is not allowed");
            }
            is_started = true;
            std::cout << label_ << " " << id_ << " is waiting for lock..." << std::endl;            
        }

        void start_post_lock() {
            std::unique_lock<std::mutex> lock {mx}; // lock will get released automatically when it goes out of scope
            std::cout << label_ << " " << id_ << " is " << action_ << "..." << std::endl;
            cv.wait(lock, [this] { return is_finished; });
            std::cout << label_ << " " << id_ << " is exiting..." << std::endl;
        }

        void finish_() {
            if (is_finished) {
                throw std::runtime_error("duplicate call to finish() is not allowed");
            }
            is_finished = true;
            std::scoped_lock lock {mx};
            cv.notify_one();
        }
};

class Reader : public Common {
    public:

        Reader(int id, std::shared_mutex& rw_mutex) : Common {"Reader", "reading", id, rw_mutex, LockMode::SHARED} {}
};

class Writer : public Common {
    public:

        Writer(int id, std::shared_mutex& rw_mutex) : Common {"Writer", "writing", id, rw_mutex, LockMode::EXCLUSIVE} {}                    
};