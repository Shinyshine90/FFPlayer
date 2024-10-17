
#include "FFPlayBuilder.h"
#include "FFDemux.h"
#include "FFDecoder.h"
#include "GLVideoView.h"
#include "FFResample.h"

IDemux *FFPlayBuilder::createDemux() {
    return new FFDemux;
}

IDecoder *FFPlayBuilder::createVideoDecoder() {
    return new FFDecoder;
}

IDecoder *FFPlayBuilder::createAudioDecoder() {
    return new FFDecoder;
}

IVideoView *FFPlayBuilder::createVideoView() {
    return new GLVideoView;
}

IResample *FFPlayBuilder::createResample() {
    return new FFResample;
}

IAudioPlay *FFPlayBuilder::createAudioPlay() {
    return new SLAudioPlay;
}
