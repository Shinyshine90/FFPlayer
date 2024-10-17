#pragma once

#include "IDecoder.h"
struct AVCodecContext;
struct AVFrame;

class FFDecoder : public IDecoder {
public:
    virtual bool open(XParameter params);

    virtual void close();

    virtual void clearBuffer() override;

    virtual bool sendPacket(XData data);

    virtual XData receiveFrame();

protected:
    AVCodecContext* codecContext = nullptr;
    AVFrame* frame = nullptr;
    //std::recursive_mutex mutex;
};