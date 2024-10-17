#include "IPlayBuilder.h"

#include "IPlayer.h"

IPlayer *IPlayBuilder::build(unsigned char index) {
    IPlayer* player = IPlayer::getInstance(index);

    player->demux = createDemux();

    player->videoDecoder = createVideoDecoder();

    player->videoView = createVideoView();

    player->audioDecoder = createAudioDecoder();

    player->resample = createResample();

    player->audioPlay = createAudioPlay();

    player->demux->addObserver(player->videoDecoder);

    player->demux->addObserver(player->audioDecoder);

    player->videoDecoder->addObserver(player->videoView);

    player->audioDecoder->addObserver(player->resample);

    player->resample->addObserver(player->audioPlay);

    return player;
}