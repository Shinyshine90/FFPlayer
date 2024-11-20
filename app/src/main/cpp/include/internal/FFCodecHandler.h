#ifndef NDKPRACTICE_FFCODECHANDLER_H
#define NDKPRACTICE_FFCODECHANDLER_H

#include <string>
#include "FFMediaQueue.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

enum FFMediaType {
    VIDEO = 0,
    AUDIO
};

enum FFPlayStatus {
    STATUS_PLAYING = 0,
    STATUS_PAUSE,
    STATUS_SEEK,
    STATUS_STOP,
};

class FFCodecHandler {

private:
const int MAX_VIDEO_PACKET_BUFFER = 20;
const int MAX_AUDIO_PACKET_BUFFER = 50;

const char* url = nullptr;

AVFormatContext* ic = nullptr;

int videoStream = -1, audioStream = -1;

AVCodecContext * videoCodecCtx, *audioCodecCtx;

AVRational aTimeBase, vTimeBase;

FFPlayStatus playStatus;

FFMediaQueue<AVPacket*> videoPackets;

FFMediaQueue<AVPacket*> audioPackets;

bool demuxFileEOF = false;

AVFrame* videoFrame = nullptr;

AVFrame* audioFrame = nullptr;

void startMediaProcessThread();

void waitAllMediaThreadExit();

void stdThreadSleep(int ms);

void demux();

void videoDecode();

void audioDecode();

void readMediaPacket();

void freePacket(AVPacket* packet);

void freeFrame(AVFrame* frame);

public:
    FFCodecHandler();

    ~FFCodecHandler();

    void SetMediaPath(const char* url);

    int InitCodec();

    void UnInitCodec();

    void StartPlayVideo();

    void StopPlayVideo();

    void SetStatusPlay();

    void SetStatusPause();

    void Seek(float position);

    void GetMediaTotalSeconds();

    int GetPlayStatus();

};
#endif
