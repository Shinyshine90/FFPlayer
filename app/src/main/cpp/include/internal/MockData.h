#ifndef NDKPRACTICE_MOCKDATA_H
#define NDKPRACTICE_MOCKDATA_H
#include <stdio.h>

static const char* YUV_PATH = "/sdcard/image.yuv";

unsigned char** getMockData() {
    int width = 1920, height = 1080;
    unsigned char** yuv = new unsigned char*[]{
            new unsigned char[width * height],
            new unsigned char[width * height / 4],
            new unsigned char[width * height / 4],
    };
    FILE * fp = fopen(YUV_PATH, "rb");
    if (!fp) {

        return yuv;
    }
    fread(yuv[0], 1, width * height, fp);
    fread(yuv[1], 1, width * height / 4, fp);
    fread(yuv[2], 1, width * height / 4, fp);
    return yuv;
}

void freeMockData(unsigned char ** data) {
    for (int i = 0; i < 3; i++) {
        delete data[i];
    }
    delete[] data;
}

#endif //NDKPRACTICE_MOCKDATA_H
