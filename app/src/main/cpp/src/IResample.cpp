
#include "IResample.h"
#include "XData.h"
#include "ALog.h"

void IResample::update(XData data) {
    auto start = std::chrono::high_resolution_clock::now();
    XData audioData = resample(data);
    std::chrono::duration<double, std::milli> duration = std::chrono::high_resolution_clock::now() - start;
    LOGI("IResample update resample complete cost %.2f", duration.count());
    if (audioData.size > 0) {
        this->notify(audioData);
    }
    duration = std::chrono::high_resolution_clock::now() - start;
    LOGI("IResample update resample notify complete cost %.2f", duration.count());
}