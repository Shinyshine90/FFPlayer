#ifndef NDKPRACTICE_FFVIDEOPLAY_H
#define NDKPRACTICE_FFVIDEOPLAY_H

#include "FFEglEnvironment.h"
#include "FFOpenGLShader.h"

class FFVideoPlay {

private:
    FFThread eglThread;

    FFEglEnvironment eglEnvironment;

    FFOpenGLShader videoPlayShader;

    void post(std::function<void()>);

public:
    FFVideoPlay();

    ~FFVideoPlay();

    void setDisplayWindow(ANativeWindow* nativeWindow);

    void removeDisplayWindow();

    void resizeDisplayWindow(int width, int height);
};
#endif
