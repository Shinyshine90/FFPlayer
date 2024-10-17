
#include "IAudioPlay.h"
#include "ALog.h"
#include "Performance.h"

void IAudioPlay::update(XData data) {
    if (!data.data || data.size <= 0) {
        return;
    }
    while (!isInterrupted) {
        LOGI("IAudioPlay update");
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (frames.size() < maxFrames) {
            LOGI("IAudioPlay insert new frame size %d", data.size);
            frames.push_back(data);
            break;
        } else {
            LOGI("IAudioPlay update loop");
            sleep(1);
        }
    }
}

XData IAudioPlay::acquireData() {
    XData d;
    while(!isInterrupted) {
        if (isPaused) {
            sleep(1);
            continue;
        }
        mutex.lock();
        if (frames.empty()) {
            mutex.unlock();
            sleep(10);
            LOGI("IAudioPlay acquire loop");
            continue;
        } else {
            XData data = frames.front();
            frames.pop_front();
            pts = data.pts;
            LOGI("IAudioPlay remaining frames %d, pts %ld", frames.size(), pts);
            mutex.unlock();
            return data;
        }
    }
    return d;
}

void IAudioPlay::run() {

}

std::list<XData> IAudioPlay::acquireAllData() {
    while(!isInterrupted) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (frames.empty()) {
            sleep(1);
        } else {
            std::list<XData> datas = frames;
            frames.clear();
            return datas;
        }
    }
    return std::list<XData>();
}

void IAudioPlay::clearBuffer() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    while (!frames.empty()) {
        XData frame = frames.front();
        frame.release();
        frames.pop_front();
    }
    pts = 0;
}
