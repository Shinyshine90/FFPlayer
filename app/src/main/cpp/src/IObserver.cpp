
#include "IObserver.h"

void IObserver::addObserver(IObserver *observer) {
    if (observer == nullptr) {
        return;
    }
    mutex.lock();
    observers.push_back(observer);
    mutex.unlock();
}

void IObserver::removeObserver(IObserver *observer) {
    mutex.lock();
    observers.erase(std::remove(observers.begin(), observers.end(), observer));
    mutex.unlock();
}

void IObserver::notify(XData data) {
    mutex.lock();
    for(const auto& observer: observers) {
        observer->update(data);
    }
    mutex.unlock();
}
