#ifndef NDKPRACTICE_XPARAMETER_H
#define NDKPRACTICE_XPARAMETER_H

struct AVCodecParameters;

class XParameter {
public:
    AVCodecParameters *avCodecParameters = nullptr;
    int channels;
    int sample_rate;
};

#endif
