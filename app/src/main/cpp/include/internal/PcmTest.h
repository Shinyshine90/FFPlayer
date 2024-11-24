

#ifndef NDKPRACTICE_PCMTEST_H
#define NDKPRACTICE_PCMTEST_H


#include <thread>
#include <stdio.h>
#include "FFOpenSLAudioPlay.h"

void testPcm() {
    //44100*4
    ///sdcard/out.pcm
    FFOpenSLAudioPlay audioPlay;
    audioPlay.initialize();
    audioPlay.start();
    FILE * f = fopen("/sdcard/out.pcm", "rb");
    short* buffer = new short[1024];
    int lens;
    while (0 < (lens = fread(buffer, 1, 1024, f))) {
        uint8_t * data = new uint8_t[lens];
        memcpy(data, buffer, lens);
        audioPlay.enqueuePcmData(new FFPcmData(data, lens));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

}

#endif //NDKPRACTICE_PCMTEST_H
