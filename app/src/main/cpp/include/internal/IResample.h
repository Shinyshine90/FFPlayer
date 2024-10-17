#ifndef NDKPRACTICE_IRESAMPLE_H
#define NDKPRACTICE_IRESAMPLE_H
#include "XParameter.h"
#include "XData.h"
#include "IObserver.h"

class IResample : public IObserver {
public:
    virtual bool open(XParameter in, XParameter out = XParameter()) = 0;
    virtual void close() = 0;
    virtual XData resample(XData data) = 0;
    virtual void update(XData data);
};


#endif //NDKPRACTICE_IRESAMPLE_H
