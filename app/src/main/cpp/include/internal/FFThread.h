#ifndef NDKPRACTICE_FFTHREAD_H
#define NDKPRACTICE_FFTHREAD_H

#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "FFLog.h"

class FFThread {
private:

    std::thread thread;

    std::atomic<bool> isInterrupt {false};

    std::deque<std::function<void()>> tasks;

    std::mutex mutex;

    std::condition_variable condition;

    void run() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this](){ return isInterrupt || !tasks.empty();});
            if (isInterrupt) {
                break;
            }
            tasks.front()();
            tasks.pop_front();
        }
    }

public:
    FFThread() {}

    virtual ~FFThread() {}

    void start() {
        thread = std::thread(&FFThread::run, this);
        thread.detach();
    }

    void post(std::function<void()> runnable) {
        if (isInterrupt) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(mutex);
            tasks.push_back(runnable);
        }
        condition.notify_one();
    }

    void stop() {
        if (isInterrupt) return;
        {
            clear();
            isInterrupt = true;
        }
        condition.notify_one();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.clear();
    }

};

#endif
