#include "GLVideoView.h"
#include "ALog.h"

void GLVideoView::setWindow(void* win) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    this->win = win;
    LOGI("GLVideoView setWidnow %ld", win);
}

void GLVideoView::render(XData data) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!win) {
        LOGE("GLVideoView render no window.");
        return;
    }
    if (!texture) {
        texture = XTexture::create();
        texture->init(win);
        LOGI("GLVideoView texture init");
        return;
    }
    texture->draw(data.datas, data.width, data.height);
    pts = data.pts;
}

void GLVideoView::close() {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (texture) {
        texture->destroy();
        texture = 0;
    }
}
