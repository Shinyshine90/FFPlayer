#include "FFDemux.h"
#include "ALog.h"
#include <stdio.h>

extern "C" {
#include <libavformat/avformat.h>
}

static double r2d(AVRational r) {
    return r.den == 0 || r.num == 0 ? 0.0 : r.num / (double )r.den;
}

FFDemux::FFDemux() {
    static bool init = true;
    if (init) {
        init = false;
        //注册解封装器
        av_register_all();
        //注册所有的编解码器
        avcodec_register_all();
        //初始化网络
        avformat_network_init();
        LOGI("av init complete 123.");

        AVCodec * encoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264);
        LOGI("FFDemux encoder 264 %s", encoder ? encoder->name : "");

        encoder = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_AAC);
        LOGI("FFDemux encoder aac %s", encoder ? encoder->name : "");
    }
}

bool FFDemux::open(const char *url) {
    std::lock_guard<std::recursive_mutex> lock(mutex);

    int ret = avformat_open_input(&formatContext, url, nullptr, nullptr);
    if (ret) {
        LOGE("open input failed, cause by %s.", av_err2str(ret));
        return false;
    }
    LOGI("open input success.");
    ret = avformat_find_stream_info(formatContext, nullptr);
    if (ret) {
        LOGE("find stream info failed, %s.", av_err2str(ret));
        return false;
    }
    getVideoDecodeParams();
    getAudioDecodeParams();
    durationMs = formatContext->duration / (AV_TIME_BASE/1000);
    LOGI("durationMs %ld.", durationMs);
    return true;
}

void FFDemux::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (formatContext) {
        avformat_close_input(&formatContext);
    }
}

XParameter FFDemux::getVideoDecodeParams() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    XParameter xParameter;
    if (formatContext == nullptr) {
        return xParameter;
    }
    int stream = av_find_best_stream(formatContext,
                                     AVMediaType::AVMEDIA_TYPE_VIDEO,
                                     -1, -1, 0, 0);
    if (stream < 0) {
        LOGE("find video stream index failed.");
        return xParameter;
    }
    videoStream = stream;
    AVCodecParameters * videoParams = formatContext->streams[stream]->codecpar;
    LOGI("find video codec params, video width = %d, height = %d.", videoParams->width, videoParams->height);
    xParameter.avCodecParameters = videoParams;
    return xParameter;
}

XParameter FFDemux::getAudioDecodeParams() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    XParameter xParameter;
    if (formatContext == nullptr) {

        return xParameter;
    }
    int stream = av_find_best_stream(formatContext,
                                     AVMediaType::AVMEDIA_TYPE_AUDIO,
                                     -1, -1, 0, 0);
    if (stream < 0) {
        LOGE("find video stream index failed.");

        return xParameter;
    }
    audioStream = stream;
    LOGI("find audio codec params.");
    xParameter.avCodecParameters = formatContext->streams[stream]->codecpar;
    xParameter.sample_rate = xParameter.avCodecParameters->sample_rate;
    xParameter.channels = xParameter.avCodecParameters->channels;

    return xParameter;
}

XData FFDemux::read() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (formatContext == nullptr) {
        return XData();
    }
    XData d;
    AVPacket* pkt = av_packet_alloc();
    int ret = av_read_frame(formatContext, pkt);
    if (ret) {
        LOGE("read packet failed, %s.", av_err2str(ret));
        return d;
    }
    d.data = (unsigned char *)pkt;
    d.size = pkt->size;
    pkt->pts = pkt->pts * r2d(formatContext->streams[pkt->stream_index]->time_base) * 1000;
    pkt->dts = pkt->dts * r2d(formatContext->streams[pkt->stream_index]->time_base) * 1000;
    d.pts = pkt->pts;
    if (pkt->stream_index == videoStream) {
        d.isAudio = false;
       // LOGI("video pack size = %d, pts = %ld", pkt->size, pkt->pts);
    } else if (pkt->stream_index == audioStream) {
        d.isAudio = true;
        LOGI("audio pack size = %d, pts = %ld", pkt->size, pkt->pts);
    } else {
        av_packet_free(&pkt);
        return XData();
    }

    return d;
}

bool FFDemux::seek(double position) {
    LOGI("FFDemux seek %lf", position);
    if (position < 0 || 1 < position) {
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex);
    LOGI("FFDemux seek %.2lf", position);
    if (!formatContext) {
        return false;
    }
    //清理缓冲
    avformat_flush(formatContext);

    long long seekPts = formatContext->streams[videoStream]->duration * position;

    int ret = av_seek_frame(formatContext, videoStream, seekPts,
                            AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    LOGE("FFDemux seek ret %d.", ret);
    return 0 <= ret;
}


