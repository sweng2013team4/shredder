#include "histthread.h"

#include "histogram.h"

HistThread::HistThread(Histogram* frmHist)
{
    this->frmHist = frmHist;

    // Connect histogram's update signal to it's draw slot
    // Lets us draw since we signal to the GUI thread
    connect(this->frmHist, SIGNAL(UpdateHistSignal(int)), this->frmHist, SLOT(DrawProgress(int)));

    // Signal to draw histogram when calculation is done
    connect(this, SIGNAL(DrawSignal()), this->frmHist, SLOT(DrawHistogram()));

    // Signal to terminate thread if window was closed
    connect(this->frmHist, SIGNAL(CloseSignal()), this, SLOT(terminate()));
}

void HistThread::run()
{
    // Calculate histogram
    this->frmHist->CalcHistogram();

    // Emit done signal
    emit DrawSignal();

    // Break all signals connected to target window
    disconnect(this, 0, this->frmHist, 0);
}

