#ifndef NDKPRACTICE_FFEGLENVIRONMENT_H
#define NDKPRACTICE_FFEGLENVIRONMENT_H

#include "FFThread.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/native_window.h>

#include <atomic>

class FFEglEnvironment {

private:

    EGLDisplay eglDisplay = EGL_NO_DISPLAY;

    EGLContext eglContext = EGL_NO_CONTEXT;

    EGLConfig eglConfig = 0;

    EGLSurface eglSurface = EGL_NO_SURFACE;

public:
    FFEglEnvironment();

    ~FFEglEnvironment();

    void init();

    void release();

    void setPreviewWindow(ANativeWindow* aNativeWindow);

    void removePreviewWindow();
};

#endif