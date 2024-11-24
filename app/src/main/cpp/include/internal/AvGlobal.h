
#ifndef NDKPRACTICE_AVGLOBAL_H
#define NDKPRACTICE_AVGLOBAL_H

extern  "C" {
#include "libavcodec/avcodec.h"
};

#include "FFLog.h"

namespace AvGlobal {
    void printAllDecoder();


    void ffmpegLogCallback(void *ptr, int level, const char *fmt, va_list vl);
}

#endif //NDKPRACTICE_AVGLOBAL_H
