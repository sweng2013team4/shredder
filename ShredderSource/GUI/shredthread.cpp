#include "shredthread.h"

#include "GUI/shredder.h"
#include "GUI/logger.h"

ShredThread::ShredThread(Scrubber* scrubber)
{
    this->scrubber = scrubber;

    if (scrubber->logger)
    {
        connect(((Logger*)this->scrubber->logger),
                SIGNAL(SignalLog(Logger::MSG_TYPE,int)),
                Shredder::GetInstance(),
                SLOT(HandleSignal(Logger::MSG_TYPE,int)));
    }
}

ShredThread::~ShredThread()
{
    delete (Logger*)scrubber->logger;
    delete scrubber;
}

void ShredThread::run()
{
    scrubber->Scrub();

    if(scrubber->logger)
    {
        disconnect(((Logger*)this->scrubber->logger),
                   SIGNAL(SignalLog(Logger::MSG_TYPE,int)),
                   Shredder::GetInstance(),
                   SLOT(HandleSignal(Logger::MSG_TYPE,int)));
    }
}
