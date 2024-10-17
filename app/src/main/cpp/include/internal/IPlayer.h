#ifndef NDKPRACTICE_IPLAYER_H
#define NDKPRACTICE_IPLAYER_H

#include "XThread.h"
#include <mutex>
struct IDemux;
struct IDecoder;
struct IVideoView;
struct IResample;
struct IAudioPlay;


class IPlayer : public XThread {
public:
    static IPlayer* getInstance(unsigned char index = 0);

    virtual bool open(const char* path);

    virtual void close();

    virtual bool setSurface(void* win);

    virtual bool start() override;

    virtual bool seek(double position);

    virtual bool isPause() override;

    virtual void pause();

    virtual void resume();

    double playPosition = 0.0;

    void(*callback)(double);

    IDemux* demux = nullptr;
    IDecoder* audioDecoder = nullptr;
    IDecoder* videoDecoder = nullptr;
    IVideoView* videoView = nullptr;
    IResample* resample = nullptr;
    IAudioPlay* audioPlay = nullptr;

protected:
    IPlayer() {};
    std::recursive_mutex mutex;
    void run() override;
    void updatePlayStation();
};


#endif