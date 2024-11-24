#ifndef NDKPRACTICE_FFCODECHANDLER_H
#define NDKPRACTICE_FFCODECHANDLER_H

#include <string>
#include "FFMediaQueue.h"
#include "FFVideoRender.h"
#include "FFAvSync.h"
#include "FFAudioSpeaker.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

enum FFMediaType {
    VIDEO = 0,
    AUDIO
};

enum FFPlayStatus {
    STATUS_INIT = 0,
    STATUS_PREPARE,
    STATUS_PLAYING,
    STATUS_PAUSE,
    STATUS_SEEK,
    STATUS_STOP,
    STATUS_RELEASE
};

class FFCodecHandler {

private:
const int MAX_VIDEO_PACKET_BUFFER = 200;
const int MAX_AUDIO_PACKET_BUFFER = 500;

const char* url = nullptr;

AVFormatContext* ic = nullptr;

int videoStream = -1, audioStream = -1;

AVCodecContext *videoCodecCtx = nullptr, *audioCodecCtx = nullptr;

AVRational aTimeBase, vTimeBase;

FFPlayStatus playStatus = STATUS_INIT;

FFMediaQueue<AVPacket*> videoPackets {MAX_VIDEO_PACKET_BUFFER};

FFMediaQueue<AVPacket*> audioPackets {MAX_AUDIO_PACKET_BUFFER};

bool demuxFileEOF = false;

AVFrame* videoFrame = nullptr;

AVFrame* audioFrame = nullptr;

std::thread demuxThread, vDecodeThread, aDecodeThread;

std::atomic<bool> isInterrupted {false };

std::mutex statusMutex;

FFAvSync timeSync;

void startMediaProcessThread();

void waitAllMediaThreadExit();

void stdThreadSleep(int ms);

void demux();

void videoDecode();

void audioDecode();

void readMediaPacket();

void freePacket(AVPacket* packet);

void freeFrame(AVFrame* frame);

void handleVideoFrame(AVFrame* frame);

void handleAudioFrame(AVFrame* frame);

public:
    FFVideoRender videoRender;

    FFAudioSpeaker audioSpeaker;

    FFCodecHandler();

    ~FFCodecHandler();

    void SetMediaPath(const char* url);

    int InitCodec();

    void UnInitCodec();

    void StartPlayVideo();

    void PausePlayVideo();

    void StopPlayVideo();

    void Seek(float position);

    int GetMediaTotalSeconds();

    int GetPlayStatus();

};
#endif
