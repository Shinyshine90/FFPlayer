#ifndef NDKPRACTICE_FFMEDIABLOCKINGQUEUE_H
#define NDKPRACTICE_FFMEDIABLOCKINGQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class FFMediaQueue {
public:
    FFMediaQueue(int maxCapacity = 100){
        this->maxCapacity = maxCapacity;
    }

    ~FFMediaQueue(){}

    void enqueue(const T& t) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(t);
    }

    T peek() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.front();
    }

    T dequeue() {
        std::lock_guard<std::mutex> lock(mutex);
        T t = queue.front();
        if (t) {
            queue.pop();
        }
        return t;
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    bool isOverflow() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size() >= maxCapacity;
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        while (!queue.empty()) {
            queue.pop();
        }
    }

private:
    int maxCapacity;
    std::mutex mutex;
    std::queue<T> queue;
};

#endif
