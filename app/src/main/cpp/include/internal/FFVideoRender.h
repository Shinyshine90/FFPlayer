#ifndef NDKPRACTICE_FFVIDEORENDER_H
#define NDKPRACTICE_FFVIDEORENDER_H

#include "FFEglEnvironment.h"
#include "FFOpenGLShader.h"

class FFVideoRender {

private:
    FFThread eglThread;

    FFEglEnvironment eglEnvironment;

    FFOpenGLShader videoPlayShader;

    void post(std::function<void()>);

public:
    FFVideoRender();

    ~FFVideoRender();

    void fill(int index, unsigned char *data, int width, int height);

    void render();

    void setDisplayWindow(ANativeWindow* nativeWindow);

    void removeDisplayWindow();

    void resizeDisplayWindow(int width, int height);
};
#endif
