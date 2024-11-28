#ifndef NDKPRACTICE_FFDEMUX_H
#define NDKPRACTICE_FFDEMUX_H

#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_map>

#include "FFLog.h"
#include "FFPlayState.h"
#include "FFMediaQueue.h"
#include "FFMediaType.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

typedef void(*OnReceiveAvPacket)(AVPacket*);

class FFDemux {
private:

    const char* url = nullptr;

    AVFormatContext* ic = nullptr;

    int audioStream = -1, videoStream = -1;

    std::mutex mutex;

    std::unordered_map<int, OnReceiveAvPacket> callbacks;

    std::thread thread;

    std::atomic<bool> isInterrupt {false};

    std::atomic<bool> isThreadStart {false};

    std::atomic<bool> isPaused {false};

    std::atomic<bool> isFileEOF {false};

    void run();

    void readMediaPacket();

    void dispatchAvPacket(FFMediaType type, AVPacket* packet);

public:
    FFDemux();

    ~FFDemux();

    void setUrl(const char* url);

    void registerAvPacketCallback(FFMediaType type, OnReceiveAvPacket callback);

    void unregisterAvPacketCallback(FFMediaType type, OnReceiveAvPacket callback);

    //void onReceivePlayStatusChanged(FFPlayState status);

    bool prepare();

    void start();

    void pause();

    void stop();

    void release();

    const AVFormatContext* getIc();

    int getAudioStream();

    int getVideoStream();

};


#endif
