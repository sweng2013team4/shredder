#ifndef SCRUBLOGGER_H
#define SCRUBLOGGER_H

#include <string>

using namespace std;

class ScrubLogger
{

public:
    enum MSG_TYPE
    {
        // Events
        SCRUB_START,
        SCRUB_FILE_START,   // nVal = current file
        SCRUB_PASS_START,   // nVal = current pass
        SCRUB_PROGRESS,     // nVal = progress precentage
        SCRUB_PASS_END,     // nVal = result
        SCRUB_VERIFY_START, // nVal = current pass
        SCRUB_VERIFY_END,   // nVal = result
        SCRUB_ENTRY_START,  // nVal = current file
        SCRUB_ENTRY_END,    // nVal = result
        SCRUB_FILE_END,     // nVal = result
        SCRUB_END,          // nVal = result

        // Errors
        SCRUB_OPEN_ERROR,   // nVal = current file
        SCRUB_VERIFY_ERROR, // nVal = current file
        SCRUB_RENAME_ERROR, // nVal = current file
        SCRUB_ERROR         // nVal = current file
    };

    string sLogFilePath;

    ScrubLogger(std::string sLogFilePath);
    virtual void WriteToLog(MSG_TYPE msgType, int nVal, const char* sMsg);
};

#endif // SCRUBLOGGER_H
