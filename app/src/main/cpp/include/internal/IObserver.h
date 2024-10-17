#ifndef NDKPRACTICE_IOBSERVER_H
#define NDKPRACTICE_IOBSERVER_H

#include "XData.h"
#include <vector>
#include <mutex>

class IObserver {
public:
    virtual void update(XData data) {};

    void addObserver(IObserver* observer);

    void removeObserver(IObserver* observer);

    void notify(XData data);

protected:
    std::recursive_mutex mutex;
    std::vector<IObserver*> observers;
};
#endif
