

#ifndef NDKPRACTICE_PCMTEST_H
#define NDKPRACTICE_PCMTEST_H

#include <thread>
#include <stdio.h>
#include "FFOpenSLAudioPlay.h"
#include "FFDemux.h"
#include "AvGlobal.h"

#include <thread>

FFOpenSLAudioPlay audioPlay;

void testPcm() {
    ///sdcard/out.pcm

    audioPlay.initialize();
    audioPlay.start();


    std::thread t([](){
        FILE * f = fopen("/sdcard/out.pcm", "rb");
        uint8_t * buffer = new uint8_t[1024];
        int lens;
        while (0 < (lens = fread(buffer, 1, 1024, f))) {
            if (audioPlay.isPcmQueueOverflow()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            uint8_t * data = new uint8_t[lens];
            memcpy(data, buffer, lens);
            audioPlay.enqueuePcmData(new FFPcmData(data, lens));
        }
    });
    t.detach();
}

FFDemux demux;

void testDemux() {
    //
    std::thread t([](){

    });
    t.detach();

    demux.setUrl("/sdcard/curry.mp4");
    demux.prepare();
    demux.start();

    demux.registerAvPacketCallback(FFMediaType::AUDIO, [](AVPacket* packet){
        LOGI("TestDemux", "receive audio pkt, size = %d.", packet->size);
        AvGlobal::freeAvPacket(packet);
    });
    demux.registerAvPacketCallback(FFMediaType::VIDEO, [](AVPacket* packet){
        LOGI("TestDemux", "receive video pkt, size = %d.", packet->size);
        AvGlobal::freeAvPacket(packet);
    });

}

#endif //NDKPRACTICE_PCMTEST_H
