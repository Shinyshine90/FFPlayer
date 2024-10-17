
#ifndef NDKPRACTICE_IAUDIOPLAY_H
#define NDKPRACTICE_IAUDIOPLAY_H

#include <list>
#include "XData.h"
#include "IObserver.h"
#include "XParameter.h"
#include "XThread.h"

class IAudioPlay : public IObserver, public XThread {
public:
    virtual bool startPlay(XParameter param) = 0;

    virtual void close() = 0;

    virtual XData acquireData();

    virtual std::list<XData> acquireAllData();

    virtual void update(XData data) override;

    virtual void clearBuffer();

    int maxFrames = 200;

    long pts = 0;

protected:
    void run() override;
    std::list<XData> frames;
    std::recursive_mutex mutex;
};

#endif
