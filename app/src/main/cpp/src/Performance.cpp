#include "Performance.h"

void calculateFuncCost(const char *scene, std::function<void()> runnable) {
    auto startStamp = std::chrono::high_resolution_clock::now();
    runnable();
    std::chrono::duration<double, std::milli> duration =
            (std::chrono::high_resolution_clock::now() - startStamp);
    LOGI("%s execute cost = %.3f ms", scene, duration.count());
}