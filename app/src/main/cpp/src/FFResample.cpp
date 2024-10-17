
#include <ALog.h>
#include "FFResample.h"

extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

bool FFResample::open(XParameter in, XParameter out) {
    close();
    std::lock_guard<std::recursive_mutex> lock(mutex);
    AVCodecParameters* params = in.avCodecParameters;
    if (params == nullptr) {
        LOGE("FFResample open, null input params.");
        return false;
    }
    SwrContext *swrContext = swr_alloc();
    swrContext = swr_alloc_set_opts(swrContext,
          av_get_default_channel_layout(out.channels), AV_SAMPLE_FMT_S16, out.sample_rate,
                                    av_get_default_channel_layout(params->channels), (AVSampleFormat)params->format, params->sample_rate, 0, 0);
    if (swrContext == nullptr) {
        LOGE("FFResample swr_alloc_set_opts failed.");
        return false;
    }
    int ret = swr_init(swrContext);
    if (ret) {
        LOGE("FFResample swr_init failed, %s.", av_err2str(ret));
        return false;
    }
    this->swrContext = swrContext;
    LOGI("FFResample swr_init success.");
    return true;
}

XData FFResample::resample(XData in) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!swrContext) {
        LOGE("FFResample swrContext nullptr.");
        return XData();
    }
    if (!in.data || in.size <= 0) {
        LOGE("FFResample receive data invalid.");
        return XData();
    }
    AVFrame* avFrame = reinterpret_cast<AVFrame*>(in.data);
    int outSize = 2 * avFrame->nb_samples * av_get_bytes_per_sample(AVSampleFormat::AV_SAMPLE_FMT_S16);
    if (outSize <= 0) {
        LOGE("outsize negative.");
        return XData();
    }
    LOGI("resample convert start size %d", outSize);
    XData out;
    out.alloc(outSize);
    int lens = swr_convert(swrContext, &out.data, avFrame->nb_samples,
                           (const uint8_t **)in.datas, avFrame->nb_samples);
    if (lens <= 0) {
        out.release();
        return out;
    }
    out.pts = in.pts;
    LOGI("resample convert success %d, pts = %d", lens, out.pts);
    return out;
}

void FFResample::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if(swrContext) {
        swr_free(&swrContext);
    }
}
