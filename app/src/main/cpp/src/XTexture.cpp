#include "XTexture.h"
#include "XShader.h"
#include "ALog.h"
#include "XEGL.h"

class CXTexture : public XTexture {
public:
    void destroy() override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        XEGL::getInstance()->close();
        shader.close();
        delete this;
    }

    bool init(void* win) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        XEGL::getInstance()->close();
        shader.close();
        if (!win) {
            LOGE("XTexture init failed, null window.");
            return false;
        }
        if (!XEGL::getInstance()->init()) {
            LOGE("XTexture init failed, egl init failed.");
            return false;
        }
        XEGL::getInstance()->setNativeWindow(win);
        if (!shader.init()) {
            LOGE("XTexture init failed, shader failed.");
            return false;
        }
        return true;
    }

    void draw(unsigned char **data, int width, int height) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        LOGE("XTexture draw %d %d", width, height);
        shader.fillTexture(0, data[0], width, height);
        shader.fillTexture(1, data[1], width / 2, height / 2);
        shader.fillTexture(2, data[2], width / 2, height / 2);
        shader.render();
        XEGL::getInstance()->swapBuffer();
    }

private:
    XShader shader;
    std::recursive_mutex mutex;
};

XTexture* XTexture::create() {
    return new CXTexture;
}


