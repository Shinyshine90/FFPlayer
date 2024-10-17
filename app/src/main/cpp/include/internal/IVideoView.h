

#ifndef NDKPRACTICE_IVIDEOVIEW_H
#define NDKPRACTICE_IVIDEOVIEW_H

#include "XData.h"
#include "IObserver.h"

class IVideoView : public IObserver {
public:
    virtual void setWindow(void* win) = 0;

    virtual void render(XData data) = 0;

    virtual void update(XData data) override;

    virtual void close() = 0;

    long pts = 0;
};
#endif
