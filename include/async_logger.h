#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <queue>

namespace quote {

class AsyncLogger {
public:
    AsyncLogger(const std::string& filename) 
        : filename_(filename), running_(true) {
        worker_ = std::thread(&AsyncLogger::flush_loop, this);
    }

    ~AsyncLogger() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        cv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void log(const std::string& msg) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(msg);
        }
        cv_.notify_one();
    }

private:
    void flush_loop() {
        std::ofstream file(filename_);
        std::queue<std::string> local_queue;

        while (true) {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !queue_.empty() || !running_; });

                if (queue_.empty() && !running_) {
                    break;
                }

                std::swap(local_queue, queue_);
            }

            while (!local_queue.empty()) {
                file << local_queue.front();
                local_queue.pop();
            }
            file.flush();
        }
    }

    std::string filename_;
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_;
    bool running_;
};

} // namespace quote
