#ifndef NDKPRACTICE_FFAVSYNC_H
#define NDKPRACTICE_FFAVSYNC_H

#include <mutex>
extern "C" {
#include <libavutil/time.h>
}


class FFAvSync {
private:
    int64_t startStamp = 0;
    int64_t lastPauseStamp = 0;
    int64_t pausedTime = 0;
    std::mutex mutex;

    int64_t nowMs() {
        return av_gettime() / 1000;
    }

public:
    bool isStart() {
        return startStamp != 0;
    }

    void startSync() {
        startStamp = nowMs();
    }

    void pauseSync() {
        lastPauseStamp = nowMs();
    }

    void resumeSync() {
        if (lastPauseStamp) {
            pausedTime += nowMs() - lastPauseStamp;
        }
        lastPauseStamp = 0;
    }

    int64_t getSyncDrift() {
        return nowMs() - startStamp - pausedTime;
    }
};


#endif
