//
// Created by Shadowalker on 2024/11/22.
//

#ifndef NDKPRACTICE_FFOPENSLAUDIOPLAY_H
#define NDKPRACTICE_FFOPENSLAUDIOPLAY_H


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <cstring>
#include <iostream>

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

class FFOpenSLAudioPlay {
private:
    // OpenSL ES 对象
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;

    SLObjectItf outputMixObject = nullptr;
    SLObjectItf playerObject = nullptr;

    SLPlayItf playerPlay = nullptr;
    SLAndroidSimpleBufferQueueItf bufferQueue = nullptr;

    // 音频缓冲区
    //static const int BUFFER_SIZE = 44100 * 2 * 2; // 1秒音频，44100Hz * 2通道 * 2字节
    //uint8_t audioBuffer[BUFFER_SIZE]; // PCM 数据缓冲区

    FFMediaQueue<FFPcmData*> pcmQueue {100000};

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

    // 启动播放
    void start();

    // 停止播放
    void stop();

    // 释放资源
    void releaseResources();


};
#endif