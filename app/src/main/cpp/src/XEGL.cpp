#include "XEGL.h"
#include "ALog.h"

#include "EGL/egl.h"
#include <android/native_window.h>

class CXEGL : public XEGL {
public:
    void close() override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (display == EGL_NO_DISPLAY) {
            return;
        }
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (surface != EGL_NO_SURFACE)
            eglDestroySurface(display, surface);
        if (context != EGL_NO_CONTEXT)
            eglDestroyContext(display, context);

        eglTerminate(display);

        display = EGL_NO_DISPLAY;
        context = EGL_NO_CONTEXT;
        surface = EGL_NO_SURFACE;
        LOGI("XEGL close complete.");

    }
    bool init() override {
       std::lock_guard<std::recursive_mutex> lock(mutex);
       EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            LOGE("eglGetDisplay failed.");
            return false;
        }
        EGLint major, minor;
        if (eglInitialize(display, &major, &minor) != EGL_TRUE) {
            LOGE("eglInitialize failed.");
            return false;
        }

        EGLint configAttrs[] {
            EGL_RENDERABLE_TYPE , EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_BUFFER_SIZE, 32,
            EGL_NONE
        };
        EGLConfig config;
        EGLint configNum;
        if (eglChooseConfig(display, configAttrs,
                            &config, 1, &configNum) != EGL_TRUE) {
            LOGE("eglChooseConfig failed.");
            return false;
        }
        EGLint contextAttrs[] {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        EGLContext context = eglCreateContext(
                display, config, EGL_NO_CONTEXT, contextAttrs);
        if (context == EGL_NO_CONTEXT) {
            LOGE("eglCreateContext failed.");
            return false;
        }
        isInit = true;
        this->display = display;
        this->config = config;
        this->context = context;
        LOGI("XEGL init complete.");
        return true;
    }

    bool setNativeWindow(void* win) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        ANativeWindow* window = static_cast<ANativeWindow*>(win);
        EGLSurface surface = eglCreateWindowSurface(display, config, window, nullptr);
        if (surface == EGL_NO_SURFACE) {
            LOGE("eglCreateWindowSurface failed.");
            return false;
        }
        this->surface = surface;
        if (eglMakeCurrent(display, surface, surface, context) != EGL_TRUE) {
            LOGE("eglMakeCurrent failed.");
            return false;
        }
        LOGI("XEGL setNative window complete.");
        return true;
    }

    bool removeNativeWindow(void* win) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        ANativeWindow* window = static_cast<ANativeWindow*>(win);
        if (surface != EGL_NO_SURFACE && eglDestroySurface(display, surface)) {
            surface = EGL_NO_SURFACE;
            return false;
        }
        if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) != EGL_TRUE) {
            return false;
        }
        LOGI("XEGL remove Native window complete.");
        return true;
    }

    bool swapBuffer() override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        eglSwapBuffers(display, surface);
        LOGI("XEGL swap buffer complete.");
        return true;
    }
};

XEGL *XEGL::getInstance() {
    static XEGL* ptr = new CXEGL();
    return ptr;
}