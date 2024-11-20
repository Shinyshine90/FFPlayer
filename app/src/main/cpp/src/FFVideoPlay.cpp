#include "FFVideoPlay.h"
#include <EGL/egl.h>

FFVideoPlay::FFVideoPlay() {
    eglThread.start();
    post([this]{
        eglEnvironment.init();
        videoPlayShader.init();
    });
}

FFVideoPlay::~FFVideoPlay() {
    LOGI("FFVideoPlay destructor.");
    std::mutex m;
    std::condition_variable condition;
    post([this, &condition]{
        videoPlayShader.release();
        eglEnvironment.release();
        condition.notify_one();
    });
    std::unique_lock<std::mutex> l;
    condition.wait_for(l, std::chrono::milliseconds(200), []{
        return true;
    });
    eglThread.stop();
    LOGI("FFVideoPlay destructor complete.");
}

void FFVideoPlay::setDisplayWindow(ANativeWindow *nativeWindow) {
    post([this, nativeWindow]{
        eglEnvironment.setPreviewWindow(nativeWindow);
    });
}

void FFVideoPlay::removeDisplayWindow() {
    post([this]{
        eglEnvironment.removePreviewWindow();
    });
}

void FFVideoPlay::resizeDisplayWindow(int width, int height) {
    post([width, height]{
        glViewport(0, 0, width, height);
    });
}

void FFVideoPlay::post(std::function<void()> runnable) {
    eglThread.post(runnable);
}




