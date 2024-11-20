#ifndef NDKPRACTICE_FFOPENGLSHADER_H
#define NDKPRACTICE_FFOPENGLSHADER_H

class FFOpenGLShader {
private:
    int program = -1;

    int vertexShader = -1;

    int fragmentShader = -1;

    unsigned yuvTextures[3] {0, 0, 0};

public:

    FFOpenGLShader();

    ~FFOpenGLShader();

    void init();

    void release();

    void render(int index, unsigned char *data, int width, int height);
};
#endif