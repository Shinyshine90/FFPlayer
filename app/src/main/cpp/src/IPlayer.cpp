#include "IPlayer.h"
#include "ALog.h"

#include "IDemux.h"
#include "IDecoder.h"
#include "IVideoView.h"
#include "IResample.h"
#include "IAudioPlay.h"



IPlayer *IPlayer::getInstance(unsigned char index) {
    static IPlayer p[256];
    return &p[index];
}

bool IPlayer::open(const char *path) {
    close();
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!demux || !demux->open(path)) {
        LOGE("IPlayer demux open failed.");
        return false;
    }

    if (!videoDecoder || !videoDecoder->open(demux->getVideoDecodeParams())) {
        LOGE("IPlayer video decode open failed.");
        return false;
    }

    if (!audioDecoder || !audioDecoder->open(demux->getAudioDecodeParams())) {
        LOGE("IPlayer audio decode open failed.");
        return false;
    }
    XParameter audioInParams = demux->getAudioDecodeParams();
    XParameter audioOutParams = demux->getAudioDecodeParams();
    audioOutParams.channels = 2;
    if (!resample || !resample->open(audioInParams, audioOutParams)) {
        LOGE("IPlayer audio resample open failed.");
        return false;
    }
    LOGI("IPlayer open success.");
    return true;
}

void IPlayer::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // demux 、 decode * 2
    XThread::stop();
    demux->stop();
    audioDecoder->stop();
    videoDecoder->stop();
    audioPlay->stop();
    //清理缓冲
    audioDecoder->clearBuffer();
    videoDecoder->clearBuffer();
    audioPlay->clearBuffer();
    //
    audioPlay->close();
    videoView->close();
    audioDecoder->close();
    videoDecoder->close();
    demux->close();
}

bool IPlayer::start() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    demux->start();

    videoDecoder->start();

    audioDecoder->start();

    audioPlay->start();

    XParameter parameter = demux->getAudioDecodeParams();
    parameter.channels = 2;
    audioPlay->startPlay(parameter);

    XThread::start();

    return true;
}


void IPlayer::run() {
    while (!isInterrupted) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        updatePlayStation();
        if (audioPlay && videoDecoder) {
            //LOGI("IPlayer audio pts %ld", this->audioPlay->pts);
            //LOGI("IPlayer video pts %ld", this->videoView->pts);
            videoDecoder->syncPts = audioPlay->pts;
        }
        sleep(1);
    }
}

bool IPlayer::setSurface(void *win) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    videoView->setWindow(win);
    return true;
}

void IPlayer::updatePlayStation() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!demux) {
        return;
    }
    int total = demux->durationMs;
    if (total <= 0) {
        return;
    }
    double position = videoDecoder->pts / (double)total;
    if (0.009 < abs(position - playPosition)) {
        if (callback)
            callback(position);
        playPosition = position;
    }

}

bool IPlayer::seek(double position) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!demux) {
        return false;
    }
    pause();
    LOGI("IPlayer pause complete.");

    if (audioPlay) {
        audioPlay->clearBuffer();
    }
    LOGI("IPlayer audioPlay clearBuffer complete.");

    if (audioDecoder) {
        audioDecoder->clearBuffer();
    }
    LOGI("IPlayer audioDecoder clearBuffer complete.");

    if (videoDecoder) {
        videoDecoder->clearBuffer();
    }
    LOGI("IPlayer videoDecoder clearBuffer complete.");

    if (demux)
        demux->seek(position);
    LOGI("IPlayer demux seek complete.");
    // 解码到实际需要显示的帧
    resume();
    LOGI("IPlayer resume complete.");
    return false;
}

bool IPlayer::isPause() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return XThread::isPause();
}

void IPlayer::pause() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    LOGI("IPlayer pause.");
    XThread::pause();
    if (demux) {
        demux->pause();
    }
    if (videoDecoder) {
        videoDecoder->pause();
    }
    if (audioDecoder) {
        audioDecoder->pause();
    }
    if (audioPlay) {
        audioPlay->pause();
    }
}

void IPlayer::resume() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    LOGI("IPlayer resume.");
    XThread::resume();
    if (demux) {
        demux->resume();
    }
    if (videoDecoder) {
        videoDecoder->resume();
    }
    if (audioDecoder) {
        audioDecoder->resume();
    }
    if (audioPlay) {
        audioPlay->resume();
    }
}



