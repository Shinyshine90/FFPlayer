
#ifndef NDKPRACTICE_XDATA_H
#define NDKPRACTICE_XDATA_H

enum XDataType {
    AV_PACKET = 0,
    U_CHAR = 1
};

class XData {
public:
    XDataType type = AV_PACKET;

    long pts = 0;

    unsigned char* data = nullptr; // packet

    unsigned char* datas[8] = { nullptr }; // images

    int width = 0, height = 0;

    bool isAudio = false;

    int size = 0;

    bool alloc(int size, const char* data = 0);

    void release();
};

#endif
