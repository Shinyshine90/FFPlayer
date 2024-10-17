#include "FFDecoder.h"
#include "ALog.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
}

bool FFDecoder::open(XParameter params) {
    close();
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (params.avCodecParameters == nullptr) {
        LOGE("open failed null params.");
        return false;
    }
    AVCodecParameters* codecParameters = params.avCodecParameters;
    AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (codec == nullptr) {
        LOGE("find codec failed for id %d.", codecParameters->codec_id);
        return false;
    }
    AVCodecContext* context = avcodec_alloc_context3(codec);
    if (context == nullptr) {
        LOGE("alloc codec context failed.");
        return false;
    }
    int ret = avcodec_parameters_to_context(context, codecParameters);
    if (ret < 0) {
        LOGE("params to context failed, %s.", av_err2str(ret));
        return false;
    }
    ret = avcodec_open2(context, codec, nullptr);
    if (ret) {
        LOGE("open codec failed, %s.", av_err2str(ret));
        return false;
    }
    context->thread_count = 10;
    codecContext = context;
    isAudio = codecParameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO;
    return true;
}

void FFDecoder::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    clearBuffer();
    if (frame) {
        av_frame_free(&frame);
    }
    if (codecContext) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
    }
}

bool FFDecoder::sendPacket(XData data) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (codecContext == nullptr || data.data == nullptr || data.size <= 0) {
        return false;
    }
    int ret = avcodec_send_packet(codecContext, (AVPacket *)(data.data));
    if (ret) {
        LOGE("send packet failed, %s.", av_err2str(ret));
    }
    return ret == 0;
}

XData FFDecoder::receiveFrame() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    XData data;
    if (codecContext == nullptr) {
        return data;
    }
    if (frame == nullptr) {
        frame = av_frame_alloc();
    }
    int ret = avcodec_receive_frame(codecContext, frame);
    //LOGI("FFDecoder receive frame complete");
    if (!ret) {
        data.data = (unsigned char*) frame;
        if (codecContext->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
            data.width = frame->width;
            data.height = frame->height;
            data.size = av_image_get_buffer_size(
                    static_cast<AVPixelFormat>(frame->format), frame->width, frame->height, 1);

        } else {
            data.size = av_get_bytes_per_sample(
                    static_cast<AVSampleFormat>(frame->format)) * frame->nb_samples * frame->channels;
            /*LOGI("FFDecoder decode audio frame, nb_channels %d, nb_samples %d, linesize = %d , format %d, sample_rate %d, data size = %d",
                 frame->channels, frame->nb_samples, frame->linesize[0], frame->format, frame->sample_rate, data.size);*/
        }
        //LOGI("FFDecoder receive av frame pts %d.", frame->pts);
        data.pts = frame->pts;
        memcpy(data.datas, frame->data,  8 * sizeof (void*));
    } else {
        LOGE("receive frame failed, %s.", av_err2str(ret));
    }
    return data;
}

void FFDecoder::clearBuffer() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    IDecoder::clearBuffer();

    if (codecContext) {
        avcodec_flush_buffers(codecContext);
    }
}



