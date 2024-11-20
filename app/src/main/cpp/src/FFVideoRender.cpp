#include "FFVideoRender.h"
#include <EGL/egl.h>

FFVideoRender::FFVideoRender() {
    eglThread.start();
    post([this]{
        eglEnvironment.init();
        videoPlayShader.init();
    });
}

FFVideoRender::~FFVideoRender() {
    LOGI("FFVideoRender destructor.");
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
    LOGI("FFVideoRender destructor complete.");
}

void FFVideoRender::setDisplayWindow(ANativeWindow *nativeWindow) {
    post([this, nativeWindow]{
        eglEnvironment.setPreviewWindow(nativeWindow);
    });
}

void FFVideoRender::removeDisplayWindow() {
    post([this]{
        eglEnvironment.removePreviewWindow();
    });
}

void FFVideoRender::resizeDisplayWindow(int width, int height) {
    post([width, height]{
        glViewport(0, 0, width, height);
    });
}

void FFVideoRender::render(int index, unsigned char *data, int width, int height) {
    post([this, index, data, width, height]{
        eglEnvironment.makeCurrentDisplay();
        videoPlayShader.render(index, data, width, height);
        eglEnvironment.swapBuffer();
    });
}

void FFVideoRender::post(std::function<void()> runnable) {
    eglThread.post(runnable);
}






