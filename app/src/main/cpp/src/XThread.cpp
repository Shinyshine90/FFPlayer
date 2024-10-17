#include <thread>
#include <sstream>

#include "ALog.h"
#include "XThread.h"

void sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

unsigned long getThreadId() {
    return std::hash<std::thread::id>()(std::this_thread::get_id());
}

void XThread::runInternal() {
    isRunning = true;
    run();
    isRunning = false;
}

bool XThread::start() {
    isPaused = false;
    isInterrupted = false;
    std::thread t(&XThread::runInternal, this);
    t.detach();
    return true;
}

void XThread::stop() {
    isInterrupted = true;
    for (int i = 0; i < 200; ++i) {
        if (!isRunning) {
            return;
        }
        sleep(1);
    }
    LOGE("exit thread timeout.");
}

void XThread::pause() {
    isPaused = true;
}

void XThread::resume() {
    isPaused = false;
}

bool XThread::isPause() {
    return isPaused;
}


