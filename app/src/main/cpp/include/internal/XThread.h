#ifndef NDKPRACTICE_XTHREAD_H
#define NDKPRACTICE_XTHREAD_H

void sleep(int ms);

unsigned long getThreadId();

class XThread {
public:
    virtual bool start();
    virtual void stop();
    virtual void pause();
    virtual void resume();
    virtual void run() = 0;
    virtual bool isPause();

protected:
    volatile bool isRunning = false;
    volatile bool isInterrupted = false;
    volatile bool isPaused = false;

private:
    void runInternal();
};
#endif
