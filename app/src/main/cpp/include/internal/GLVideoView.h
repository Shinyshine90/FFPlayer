#ifndef NDKPRACTICE_GLVIDEOVIEW_H
#define NDKPRACTICE_GLVIDEOVIEW_H

#include "IVideoView.h"
#include "XTexture.h"

class GLVideoView : public IVideoView {
public:
    virtual void setWindow(void* win) override;

    virtual void render(XData data) override;

    virtual void close() override;
private:
    std::recursive_mutex mutex;
    void* win = nullptr;
    XTexture* texture = nullptr;
};

#endif