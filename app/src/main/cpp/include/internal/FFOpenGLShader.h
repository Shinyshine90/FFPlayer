#ifndef NDKPRACTICE_FFOPENGLSHADER_H
#define NDKPRACTICE_FFOPENGLSHADER_H

class FFOpenGLShader {
private:
    int program = -1;

    int vertexShader = -1;

    int fragmentShader = -1;

public:

    FFOpenGLShader();

    ~FFOpenGLShader();

    void init();

    void release();

    void draw();
};
#endif