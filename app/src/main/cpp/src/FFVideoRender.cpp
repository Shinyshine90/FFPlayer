#include "FFVideoRender.h"
#include <EGL/egl.h>
#include <queue>

#define TAG "FFVideoRender"

FFVideoRender::FFVideoRender() {
    post([this]{
        eglEnvironment.init();
        videoPlayShader.init();
    });
}

FFVideoRender::~FFVideoRender() {
    LOGI(TAG, "FFVideoRender destructor.");
    std::mutex m;
    std::condition_variable condition;
    eglThread.clear();
    post([this, &condition]{
        videoPlayShader.release();
        eglEnvironment.release();
        condition.notify_one();
    });
    std::unique_lock<std::mutex> l;
    condition.wait_for(l, std::chrono::milliseconds(200), []{
        return true;
    });
    LOGI(TAG, "FFVideoRender destructor complete.");
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

void FFVideoRender::fill(int index, unsigned char *data, int width, int height) {
    post([this, index, data, width, height]{
        videoPlayShader.fillYUV(index, data, width, height);
        delete data;
    });
}

void FFVideoRender::render() {
    post([this]{
        eglEnvironment.makeCurrentDisplay();
        videoPlayShader.render();
        eglEnvironment.swapBuffer();
    });
}

void FFVideoRender::post(std::function<void()> runnable) {
    eglThread.post(runnable);
}







