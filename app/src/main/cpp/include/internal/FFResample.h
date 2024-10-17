#ifndef NDKPRACTICE_FFRESAMPLE_H
#define NDKPRACTICE_FFRESAMPLE_H
#include "IResample.h"

struct SwrContext;

class FFResample : public IResample {
public:
    bool open(XParameter in, XParameter out = XParameter()) override;
    void close() override;
    XData resample(XData data) override;
protected:
    SwrContext *swrContext = nullptr;
    std::recursive_mutex mutex;
};
#endif //NDKPRACTICE_FFRESAMPLE_H
