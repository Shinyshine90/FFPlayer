
#ifndef NDKPRACTICE_FFAUDIOSPEAKER_H
#define NDKPRACTICE_FFAUDIOSPEAKER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include "FFOpenSLAudioPlay.h"
#include "FFMediaQueue.h"

class FFAudioSpeaker {

private:
    SwrContext *swrCtx = nullptr;

    FFOpenSLAudioPlay openSlAudioPlay;

    int inSampleRate, outSampleRate;
    int64_t inChannelLayout, outChannelLayout;
    AVSampleFormat inSampleFormat, outSampleFormat;

public:
    FFAudioSpeaker();

    ~FFAudioSpeaker();

    void init(int inSampleRate, AVSampleFormat inSampleFormat, int64_t inChannelLayout,
              int outSampleRate, AVSampleFormat outSampleFormat,
              int64_t outChannelLayout);

    void start();

    void convertPCM(AVFrame* aFrame);

    void freePcm(uint8_t** pcm);

    void stop();

    void release();
};

#endif