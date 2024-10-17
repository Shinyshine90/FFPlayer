#ifndef NDKPRACTICE_FFDEMUX_H
#define NDKPRACTICE_FFDEMUX_H

#include "IDemux.h"

struct AVFormatContext;

class FFDemux : public IDemux {
public:
    FFDemux();

    virtual bool open(const char *url);

    bool seek(double position) override;

    virtual void close();

    virtual XParameter getVideoDecodeParams();

    virtual XParameter getAudioDecodeParams();

    virtual XData read();

private:
    std::recursive_mutex mutex;

    AVFormatContext *formatContext = nullptr;

    int videoStream = -1;

    int audioStream = -1;


};

#endif
