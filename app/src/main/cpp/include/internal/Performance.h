

#ifndef NDKPRACTICE_PERFORMANCE_H
#define NDKPRACTICE_PERFORMANCE_H

#include <chrono>
#include <ALog.h>
#include <functional>

void calculateFuncCost(const char* scene, std::function<void()> runnable);

#endif
