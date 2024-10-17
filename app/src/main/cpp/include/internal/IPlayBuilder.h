//
// Created by Shadowalker on 10/12/2024.
//

#ifndef NDKPRACTICE_IPLAYBUILDER_H
#define NDKPRACTICE_IPLAYBUILDER_H

#include "IDemux.h"
#include "IDecoder.h"
#include "IResample.h"
#include "IVideoView.h"
#include "IAudioPlay.h"
#include "SLAudioPlay.h"

#include "IPlayer.h"

class IPlayBuilder {
public:
    virtual IPlayer* build(unsigned char index);
protected:
    virtual IDemux* createDemux() = 0;
    virtual IDecoder* createVideoDecoder() = 0;
    virtual IDecoder* createAudioDecoder() = 0;
    virtual IVideoView* createVideoView() = 0;
    virtual IResample* createResample() = 0;
    virtual IAudioPlay* createAudioPlay() = 0;
};


#endif
