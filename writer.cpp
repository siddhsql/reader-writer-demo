#include <iostream>
#include <shared_mutex>
#include <condition_variable>
#include <thread>

/**
 * this class is not thread-safe meaning an instance of the class should not be shared between multiple threads
 * which would call its methods concurrently. create a different instance of Writer for each different thread to
 * simulate multiple writers running concurrently.
 */
class Writer {
    public:

        Writer(int id, std::shared_mutex& rw_mutex) : rw_mutex_(rw_mutex) {
            id_ = id;
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
        bool is_started;
        bool is_finished;
        std::shared_mutex& rw_mutex_;
        int id_;

        void start_() {
            if (is_started) {
                throw std::runtime_error("duplicate call to start() is not allowed");
            }
            is_started = true;
            std::cout << "Writer " << id_ << " is waiting for lock..." << std::endl;
            std::unique_lock<std::shared_mutex> rw_lock {rw_mutex_};
            std::unique_lock<std::mutex> lock {mx};
            std::cout << "Writer " << id_ << " is writing..." << std::endl;
            cv.wait(lock, [] { return true; });
            lock.unlock();
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