#include "scrublogger.h"

#include <stdio.h>

ScrubLogger::ScrubLogger(string sLogFilePath)
{
    this->sLogFilePath = sLogFilePath;
}

void ScrubLogger::WriteToLog(MSG_TYPE msgType, int nVal, const char* sMsg)
{
    // Check if error
    //if (msgType > ScrubLogger::SCRUB_END)
    {
        FILE* fpFile = fopen(sLogFilePath.c_str(), "a");

        if (fpFile)
        {
            if (sMsg)
                fprintf(fpFile, "Info: %s.\tMessage Type = %d; nVal = %d;\n",sMsg, msgType, nVal);
            else
                fprintf(fpFile, "Info: Type = %d; nVal = %d;\n", msgType, nVal);

            fclose(fpFile);
        }
    }
}
