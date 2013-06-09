#ifndef LOGGER_H
#define LOGGER_H

#include <QtGui>
#include "Scrubber/scrublogger.h"


class Logger : public QObject, public ScrubLogger
{
public:
    Logger(QString sLogFilePath);

    void WriteToLog(ScrubLogger::MSG_TYPE msgType, int nResult, const char* sMsg);

Q_OBJECT

signals:
    void SignalLog(Logger::MSG_TYPE, int);
};

// Must register Logger:MSG_TYPE for QueuedConnection
Q_DECLARE_METATYPE(Logger::MSG_TYPE)


#endif // LOGGER_H
