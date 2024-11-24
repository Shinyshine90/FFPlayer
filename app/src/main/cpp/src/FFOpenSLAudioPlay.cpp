#include "FFOpenSLAudioPlay.h"
#include "FFLog.h"

#include <thread>

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
    // 此处用静音数据填充缓冲区，实际使用时填入有效 PCM 数据
    LOGI("FFOpenSLAudioPlay queue size = %ld, thread id = %p.", pcmQueue.size(), std::this_thread::get_id());
    while (engineObject && pcmQueue.isEmpty()) {
        LOGW("FFOpenSLAudioPlay sleep for data.");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (engineObject) {
        FFPcmData* pcm = pcmQueue.dequeue();
        (*bufferQueue)->Enqueue(bufferQueue, pcm->data, pcm->size);
        freePcmData(pcm);
    }
}

// 初始化播放器
bool FFOpenSLAudioPlay::initialize() {
    LOGI("FFOpenSLAudioPlay init prev thread id = %p.", std::this_thread::get_id());
    // 创建引擎
    if (slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr) != SL_RESULT_SUCCESS) {
        LOGE("Failed to create OpenSL engine.");
        return false;
    }
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    // 创建输出混音器
    SLInterfaceID outputMixItfIds[1] = {SL_IID_ENVIRONMENTALREVERB}; // 导出混音效果设置接口
    SLboolean outputMixItfReq[1] = {SL_BOOLEAN_FALSE}; // FALSE：该接口不支持，也不影响对象的初始化流程；TRUE：接口不支持，会导致初始化失败

    if ((*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, outputMixItfIds, outputMixItfReq) != SL_RESULT_SUCCESS) {
        LOGE("Failed to create output mix.");
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
        LOGE("Failed to create audio player.");
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

void FFOpenSLAudioPlay::run() {
    LOGI("FFOpenSLAudioPlay start thread id = %p.", std::this_thread::get_id());
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (playerPlay) {
            (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
            uint8_t silentBuffer[2] = {0};
            (*bufferQueue)->Enqueue(bufferQueue, silentBuffer, sizeof(silentBuffer));
        }
    }

    while (status < AudioPlayStatus::STOP) {
        std::unique_lock<std::mutex> lock(mutex);
        if (status == AudioPlayStatus::PAUSE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        condition.wait(lock);
        if (status == AudioPlayStatus::STOP) {
            break;
        }
    }
}

// 启动播放
void FFOpenSLAudioPlay::start() {
    LOGI("FFOpenSLAudioPlay start prev thread id = %p.", std::this_thread::get_id());
    std::lock_guard<std::mutex> lock(mutex);
    if (status == AudioPlayStatus::IDLE) {
        status = AudioPlayStatus::PLAYING;
        try {
            thread = std::thread(&FFOpenSLAudioPlay::run, this);
        } catch (const std::exception &e) {
            LOGE("Thread creation failed: %s", e.what());
            // 可以在此处捕获异常，并进行适当的资源清理
        }
    }
}

void FFOpenSLAudioPlay::pause() {
    std::lock_guard<std::mutex> lock(mutex);
    if (status == AudioPlayStatus::PLAYING) {
        status = AudioPlayStatus::PAUSE;
    }
}

// 停止播放
void FFOpenSLAudioPlay::stop() {
    std::lock_guard<std::mutex> lock(mutex);
    if (status == AudioPlayStatus::STOP) {
        return;
    }

    if (playerPlay) {
        (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    }
    status = AudioPlayStatus::STOP;
    condition.notify_one();
    if (thread.joinable()) {
        thread.join();
    }
}

// 释放资源
void FFOpenSLAudioPlay::release() {
    std::lock_guard<std::mutex> lock(mutex);
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
    pcmQueue.enqueue(data);
}

void FFOpenSLAudioPlay::freePcmData(FFPcmData *data) {
    delete data;
}

bool FFOpenSLAudioPlay::isStart() {
    std::lock_guard<std::mutex> lock(mutex);
    return status == AudioPlayStatus::PLAYING;
}




