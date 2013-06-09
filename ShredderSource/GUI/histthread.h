#ifndef HISTTHREAD_H
#define HISTTHREAD_H

#include <QThread>
#include "histogram.h"

class HistThread : public QThread
{
    Q_OBJECT

public:
    HistThread(Histogram* frmHist);

private:
    Histogram* frmHist;

protected:
    void run();

signals:
    void DrawSignal();
};

#endif // HISTTHREAD_H
