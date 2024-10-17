//
// Created by Shadowalker on 10/12/2024.
//

#ifndef NDKPRACTICE_FFPLAYBUILDER_H
#define NDKPRACTICE_FFPLAYBUILDER_H


#include "IPlayer.h"
#include "IPlayBuilder.h"

class FFPlayBuilder : public IPlayBuilder {
protected:
    virtual IDemux* createDemux() override;
    virtual IDecoder* createVideoDecoder() override;
    virtual IDecoder* createAudioDecoder() override;
    virtual IVideoView* createVideoView() override;
    virtual IResample* createResample() override;
    virtual IAudioPlay* createAudioPlay() override;
};


#endif
