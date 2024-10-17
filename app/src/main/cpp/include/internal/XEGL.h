#ifndef NDKPRACTICE_XEGL_H
#define NDKPRACTICE_XEGL_H

#include "EGL/egl.h"
#include <android/native_window.h>
#include "mutex"

class XEGL
{
public:
    static XEGL* getInstance();

    virtual bool init() = 0;

    virtual void close() = 0;

    virtual bool setNativeWindow(void* window) = 0;

    virtual bool removeNativeWindow(void* window) = 0;

    virtual bool swapBuffer()  = 0;

    bool isInit = false;

protected:
    XEGL(){}
    std::recursive_mutex mutex;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLConfig config = nullptr;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
};


#endif //NDKPRACTICE_XEGL_H
