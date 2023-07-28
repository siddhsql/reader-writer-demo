#include <iostream>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <ncurses.h>
#include <cstdio> // For C string formatting functions

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
            t1.detach();    // permits the thread to execute independently from the thread handle
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
        const std::string label_;   // do not use const std::string& here. see: https://stackoverflow.com/questions/76789682/c-weird-behavior-with-stdstring
        const std::string action_;
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
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "%s %d is waiting for lock...\n", label_.c_str(), id_);
            printw(buffer);
        }

        void start_post_lock() {
            std::unique_lock<std::mutex> lock {mx}; // lock will get released automatically when it goes out of scope
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "%s %d is %s...\n", label_.c_str(), id_, action_.c_str());
            printw(buffer);
            cv.wait(lock, [this] { return is_finished; });
            snprintf(buffer, sizeof(buffer), "%s %d is exiting...\n", label_.c_str(), id_);
            printw(buffer);            
        }

        void finish_() {
            if (is_finished) {
                throw std::runtime_error("duplicate call to finish() is not allowed");
            }
            is_finished = true;
            std::scoped_lock lock {mx}; // lock will get released automatically when it goes out of scope
            cv.notify_one();    // wake up the other thread waiting on this cv
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