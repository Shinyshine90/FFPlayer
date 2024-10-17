#include "IDecoder.h"
#include "ALog.h"

const char* IDecoder::getMediaTypeName() {
    if (isAudio) {
        return "Audio";
    } else {
        return "Video";
    }
}

void IDecoder::update(XData pkt) {
    if (pkt.isAudio != isAudio) {
        return;
    }
    auto start = std::chrono::high_resolution_clock::now();
    LOGI("IDecoder, update media type %s thread %ld,decoder receive packet size %d.", getMediaTypeName(),getThreadId(), pkt.size);
    while (!isInterrupted) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (packs.size() < maxBufferSize) {
            packs.push_back(pkt);
            break;
        }
        sleep(1);
    }
    std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start;
    LOGI("IDecoder, update complete, isAudio = %d, cost = %.2f.", pkt.isAudio, duration.count());
}

void IDecoder::run() {
    while(!isInterrupted) {
        if (isPaused) {
            sleep(1);
            continue;
        }
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!isAudio && 0 < syncPts) {
            if (syncPts < pts) {
                sleep(1);
                continue;
            }
        }
        if (packs.empty()) {
            sleep(1);
            continue;
        }
        XData packet = packs.front();
        packs.pop_front();
        bool send = sendPacket(packet);
        if (send) {
            while (!isInterrupted) {
                XData frame = receiveFrame();
                if (frame.data == nullptr) {
                    break;
                }
                pts = frame.pts;
                LOGI("IDecoder receive frame complete. %ld, %s decoder receive frame size = %d", getThreadId() ,getMediaTypeName(), frame.size);
                notify(frame);
            }
        }
        packet.release();
    }
}

void IDecoder::clearBuffer() {
    LOGI("IDecoder clearBuffer invoked.");
    std::lock_guard<std::recursive_mutex> lock(mutex);
    LOGI("IDecoder clearBuffer acquired lock.");
    while (!packs.empty()) {
        XData data = packs.front();
        data.release();
        packs.pop_front();
    }
    pts = 0;
    syncPts = 0;
    LOGI("IDecoder clearBuffer release lock.");
}

