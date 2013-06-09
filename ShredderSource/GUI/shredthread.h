#ifndef SHREDTHREAD_H
#define SHREDTHREAD_H

#include <QtGui>

#include "Scrubber/scrubber.h"

class ShredThread : public QThread
{
public:
    Scrubber* scrubber;

public:
    ShredThread(Scrubber* scrubber);
    ~ShredThread();

protected:
    void run();
};

#endif // SHREDTHREAD_H
