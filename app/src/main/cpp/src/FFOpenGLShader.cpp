#include <GLES2/gl2.h>
#include "FFOpenGLShader.h"
#include "FFLog.h"

#define TAG "FFOpenGLShader"

#define GET_STR(str) #str

const char *vertex_shader = GET_STR(
        attribute vec4 aVertexCoors;
        attribute vec2 aTextureCoors;
        varying vec2 vTextureCoors;

        void main() {
            vTextureCoors = vec2(aTextureCoors.x, 1.0 - aTextureCoors.y);
            gl_Position = aVertexCoors;
        }
);

const char *fragment_shader_yuv420p = GET_STR(
        precision mediump float;
        varying vec2 vTextureCoors;
        uniform sampler2D yTexture;
        uniform sampler2D uTexture;
        uniform sampler2D vTexture;

        void main() {
            vec3 yuv = vec3(texture2D(yTexture, vTextureCoors).r,
                            texture2D(uTexture, vTextureCoors).r - 0.5,
                            texture2D(vTexture, vTextureCoors).r - 0.5);
            vec3 rgb = mat3(1.0, 1.0, 1.0,
                            0.0, -0.39465, 2.03211,
                            1.13983, -0.58060, 0.0) * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
);

const float vertex_coors[]{
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f
};

const float texture_coors[]{
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
};

FFOpenGLShader::FFOpenGLShader() {

}

FFOpenGLShader::~FFOpenGLShader() {

}

GLint initProgram(GLint vertexShader, GLint fragmentShader) {
    GLuint program = glCreateProgram();
    if (program == 0) {
        LOGE(TAG, "create program failed.");
        return 0;
    }
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        LOGE(TAG, "link program failed.");
        glDeleteProgram(program);
        return 0;
    }
    LOGI(TAG, "FFOpenGLShader init program success.");
    return program;
}

GLuint initShader(const char *source, GLint type) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        LOGE(TAG, "create shader failed, %d", shader);
        return 0;
    }
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        LOGE(TAG, "compile shader failed, %s.", source);
        glDeleteShader(shader);
        return 0;
    }
    LOGI(TAG, "FFOpenGLShader init shader success.");
    return shader;
}

void genTexture(unsigned int* textures, int index, int width, int height) {
    if (textures[index]) {
        return;
    }
    GLuint texture;
    glGenTextures(1, &texture);
    LOGI(TAG, "gen texture index %d texture %d", index, texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
                 GL_LUMINANCE, GL_UNSIGNED_BYTE,nullptr);
    textures[index] = texture;
    LOGI(TAG, "FFOpenGLShader gen texture success.");
}

void setupVertexData(GLint glProgram) {
    GLint vertexCoorsLoc = glGetAttribLocation(glProgram, "aVertexCoors");
    glEnableVertexAttribArray(vertexCoorsLoc);
    glVertexAttribPointer(vertexCoorsLoc, 2, GL_FLOAT, GL_FALSE, 0, vertex_coors);

    GLint textureCoorsLoc = glGetAttribLocation(glProgram, "aTextureCoors");
    glEnableVertexAttribArray(textureCoorsLoc);
    glVertexAttribPointer(textureCoorsLoc, 2, GL_FLOAT, GL_FALSE, 0, texture_coors);
}

void setupTextureUniform(unsigned int glProgram) {
    //设置纹理单元
    int textureYLoc = glGetUniformLocation(glProgram, "yTexture");
    int textureULoc = glGetUniformLocation(glProgram, "uTexture");
    int textureVLoc = glGetUniformLocation(glProgram, "vTexture");
    //纹理单元对应GL_TEXTURE2、GL_TEXTURE3、GL_TEXTURE4
    glUniform1i(textureYLoc, 0);
    glUniform1i(textureULoc, 1);
    glUniform1i(textureVLoc, 2);
}

void fillTextureData(int index, unsigned int *yuvTextures, unsigned char *data, int width,
                     int height) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, yuvTextures[index]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                    GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
}

void FFOpenGLShader::init() {
    vertexShader = initShader(vertex_shader, GL_VERTEX_SHADER);
    fragmentShader = initShader(fragment_shader_yuv420p, GL_FRAGMENT_SHADER);
    program = initProgram(vertexShader, fragmentShader);
    LOGE(TAG, "FFOpenGLShader create gl program , %d.", program);
    glUseProgram(program);
    setupVertexData(program);
    setupTextureUniform(program);
}

void FFOpenGLShader::release() {
    if (program) {
        glDeleteProgram(program);
        program = 0;
    }

    if (vertexShader) {
        glDeleteShader(vertexShader);
        vertexShader = 0;
    }

    if (fragmentShader) {
        glDeleteShader(fragmentShader);
        fragmentShader = 0;
    }

    glDeleteTextures(3, yuvTextures);
}

void FFOpenGLShader::fillYUV(int index, unsigned char *data, int width, int height) {
    genTexture(yuvTextures, index, width, height);
    fillTextureData(index, yuvTextures, data, width, height);
}

void FFOpenGLShader::render() {
    glUseProgram(program);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}