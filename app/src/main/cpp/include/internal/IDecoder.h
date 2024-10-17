//
// Created by Shadowalker on 10/9/2024.
//

#ifndef NDKPRACTICE_IDECODER_H
#define NDKPRACTICE_IDECODER_H

#include "XParameter.h"
#include "IObserver.h"
#include "XThread.h"
#include <list>

class IDecoder : public XThread, public IObserver {
public:

    const char* getMediaTypeName();

    virtual void close() = 0;

    virtual bool open(XParameter params) = 0;

    virtual bool sendPacket(XData data) = 0;

    virtual XData receiveFrame() = 0;

    virtual void clearBuffer();

    bool isAudio = false;

    int syncPts = 0;

    int pts = 0;

protected:
    std::recursive_mutex mutex;
    std::list<XData> packs;
    int maxBufferSize = 1000;
    virtual void run() override;
    virtual void update(XData data) override;
};

#endif
