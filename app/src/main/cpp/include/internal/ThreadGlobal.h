#ifndef NDKPRACTICE_THREADGLOBAL_H
#define NDKPRACTICE_THREADGLOBAL_H

namespace ThreadGlobal {

    void sleep(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};

#endif
