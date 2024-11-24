#include "FFAudioSpeaker.h"
#include "FFLog.h"
typedef AVSampleFormat format;

FFAudioSpeaker::FFAudioSpeaker() {

}

FFAudioSpeaker::~FFAudioSpeaker() {

}

void FFAudioSpeaker::init(int inSampleRate, AVSampleFormat inSampleFormat, int64_t inChannelLayout,
                          int outSampleRate, AVSampleFormat outSampleFormat,
                          int64_t outChannelLayout) {

    this->inSampleRate = inSampleRate;
    this->inSampleFormat = inSampleFormat;
    this->inChannelLayout = inChannelLayout;
    this->outSampleRate = outSampleRate;
    this->outSampleFormat = outSampleFormat;
    this->outChannelLayout = outChannelLayout;

    LOGI("FFAudioSpeaker create swr in sample, rate = %d, fmt = %d, channel = %ld.", inSampleRate, inSampleFormat, inChannelLayout);
    LOGI("FFAudioSpeaker create swr out sample, rate = %d, fmt = %d, channel = %ld.", outSampleRate, outSampleFormat, outChannelLayout);

    swrCtx = swr_alloc_set_opts(nullptr,
                                outChannelLayout, outSampleFormat,
                                outSampleRate,inChannelLayout,
                                inSampleFormat, inSampleRate,
                                0, nullptr);

    if (!swrCtx) {
        LOGE("Swr context config failed.");
        return;
    }

    if (swr_init(swrCtx) < 0) {
        LOGE("Swr context init failed.");
        swr_free(&swrCtx);
        return;
    }

    LOGE("Swr context init success.");
}


void FFAudioSpeaker::start() {
    if (openSlAudioPlay.isStart()) {
        return;
    }
    openSlAudioPlay.start();
}

void FFAudioSpeaker::convertPCM(AVFrame *aFrame) {
    if (!swrCtx) {
        LOGE("FFAudioSpeaker SwrContext is not initialized.");
        return;
    }

    LOGI("FFAudioSpeaker rate = %d, fmt = %d, channel = %ld, samples = %d", aFrame->sample_rate, aFrame->format, aFrame->channel_layout, aFrame->nb_samples);

    // 计算输出样本数（考虑重采样的延迟）
    int output_nb_samples = 1024;

    // 分配输出缓冲区
    uint8_t **output_data = new uint8_t*[1];
    output_data[0] = new uint8_t[1024 * 2];
    //int output_channels = av_get_channel_layout_nb_channels(outChannelLayout); // 输出通道数

   /* int lines[8];
    int ret = av_samples_alloc_array_and_samples(
            &output_data,    // 输出缓冲区
            lines,         // 行大小
            output_channels, // 输出通道数
            output_nb_samples, // 输出样本数
            outSampleFormat, // 输出样本格式
            0);              // 不对齐
    if (ret < 0) {
        LOGE("FFAudioSpeaker Failed to allocate output buffer: %s\n", av_err2str(ret));
        return;
    }*/

    // 执行重采样
    int ret = swr_convert(
            swrCtx,
            output_data,        // 输出缓冲区
            output_nb_samples,  // 最大输出样本数
            (const uint8_t **)aFrame->data, // 输入缓冲区
            aFrame->nb_samples);  // 输入样本数
    if (ret < 0) {
        LOGE("FFAudioSpeaker Error while resampling: %s\n", av_err2str(ret));
        av_freep(&output_data[0]);
        av_freep(&output_data);
        return;
    }

    int calSize = av_samples_get_buffer_size(NULL, 2, ret,
                               outSampleFormat, 1);
    // 获取实际输出样本数
    int converted_samples = ret;
    int expectSize = converted_samples * 2 * 2;
    LOGI("FFAudioSpeaker resample complete samples %d, expected size = %d, calSize = %d.",  converted_samples, expectSize, calSize);

    uint8_t *pcmBuffer = new uint8_t[expectSize];
    memcpy(pcmBuffer, output_data[0], expectSize);
    openSlAudioPlay.enqueuePcmData(new FFPcmData(pcmBuffer, expectSize));

    freePcm(output_data);
}

void FFAudioSpeaker::freePcm(uint8_t **pcm) {
    if (pcm) {
        av_freep(&pcm[0]);
        av_freep(&pcm);
    }
}

void FFAudioSpeaker::stop() {
    openSlAudioPlay.stop();
}

void FFAudioSpeaker::release() {
    openSlAudioPlay.stop();
    if (swrCtx) {
        swr_free(&swrCtx);
    }
}







