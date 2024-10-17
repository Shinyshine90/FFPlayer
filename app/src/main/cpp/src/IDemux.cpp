#include "IDemux.h"
#include "ALog.h"

#include <chrono>

void IDemux::run() {
    while (!this->isInterrupted) {
        if (isPaused) {
            sleep(1);
            continue;
        }
        auto startStamp = std::chrono::high_resolution_clock::now();
        LOGI("IDemux start read.");
        XData data = read();
        std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - startStamp;
        if (0 < data.size) {
            notify(data);
            duration = std::chrono::high_resolution_clock::now() - startStamp;
            LOGI("IDemux read notify complete, type = %d, cost = %.2f.", data.isAudio, duration.count());
        } else {
            break;
        }
    }
}
