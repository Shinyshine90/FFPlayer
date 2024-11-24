#include "FFCodecHandler.h"
#include "FFLog.h"

#include <thread>
#include <atomic>
#include <chrono>

FFCodecHandler::FFCodecHandler() {}

FFCodecHandler::~FFCodecHandler() {}

void FFCodecHandler::SetMediaPath(const char *url) {
    std::lock_guard<std::mutex> lock(statusMutex);
    this->url = url;
}

int FFCodecHandler::InitCodec() {
    std::lock_guard<std::mutex> lock(statusMutex);
    playStatus = FFPlayStatus::STATUS_PREPARE;
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
    videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    LOGI("avformat, found video stream %d, audio stream %d.", videoStream, audioStream);

    if (0 <= videoStream) {
        LOGI("avformat, video codec id = %d.", ic->streams[videoStream]->codecpar->codec_id);
        AVCodec* videoCodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
        //AVCodec* videoCodec = avcodec_find_decoder_by_name("h264_mediacodec");
        if (videoCodec) {
            videoCodecCtx = avcodec_alloc_context3(videoCodec);
            videoCodecCtx->thread_count = 8;
            avcodec_parameters_to_context(videoCodecCtx, ic->streams[videoStream]->codecpar);
            if (avcodec_open2(videoCodecCtx, videoCodec, nullptr)) {
                LOGE("video codec open failed.");
            } else {
                LOGI("video codec open success, id = %d, fmt = %d.", videoCodecCtx->codec_id, videoCodecCtx->pix_fmt);
            }
        } else {
            LOGE("avformat, found no codec for video stream.");
        }
        vTimeBase = ic->streams[videoStream]->time_base;
    }
    if (0 <= audioStream) {
        LOGI("avformat, audio codec id = %d.", ic->streams[videoStream]->codecpar->codec_id);
        AVCodec* audioCodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
        if (audioCodec) {
            audioCodecCtx = avcodec_alloc_context3(audioCodec);
            audioCodecCtx->thread_count = 4;
            avcodec_parameters_to_context(audioCodecCtx, ic->streams[audioStream]->codecpar);
            if (avcodec_open2(audioCodecCtx, audioCodec, nullptr)) {
                LOGE("audio codec open failed.");
            } else {
                audioSpeaker.init(audioCodecCtx->sample_rate,
                                  audioCodecCtx->sample_fmt,
                                  audioCodecCtx->channel_layout,
                                  44100,
                                  AVSampleFormat::AV_SAMPLE_FMT_S16,
                                  av_get_default_channel_layout(2));
                LOGI("audio codec open success.");
            }
        } else {
            LOGE("avformat, found no codec for audio stream.");
        }
        aTimeBase = ic->streams[audioStream]->time_base;
    }
    return 0;
}

void FFCodecHandler::UnInitCodec() {
    StopPlayVideo();
    std::lock_guard<std::mutex> lock(statusMutex);
    playStatus = FFPlayStatus::STATUS_RELEASE;
    if (videoCodecCtx) {
        avcodec_close(videoCodecCtx);
    }
    if (audioCodecCtx) {
        avcodec_close(audioCodecCtx);
    }
    if (ic) {
        avformat_close_input(&ic);
    }

    freeFrame(videoFrame);

    freeFrame(audioFrame);

    while (!videoPackets.isEmpty()) {
        freePacket(videoPackets.dequeue());
    }

    while (!audioPackets.isEmpty()) {
        freePacket(audioPackets.dequeue());
    }
}

void FFCodecHandler::StartPlayVideo() {
    std::lock_guard<std::mutex> lock(statusMutex);
    FFPlayStatus prevStatus = playStatus;
    playStatus = FFPlayStatus::STATUS_PLAYING;
    //audioSpeaker.start();
    if (prevStatus == FFPlayStatus::STATUS_PREPARE) {
        startMediaProcessThread();
        LOGI("FFCodecHandler start.");
    } else if (prevStatus == FFPlayStatus::STATUS_PAUSE) {
        timeSync.resumeSync();
        LOGI("FFCodecHandler resume.");
    }
}

void FFCodecHandler::PausePlayVideo() {
    std::lock_guard<std::mutex> lock(statusMutex);
    playStatus = FFPlayStatus::STATUS_PAUSE;
    timeSync.pauseSync();
    audioSpeaker.stop();
    LOGI("FFCodecHandler pause.");
}

void FFCodecHandler::StopPlayVideo() {
    std::lock_guard<std::mutex> lock(statusMutex);
    if (STATUS_STOP <= playStatus) return;
    playStatus = FFPlayStatus::STATUS_STOP;
    audioSpeaker.stop();
    isInterrupted = true;
    waitAllMediaThreadExit();
    LOGI("FFCodecHandler stop.");
}

void FFCodecHandler::Seek(float position) {
    std::lock_guard<std::mutex> lock(statusMutex);
    playStatus = FFPlayStatus::STATUS_SEEK;
    //
    LOGI("FFCodecHandler seek %f.", position);
}

int FFCodecHandler::GetMediaTotalSeconds() {
    if (ic && 0 <= videoStream) {
        return ic->streams[videoStream]->duration * av_q2d(vTimeBase) * 1000;
    }
    return 0;
}

