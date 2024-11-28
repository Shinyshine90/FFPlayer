#include "FFDemux.h"
#include "ThreadGlobal.h"
#include "AvGlobal.h"

#define TAG "FFDemux"

FFDemux::FFDemux() {}

FFDemux::~FFDemux() {}

void FFDemux::setUrl(const char *uri) {
    this->url = uri;
}

bool FFDemux::prepare() {
    std::lock_guard<std::mutex> lock(mutex);
    ic = avformat_alloc_context();
    if (this->url == nullptr) {
        LOGF(TAG, "init codec null url.");
        return false;
    }
    if (avformat_open_input(&ic, url, nullptr, nullptr)) {
        LOGF(TAG, "init codec failed, avformat open input failed.");
        return false;
    }

    if (avformat_find_stream_info(ic, nullptr) < 0) {
        LOGF(TAG, "init codec failed, find stream info failed.");
        return false;
    }

    videoStream = av_find_best_stream(ic,AVMEDIA_TYPE_VIDEO,
                                      -1, -1, nullptr, 0);

    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
                                      -1, -1, nullptr, 0);

    LOGI(TAG, "init code, init avformat complete, as = %d, vs = %d.", audioStream, videoStream);

    return 0 <= videoStream || audioStream <= 0;
}

void FFDemux::run() {
    while (isInterrupt.load()) {
        if (isPaused.load()) {
            ThreadGlobal::sleep(10);
            continue;
        }
        if (isFileEOF.load()) {
            ThreadGlobal::sleep(10);
            continue;
        }
        readMediaPacket();
    }
}

void FFDemux::start() {
    std::lock_guard<std::mutex> lock(mutex);
    if (isThreadStart.load()) {
        isPaused.store(false);
    } else {
        thread = std::thread(&FFDemux::run, this);
    }
}

void FFDemux::pause() {
    std::lock_guard<std::mutex> lock(mutex);
    isPaused.store(true);
}

void FFDemux::stop() {
    std::lock_guard<std::mutex> lock(mutex);
    isInterrupt.store(true);
    isThreadStart.store(false);
    if (thread.joinable()) {
        thread.join();
    }
}

void FFDemux::release() {
    std::lock_guard<std::mutex> lock(mutex);
    if (ic) {
        avformat_close_input(&ic);
    }
}

void FFDemux::registerAvPacketCallback(FFMediaType type, OnReceiveAvPacket callback) {
    std::lock_guard<std::mutex> lock(mutex);
    callbacks[type] = callback;
}

void FFDemux::unregisterAvPacketCallback(FFMediaType type, OnReceiveAvPacket callback) {
    std::lock_guard<std::mutex> lock(mutex);
    callbacks.erase(type);
}

void FFDemux::readMediaPacket() {
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        LOGE(TAG, "alloc packet failed.");
        return;
    }
    int ret = av_read_frame(ic, packet);
    if (ret == 0) {
        if (packet->stream_index == videoStream) {
            AVPacket* copy = av_packet_alloc();
            if (!av_packet_ref(copy, packet)) {
                dispatchAvPacket(FFMediaType::VIDEO, copy);
            } else {
                AvGlobal::freeAvPacket(packet);
            }
        } else if (packet->stream_index == audioStream) {
            AVPacket* copy = av_packet_alloc();
            if (!av_packet_ref(copy, packet)) {
                dispatchAvPacket(FFMediaType::AUDIO, copy);
            } else {
                AvGlobal::freeAvPacket(packet);
            }
        }
    } else if (ret < 0) {
        if (!isFileEOF.load() && ret == AVERROR_EOF) {
            isFileEOF.store(true);
        }
    }
    AvGlobal::freeAvPacket(packet);
}

void FFDemux::dispatchAvPacket(FFMediaType type, AVPacket *packet) {
    OnReceiveAvPacket callback = callbacks[type];
    if (callback) {
        callback(packet);
    }
}

const AVFormatContext *FFDemux::getIc() {
    std::lock_guard<std::mutex> lock(mutex);
    return ic;
}

int FFDemux::getAudioStream() {
    return audioStream;
}

int FFDemux::getVideoStream() {
    return videoStream;
}

void onReceivePlayStatusChanged(FFPlayState status) {
    switch (status) {
        case FFPlayState::INIT:
            break;
        case FFPlayState::PREPARE:
            break;
        case FFPlayState::PLAYING:
            break;
        case FFPlayState::PAUSE:
            break;
        case FFPlayState::SEEK:
            break;
        case FFPlayState::STOP:
            break;
        case FFPlayState::RELEASE:
            break;
    }
}









