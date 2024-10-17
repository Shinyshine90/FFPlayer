#include "SLAudioPlay.h"

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "ALog.h"

static SLObjectItf engineSL = nullptr;
static SLEngineItf eng = nullptr;
static SLObjectItf mix = nullptr;
static SLObjectItf player = nullptr;
static SLPlayItf iplayer = nullptr;
static SLAndroidSimpleBufferQueueItf pcmQue = nullptr;

static void PcmCall(SLAndroidSimpleBufferQueueItf bf, void *slAudioPlay) {
    LOGI("SLAudioPlay PcmCall");
    SLAudioPlay *ap = (SLAudioPlay *) slAudioPlay;
    if (!ap) {
        LOGE("SLAudioPlay PcmCall failed contex is null!");
        return;
    }
    ap->playCall((void *) bf);
}

void SLAudioPlay::playCall(void *slBufferQueue) {
    LOGI("SLAudioPlay playCall.");
    std::lock_guard<std::recursive_mutex> lock(mutex);
    SLAndroidSimpleBufferQueueItf bufferQueueItf = (SLAndroidSimpleBufferQueueItf) slBufferQueue;
    XData data = acquireData();
    if (data.size <= 0) {
        LOGE("SLAudioPlay play call invalid data.");
    }
    if (!buffer) {
        return;
    }
    memcpy(buffer, data.data, data.size);
    (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, data.size);
    data.release();
    LOGI("SLAudioPlay playCall complete.");
}

static SLEngineItf CreateSL() {
    SLresult re;
    SLEngineItf en;
    re = slCreateEngine(&engineSL, 0, 0, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    re = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    re = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &en);
    if (re != SL_RESULT_SUCCESS) return nullptr;
    return en;
}

void SLAudioPlay::run() {

}

bool SLAudioPlay::startPlay(XParameter out) {
    close();
    std::lock_guard<std::recursive_mutex> lock(mutex);
    //1 创建引擎
    eng = CreateSL();
    if (eng) {
        LOGI("CreateSL success！ ");
    } else {
        LOGE("CreateSL failed！ ");
        return false;
    }
    //2 创建混音器
    SLresult re = 0;
    re = (*eng)->CreateOutputMix(eng, &mix, 0, 0, 0);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("SL_RESULT_SUCCESS failed!");
        return false;
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("(*mix)->Realize failed!");
        return false;
    }
    SLDataLocator_OutputMix outmix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSink = {&outmix, 0};

    //3 配置音频信息
    //缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue que = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    LOGI("SLAudioPlay audio config, channels %d, sample rate %d.", out.channels, out.sample_rate);
    //音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            (unsigned int) out.channels,
            (unsigned int) out.sample_rate * 1000,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN //字节序，小端
    };
    SLDataSource ds = {&que, &pcm};


    //4 创建播放器
    const SLInterfaceID ids[] = {SL_IID_PLAY, SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng, &player, &ds, &audioSink,
                                   sizeof(ids) / sizeof(SLInterfaceID), ids, req);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer failed!");
        return false;
    } else {
        LOGI("CreateAudioPlayer success!");
    }
    (*player)->Realize(player, SL_BOOLEAN_FALSE);
    //获取player接口
    re = (*player)->GetInterface(player, SL_IID_PLAY, &iplayer);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("GetInterface SL_IID_PLAY failed!");
        return false;
    }
    re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("GetInterface SL_IID_BUFFERQUEUE failed!");
        return false;
    }

    //设置回调函数，播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue, PcmCall, this);

    //设置为播放状态
    (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_PLAYING);

    //启动队列回调
    (*pcmQue)->Enqueue(pcmQue, "", 1);
    LOGI("SLAudioPlay::StartPlay success!");
    return true;
}

SLAudioPlay::SLAudioPlay() {
    buffer = new unsigned char[1024 * 1024];
}

SLAudioPlay::~SLAudioPlay() {
    for (auto &data: slBuffer) {
        data.release();
    }
    delete buffer;
}

void SLAudioPlay::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    clearBuffer();
    if (iplayer && (*iplayer)) {
        (*iplayer)->SetPlayState(iplayer, SL_PLAYSTATE_STOPPED);
        iplayer = nullptr;
    }
    if (pcmQue && *pcmQue) {
        (*pcmQue)->Clear(pcmQue);
        pcmQue = nullptr;
    }
    if (player && (*player)) {
        (*player)->Destroy(player);
        player = nullptr;
    }
    if (mix && (*mix)) {
        (*mix)->Destroy(mix);
        mix = nullptr;
    }
    if (engineSL && (*engineSL)) {
        (*engineSL)->Destroy(engineSL);
        engineSL = nullptr;
    }
}




