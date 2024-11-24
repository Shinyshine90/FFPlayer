//
// Created by Shadowalker on 2024/11/22.
//

#ifndef NDKPRACTICE_FFOPENSLAUDIOPLAY_H
#define NDKPRACTICE_FFOPENSLAUDIOPLAY_H


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <cstring>
#include <iostream>
#include <thread>

#include "FFMediaQueue.h"

class FFPcmData {
public:
    uint8_t* data;
    size_t size;

    FFPcmData(uint8_t* data,size_t size) : data(data), size(size) {}

    ~FFPcmData() {
        delete data;
    }
};

enum class AudioPlayStatus {
    IDLE,
    PLAYING,
    PAUSE,
    STOP
};

class FFOpenSLAudioPlay {
private:
    // OpenSL ES 对象
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;

    SLObjectItf outputMixObject = nullptr;
    SLObjectItf playerObject = nullptr;

    SLPlayItf playerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf bufferQueue = nullptr;

    AudioPlayStatus status = AudioPlayStatus::IDLE;

    std::thread thread;

    std::mutex mutex;

    std::condition_variable condition;

    FFMediaQueue<FFPcmData*> pcmQueue {100};

    // PCM 数据回调（静态方法以适配 OpenSL 回调机制）
    static void bufferCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

    // 将 PCM 数据填充到缓冲区
    void enqueueBuffer();

    void freePcmData(FFPcmData* data);

public:
    // 构造函数
    FFOpenSLAudioPlay();

    // 析构函数
    ~FFOpenSLAudioPlay();

    // 初始化播放器
    bool initialize();

    void enqueuePcmData(FFPcmData* data);

    bool isStart();

    // 启动播放
    void start();

    void pause();

    // 停止播放
    void stop();

    void run();

    // 释放资源
    void release();


};
#endif