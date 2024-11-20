#include "FFEglEnvironment.h"

#include "FFLog.h"
#include <condition_variable>

FFEglEnvironment::FFEglEnvironment() {

}

FFEglEnvironment::~FFEglEnvironment() {

}

void FFEglEnvironment::init() {
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    // 1. 创建 EGL
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOGE("FFEglEnvironment create display failed.");
        return;
    }

    // 2. 初始化 EGL
    EGLint major, minor;
    if (!eglInitialize(eglDisplay, &major, &minor)) {
        LOGE("FFEglEnvironment initialize display failed.");
        return;
    }
    LOGI("FFEglEnvironment init egl success, %d, %d", major, minor);

    // 3. 配置 EGL
    EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // 使用 OpenGL ES 2.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            //EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };

    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay, configAttribs,
                         &eglConfig, 1, &numConfigs)) {
        LOGE("FFEglEnvironment config display failed.");
        return;
    }

    // 创建 EGL 上下文
    EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,EGL_NONE
    };
    eglContext = eglCreateContext(eglDisplay, eglConfig,
                                  EGL_NO_CONTEXT, contextAttribs);
    if (eglContext == EGL_NO_CONTEXT) {
        LOGE("FFEglEnvironment create context failed.");
        return;
    }
    EGLint error = eglGetError();
    if (error != EGL_SUCCESS) {
        LOGE("EGL error: %d", error);
    }

    LOGI("FFEglEnvironment init complete.");
    LOGI("create shader, %d", glCreateShader(GL_VERTEX_SHADER));
    LOGI("create shader, %d", glCreateShader(GL_VERTEX_SHADER));
    LOGI("create shader, %d", glCreateShader(GL_VERTEX_SHADER));
}

void FFEglEnvironment::setPreviewWindow(ANativeWindow *nativeWindow) {
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
    if (eglSurface == EGL_NO_SURFACE) {
        LOGE("FFEglEnvironment createEglSurface failed.");
    } else {
        if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
            LOGE("FFEglEnvironment eglMakeCurrent failed.");
        }
    }
}

void FFEglEnvironment::removePreviewWindow() {
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext);
    if (eglSurface != EGL_NO_SURFACE) {
        if (!eglDestroySurface(eglDisplay, eglSurface)) {
            LOGE("FFEglEnvironment removePreviewWindow failed.");
        }
    }
    eglSurface = EGL_NO_SURFACE;
}

void FFEglEnvironment::release() {
    eglDestroyContext(eglDisplay, eglContext);
    removePreviewWindow();
    eglTerminate(eglDisplay);
}

void FFEglEnvironment::makeCurrentDisplay() {
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
}

void FFEglEnvironment::swapBuffer() {
    if (eglSurface != EGL_NO_SURFACE) {
        eglSwapBuffers(eglDisplay, eglSurface);
    }
}




