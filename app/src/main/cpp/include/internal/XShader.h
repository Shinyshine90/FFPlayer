#ifndef NDKPRACTICE_XSHADER_H
#define NDKPRACTICE_XSHADER_H

#include <mutex>

class XShader {
public:
    virtual bool init();

    virtual void close();

    virtual void render();

    virtual void fillTexture(int index, unsigned char* data, int width, int height);

private:
    std::recursive_mutex mutex;

    unsigned int vertexShader = 0;

    unsigned int fragmentShader = 0;

    unsigned int glProgram = 0;

    unsigned int yuvTextures[3] {0};
};

#endif
