#include "FFOpenSLAudioPlay.h"
#include "FFLog.h"

#include <thread>

#define TAG "FFOpenSLAudioPlay"

FFOpenSLAudioPlay::FFOpenSLAudioPlay() {
    initialize();
}

// 析构函数
FFOpenSLAudioPlay::~FFOpenSLAudioPlay() {
    release();
}

void FFOpenSLAudioPlay::bufferCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    auto *player = static_cast<FFOpenSLAudioPlay *>(context);
    player->enqueueBuffer();
}

// 将 PCM 数据填充到缓冲区
void FFOpenSLAudioPlay::enqueueBuffer() {
    LOGI(TAG, "FFOpenSLAudioPlay enqueueBuffer buffers size %ld.", buffers.size());
    if (!buffers.empty()) {
        freePcmData(buffers.front());
        buffers.pop();
    }
    // 此处用静音数据填充缓冲区，实际使用时填入有效 PCM 数据
    while (status.load() < AudioPlayStatus::RELEASE && pcmQueue.isEmpty()) {
        LOGW(TAG, "FFOpenSLAudioPlay sleep for data.");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (status.load() < AudioPlayStatus::RELEASE) {
        FFPcmData* pcm = pcmQueue.dequeue();
        LOGI(TAG, "FFOpenSLAudioPlay, enqueueBuffer size %ld, bufferLeft %ld", pcm->size, pcmQueue.size());
        SLresult ret = (*bufferQueue)->Enqueue(bufferQueue, pcm->data, pcm->size);
        buffers.push(pcm);
        if (ret) {
            LOGE(TAG, "FFOpenSLAudioPlay, enqueueBuffer failed, ret = %d.", ret);
        }
    }
}

// 初始化播放器
bool FFOpenSLAudioPlay::initialize() {
    LOGI(TAG, "FFOpenSLAudioPlay init prev thread id = %p.", std::this_thread::get_id());
    // 创建引擎
    if (slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr) != SL_RESULT_SUCCESS) {
        LOGE(TAG, "Failed to create OpenSL engine.");
        return false;
    }
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    // 创建输出混音器
    SLInterfaceID outputMixItfIds[1] = {SL_IID_ENVIRONMENTALREVERB}; // 导出混音效果设置接口
    SLboolean outputMixItfReq[1] = {SL_BOOLEAN_FALSE}; // FALSE：该接口不支持，也不影响对象的初始化流程；TRUE：接口不支持，会导致初始化失败

    if ((*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, outputMixItfIds, outputMixItfReq) != SL_RESULT_SUCCESS) {
        LOGE(TAG, "Failed to create output mix.");
        return false;
    }
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    // 配置 PCM 数据源
    SLDataLocator_AndroidSimpleBufferQueue locBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 100 // 双缓冲
    };
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,
            2,                              // 双通道
            SL_SAMPLINGRATE_44_1,           // 采样率 44100Hz
            SL_PCMSAMPLEFORMAT_FIXED_16,    // 每个样本 16 位
            SL_PCMSAMPLEFORMAT_FIXED_16,    // 每个通道 16 位
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 双声道布局
            SL_BYTEORDER_LITTLEENDIAN       // 小端字节序
    };
    SLDataSource audioSrc = {&locBufferQueue, &formatPcm};

    // 配置音频输出
    SLDataLocator_OutputMix locOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&locOutputMix, nullptr};

    // 创建播放器
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    if ((*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSink, 1, ids, req) != SL_RESULT_SUCCESS) {
        LOGE(TAG, "Failed to create audio player.");
        return false;
    }
    (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);

    // 获取播放接口
    (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);

    // 获取缓冲区队列接口
    (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &bufferQueue);

    // 注册回调
    (*bufferQueue)->RegisterCallback(bufferQueue, bufferCallback, this);

    return true;
}


// 启动播放
void FFOpenSLAudioPlay::start() {
    LOGI(TAG, "FFOpenSLAudioPlay start prev thread id = %p.", std::this_thread::get_id());
    std::lock_guard<std::mutex> lock(mutex);
    if (status.load() == AudioPlayStatus::IDLE) {
        status.store(AudioPlayStatus::PLAYING);
        (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
        (*bufferQueue)->Enqueue(bufferQueue, "", 2);
    }
}

void FFOpenSLAudioPlay::pause() {
    std::lock_guard<std::mutex> lock(mutex);
    if (status.load() == AudioPlayStatus::PLAYING) {
        status.store(AudioPlayStatus::PAUSE);
    }
}

// 停止播放
void FFOpenSLAudioPlay::stop() {
    std::lock_guard<std::mutex> lock(mutex);
    if (status.load() == AudioPlayStatus::STOP) {
        return;
    }

    if (playerPlay) {
        (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    }
    status.store(AudioPlayStatus::STOP);
}

// 释放资源
void FFOpenSLAudioPlay::release() {
    if  (status.load() == AudioPlayStatus::RELEASE) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    status.store(AudioPlayStatus::RELEASE);
    if (playerObject) {
        (*playerObject)->Destroy(playerObject);
        playerObject = nullptr;
        playerPlay = nullptr;
        bufferQueue = nullptr;
    }
    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = nullptr;
    }
    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = nullptr;
        engineEngine = nullptr;
    }
}

void FFOpenSLAudioPlay::enqueuePcmData(FFPcmData *data) {
    while (pcmQueue.isOverflow()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    pcmQueue.enqueue(data);
}

void FFOpenSLAudioPlay::freePcmData(FFPcmData *data) {
    delete data;
}

bool FFOpenSLAudioPlay::isStart() {
    std::lock_guard<std::mutex> lock(mutex);
    return status.load() == AudioPlayStatus::PLAYING;
}

bool FFOpenSLAudioPlay::isPcmQueueOverflow() {
    return pcmQueue.isOverflow();
}




