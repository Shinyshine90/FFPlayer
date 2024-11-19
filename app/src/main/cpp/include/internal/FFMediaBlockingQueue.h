#ifndef NDKPRACTICE_FFMEDIABLOCKINGQUEUE_H
#define NDKPRACTICE_FFMEDIABLOCKINGQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class FFMediaBlockingQueue {
public:
    FFMediaBlockingQueue(int maxCapacity = 200){
        this->maxCapacity = maxCapacity;
    }

    ~FFMediaBlockingQueue(){}

    void enqueue(const T& t) {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this](){ return this->queue.size() < maxCapacity; });
        queue.push(t);
        condition.notify_all();
    }

    T dequeue() {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this](){ return !this->queue.empty(); });
        T t = queue.front();
        queue.pop();
        condition.notify_all();
        return t;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

private:
    int maxCapacity;
    std::mutex mutex;
    std::condition_variable condition;
    std::queue<T> queue;
};

#endif
