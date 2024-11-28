#pragma once

#include "android/log.h"

#define GLOBAL_TAG "FFPlayerNative"

#define LOGV(TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE,GLOBAL_TAG "_" TAG,__VA_ARGS__)
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG,GLOBAL_TAG "_" TAG,__VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO,GLOBAL_TAG "_" TAG,__VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN,GLOBAL_TAG "_" TAG,__VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR,GLOBAL_TAG "_" TAG,__VA_ARGS__)
#define LOGF(TAG, ...) __android_log_print(ANDROID_LOG_FATAL,GLOBAL_TAG "_" TAG,__VA_ARGS__)