int FFCodecHandler::GetPlayStatus() {
    return playStatus;
}

void FFCodecHandler::startMediaProcessThread() {
    if (ic == nullptr) {
        LOGE("FFCodecHandler avformat open input failed.");
        return;
    }
    demuxThread = std::thread(&FFCodecHandler::demux, this);
    if (videoCodecCtx) {
        vDecodeThread = std::thread(&FFCodecHandler::videoDecode, this);
    }
    if (audioCodecCtx) {
        aDecodeThread = std::thread(&FFCodecHandler::audioDecode, this);
    }
}

void FFCodecHandler::waitAllMediaThreadExit() {
    if (demuxThread.joinable()) {
        demuxThread.join();
    }
    if (vDecodeThread.joinable()) {
        vDecodeThread.join();
    }
    if (aDecodeThread.joinable()) {
        aDecodeThread.join();
    }
}

void FFCodecHandler::stdThreadSleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void FFCodecHandler::demux() {
    while (!isInterrupted) {
        if (playStatus == FFPlayStatus::STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }

        if (videoPackets.isOverflow() || audioPackets.isOverflow()) {
            stdThreadSleep(10);
            LOGE("packets overflow, vs = %ld, as = %ld.", videoPackets.size(), audioPackets.size());
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
    //if (true) return;
    if (videoFrame == nullptr) {
        videoFrame  = av_frame_alloc();
    }
    while (!isInterrupted) {
        if (playStatus == FFPlayStatus::STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }
        if (videoPackets.isEmpty()) {
            stdThreadSleep(10);
            continue;
        }

        AVPacket* packet = videoPackets.dequeue();
        int ret = avcodec_send_packet(videoCodecCtx, packet);
        freePacket(packet);
        if (ret) {
            LOGE("send packet failed.");
        }

        while (avcodec_receive_frame(videoCodecCtx, videoFrame) == 0) {
            //LOGI("receive video frame, pts = %f.", videoFrame->pts * av_q2d(vTimeBase));
            handleVideoFrame(videoFrame);
        }
    }
}

void FFCodecHandler::audioDecode() {
    if (audioFrame == nullptr) {
        audioFrame  = av_frame_alloc();
    }
    while (!isInterrupted) {
        if (playStatus == FFPlayStatus::STATUS_PAUSE) {
            stdThreadSleep(10);
            continue;
        }
        if (audioPackets.isEmpty()) {
            stdThreadSleep(10);
            continue;
        }

        AVPacket* packet = audioPackets.dequeue();
        int ret = avcodec_send_packet(audioCodecCtx, packet);
        if (ret) {
            LOGE("audio decode send packet failed, ret = %d.", ret);
        }
        freePacket(packet);

        if (avcodec_receive_frame(audioCodecCtx, audioFrame) == 0) {
            LOGI("receive audio frame, pts = %f.",audioFrame->pts * av_q2d(aTimeBase));
            handleAudioFrame(audioFrame);
        }
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
        if (!timeSync.isStart()) {
            timeSync.startSync();
        }
    } else if (ret < 0) {
        if (!demuxFileEOF && ret == AVERROR_EOF) {
            demuxFileEOF = true;
        }
    }
    freePacket(packet);
    stdThreadSleep(1);
}

void FFCodecHandler::freePacket(AVPacket *packet) {
    if (packet) {
        av_packet_unref(packet);
        av_packet_free(&packet);
    }
}

void FFCodecHandler::freeFrame(AVFrame *frame) {
    if (frame) {
        av_frame_free(&frame);
    }
}

void FFCodecHandler::handleVideoFrame(AVFrame *frame) {
    if (!frame) {
        return;
    }
    int64_t ptsTime = frame->pts * av_q2d(vTimeBase) * 1000;
    int64_t draftTime = timeSync.getSyncDrift();
    int64_t diff = abs(ptsTime - draftTime);
    if (draftTime < ptsTime) {
        while (draftTime < ptsTime) {
            LOGI("pts = %ld, draft = %ld, sync video sleep.", ptsTime, draftTime);
            stdThreadSleep(ptsTime - draftTime);
            draftTime = timeSync.getSyncDrift();
        }
    } else if (100 < diff) {
        LOGE("pts = %ld, draft = %ld, sync video drop.", ptsTime, draftTime);
        return;
    }

    int width = frame->width, height = frame->height;
    unsigned char* y = new unsigned char[width * height];
    unsigned char* u = new unsigned char[width * height / 4];
    unsigned char* v = new unsigned char[width * height / 4];
    memcpy(y, frame->data[0], width * height);
    memcpy(u, frame->data[1], width * height / 4);
    memcpy(v, frame->data[2], width * height / 4);
    videoRender.fill(0, y,width, height);
    videoRender.fill(1, u,width / 2, height / 2);
    videoRender.fill(2, v,width / 2, height / 2);
    videoRender.render();
}

void FFCodecHandler::handleAudioFrame(AVFrame *frame) {
    if (!frame) return;
    audioSpeaker.start();
    audioSpeaker.convertPCM(frame);
}
