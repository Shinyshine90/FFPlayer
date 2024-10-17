#ifndef NDKPRACTICE_IDEMUX_H
#define NDKPRACTICE_IDEMUX_H

#include "XData.h"
#include "XThread.h"
#include "IObserver.h"
#include "XParameter.h"

class IDemux : public XThread, public IObserver {
public:
    virtual bool open(const char *url) = 0;

    virtual bool seek(double position) = 0;

    virtual void close() = 0;

    virtual XParameter getVideoDecodeParams() = 0;

    virtual XParameter getAudioDecodeParams() = 0;

    virtual XData read() = 0;

    long durationMs = 0;

protected:
    virtual void run();

};

#endif
