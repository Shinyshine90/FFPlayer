#include "FFCodecHandler.h"
#include "FFLog.h"

#include <thread>
#include <atomic>
#include <chrono>

int vPacketSizeTotal = 0;

std::atomic<bool> demuxThreadRunning(false);

std::atomic<bool> vDecodeThreadRunning(false);

std::atomic<bool> aDecodeThreadRunning(false);

void android_ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vl) {
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

FFCodecHandler::FFCodecHandler() {
    av_register_all();
    avformat_network_init();
    //av_log_set_callback(android_ffmpeg_log_callback);
    LOGI("FFmpeg version info %s.", av_version_info());
}

FFCodecHandler::~FFCodecHandler() {}

void FFCodecHandler::SetMediaPath(const char *url) {
    this->url = url;
}

int FFCodecHandler::InitCodec() {
    ic = avformat_alloc_context();
    if (this->url == nullptr) {
        LOGF("init codec null url.");
        return -1;
    }
    if (avformat_open_input(&ic, url, nullptr, nullptr)) {
        LOGF("init codec failed, avformat open input failed.");
        return -1;
    }

    if (avformat_find_stream_info(ic, nullptr) < 0) {
        LOGF("init codec failed, find stream info failed.");
        return -1;
    }
    LOGI("init code, init avformat complete.");
    //av_dump_format(ic, 0, url, 0);
    videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    LOGI("avformat, found video stream %d, audio stream %d.", videoStream, audioStream);
    if (0 <= videoStream) {
        AVCodec* videoCodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
        if (videoCodec) {
            videoCodecCtx = avcodec_alloc_context3(videoCodec);
            avcodec_parameters_to_context(videoCodecCtx, ic->streams[videoStream]->codecpar);
            if (avcodec_open2(videoCodecCtx, videoCodec, nullptr)) {
                LOGE("video codec open failed.");
            }
        } else {
            LOGE("avformat, found no codec for video stream.");
        }
    }
    if (0 <= audioStream) {
        AVCodec* audioCodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
        if (audioCodec) {
            audioCodecCtx = avcodec_alloc_context3(audioCodec);
            avcodec_parameters_to_context(audioCodecCtx, ic->streams[audioStream]->codecpar);
            if (avcodec_open2(audioCodecCtx, audioCodec, nullptr)) {
                LOGE("audio codec open failed.");
            }
        } else {
            LOGE("avformat, found no codec for audio stream.");
        }
    }
    //LOGI("avcodec, vCtx = %p, aCtx = %p.", videoCodecCtx, audioCodecCtx);
    //LOGI("video info, width = %d, height = %d.", videoCodecCtx->width, videoCodecCtx->height);

    vTimeBase = ic->streams[videoStream]->time_base;
    aTimeBase = ic->streams[audioStream]->time_base;
    return 0;
}

void FFCodecHandler::UnInitCodec() {
    demuxThreadRunning = false;
    if (videoCodecCtx) {
        avcodec_close(videoCodecCtx);
    }
    if (audioCodecCtx) {
        avcodec_close(audioCodecCtx);
    }
    if (ic) {
        avformat_close_input(&ic);
    }

    videoPackets.clear();
    audioPackets.clear();
    demuxFileEOF = false;
}

void FFCodecHandler::StartPlayVideo() {
    startMediaProcessThread();
}

void FFCodecHandler::StopPlayVideo() {

}

void FFCodecHandler::SetStatusPlay() {

}

void FFCodecHandler::SetStatusPause() {

}

void FFCodecHandler::Seek(float position) {

}

void FFCodecHandler::GetMediaTotalSeconds() {

}

int FFCodecHandler::GetPlayStatus() {
    return 0;
}


void FFCodecHandler::startMediaProcessThread() {
    demuxThreadRunning = true;

    std::thread demuxThread(&FFCodecHandler::demux, this);
    demuxThread.detach();

    std::thread videoDecodeThread(&FFCodecHandler::videoDecode, this);
    videoDecodeThread.detach();

    std::thread audioDecodeThread(&FFCodecHandler::audioDecode, this);
    audioDecodeThread.detach();
}

void FFCodecHandler::waitAllMediaThreadExit() {

}

void FFCodecHandler::stdThreadSleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void FFCodecHandler::demux() {
    while (demuxThreadRunning) {
        if (playStatus == FFPlayStatus::STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }
        //LOGI("v size %d, a size %d.", videoPackets.size(), audioPackets.size());
        if (videoPackets.isOverflow() || audioPackets.isOverflow()) {
            stdThreadSleep(10);
            continue;
        }

        if (demuxFileEOF) {
            stdThreadSleep(10);
            continue;
        }

        readMediaPacket();
    }
}

void FFCodecHandler::videoDecode() {
    if (ic == nullptr || videoCodecCtx == nullptr) {
        return;
    }
    if (videoFrame == nullptr) {
        videoFrame  = av_frame_alloc();
    }
    while (demuxThreadRunning) {
        vDecodeThreadRunning = true;
        if (playStatus == FFPlayStatus::STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }
        if (videoPackets.isEmpty()) {
            stdThreadSleep(10);
            continue;
        }
        AVPacket* packet = videoPackets.dequeue();
        if (packet == nullptr) {
            stdThreadSleep(10);
            continue;
        }

        int ret = avcodec_send_packet(videoCodecCtx, packet);
        if (ret) {
            LOGE("send packet to codec failed.");
        }
        if (avcodec_receive_frame(videoCodecCtx, videoFrame) == 0) {
            LOGI("receive video frame size = %d, %d, pts = %f.", videoFrame->width, videoFrame->height,
                 videoFrame->pts * av_q2d(vTimeBase));
        }
        freePacket(packet);
    }
}

void FFCodecHandler::audioDecode() {
    if (ic == nullptr || audioCodecCtx == nullptr) {
        return;
    }
    if (audioFrame == nullptr) {
        audioFrame  = av_frame_alloc();
    }
    while (demuxThreadRunning) {
        vDecodeThreadRunning = true;
        if (playStatus == FFPlayStatus::STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }
        if (audioPackets.isEmpty()) {
            stdThreadSleep(10);
            continue;
        }
        AVPacket* packet = audioPackets.dequeue();
        if (packet == nullptr) {
            stdThreadSleep(10);
            continue;
        }

        int ret = avcodec_send_packet(audioCodecCtx, packet);
        if (ret) {
            LOGE("send packet to codec failed.");
        }
        if (avcodec_receive_frame(audioCodecCtx, audioFrame) == 0) {
            LOGI("receive audio frame samples = %d, pts = %f.", audioFrame->nb_samples,
                 audioFrame->pts * av_q2d(aTimeBase));
        }
        freePacket(packet);
    }
}

void FFCodecHandler::readMediaPacket() {
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        LOGE("alloc packet failed.");
        return;
    }
    int ret = av_read_frame(ic, packet);
    if (ret == 0) {
        if (packet->stream_index == videoStream) {
            AVPacket* copy = av_packet_alloc();
            if (!av_packet_ref(copy, packet)) {
                videoPackets.enqueue(copy);
            } else {
                freePacket(copy);
            }
        } else if (packet->stream_index == audioStream) {
            AVPacket* copy = av_packet_alloc();
            if (!av_packet_ref(copy, packet)) {
                audioPackets.enqueue(copy);
            } else {
                freePacket(copy);
            }
        }
    } else if (ret < 0) {
        if (!demuxFileEOF && ret == AVERROR_EOF) {
            demuxFileEOF = true;
        }
    }
    freePacket(packet);
}

void FFCodecHandler::freePacket(AVPacket *packet) {
    if (packet) {
        if (packet->buf) {
            LOGI("packet refs count = %d.", av_buffer_get_ref_count(packet->buf));
        }
        av_packet_unref(packet);
        av_packet_free(&packet);
    }
}

void FFCodecHandler::freeFrame(AVFrame *frame) {
    if (frame) {
        av_frame_free(&frame);
    }
}








