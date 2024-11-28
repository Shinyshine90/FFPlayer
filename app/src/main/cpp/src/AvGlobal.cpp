
#include "AvGlobal.h"

namespace AvGlobal {

    void printAllDecoder() {
        const AVCodec *codec = NULL;
        // 遍历所有解码器
        LOGI("AvGlobal", "Supported Decoders:\n");
        while ((codec = av_codec_next(codec)) != NULL) {
            if (av_codec_is_decoder(codec)) { // 仅列出解码器
                LOGI("Decoder: %s, Type: %s\n",codec->name,
                     av_get_media_type_string(codec->type));
            }
        }
    }

    void ffmpegLogCallback(void *ptr, int level, const char *fmt, va_list vl) {
        int android_log_level;
        switch (level) {
            case AV_LOG_PANIC:
            case AV_LOG_FATAL:
                android_log_level = ANDROID_LOG_FATAL;
                break;
            case AV_LOG_ERROR:
                android_log_level = ANDROID_LOG_ERROR;
                break;
            case AV_LOG_WARNING:
                android_log_level = ANDROID_LOG_WARN;
                break;
            case AV_LOG_INFO:
                android_log_level = ANDROID_LOG_INFO;
                break;
            case AV_LOG_VERBOSE:
            case AV_LOG_DEBUG:
                android_log_level = ANDROID_LOG_DEBUG;
                break;
            default:
                android_log_level = ANDROID_LOG_DEFAULT;
                break;
        }
        __android_log_vprint(android_log_level, "FFMPEG", fmt, vl);
    }

    void freeAvPacket(AVPacket* packet) {
        if (packet) {
            av_packet_unref(packet);
            av_packet_free(&packet);
        }
    }
}