#include "logger.h"

#include <QtGui>
#include <stdio.h>

Logger::Logger(QString sLogFilePath):ScrubLogger(sLogFilePath.toStdString())
{
}

void Logger::WriteToLog(ScrubLogger::MSG_TYPE msgType, int nVal, const char* sMsg)
{
    // Signal to GUI thread with event
    emit SignalLog(msgType, nVal);

    ScrubLogger::WriteToLog(msgType, nVal, sMsg);
}
