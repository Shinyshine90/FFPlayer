
#include "XData.h"

extern "C" {
#include "libavformat/avformat.h"
}

bool XData::alloc(int size, const char *data) {
    type = U_CHAR;
    if (size <= 0) {
        return false;
    }
    this->data = new unsigned char[size];
    if (!this->data) {
        return false;
    }
    this->size = size;
    if (data) {
        memcpy(this->data, data, size * sizeof(char*));
    }
    return true;
}

void XData::release() {
    if (data == nullptr) return;
    if (type == AV_PACKET) {
        AVPacket* packet = reinterpret_cast<AVPacket*>(data);
        av_packet_free(&packet);
    } else {
        delete data;
    }

    data = 0;
    size = 0;
}