
#ifndef NDKPRACTICE_SLAUDIOPLAY_H
#define NDKPRACTICE_SLAUDIOPLAY_H

#include <list>
#include "IObserver.h"
#include "IAudioPlay.h"

class SLAudioPlay: public IAudioPlay {
public:
    SLAudioPlay();
    virtual ~SLAudioPlay();
    void run() override;
    bool startPlay(XParameter param) override;
    void close() override;
    void playCall(void* slBufferQueue);
protected:
    //std::recursive_mutex mutex;
    unsigned char* buffer = nullptr;
    std::list<XData> slBuffer;

};


#endif //NDKPRACTICE_SLAUDIOPLAY_H
