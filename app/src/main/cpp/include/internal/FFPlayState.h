
#ifndef NDKPRACTICE_FFPLAYSTATE_H
#define NDKPRACTICE_FFPLAYSTATE_H

enum FFPlayState {
    INIT = 0,
    PREPARE,
    PLAYING,
    PAUSE,
    SEEK,
    STOP,
    RELEASE
};

#endif
