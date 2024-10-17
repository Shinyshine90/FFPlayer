#ifndef NDKPRACTICE_XTEXTURE_H
#define NDKPRACTICE_XTEXTURE_H

class XTexture {
public:
    static XTexture* create();

    virtual bool init(void* win) = 0;

    virtual void draw(unsigned char* data[], int width, int height) = 0;

    virtual void destroy() = 0;

    virtual ~XTexture() {};
protected:
    XTexture() {}
};
#endif //NDKPRACTICE_XTEXTURE_H
