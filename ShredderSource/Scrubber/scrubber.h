#ifndef SCRUBBER_H
#define SCRUBBER_H

#include <string>
#include <vector>
#include <sys/types.h>

#include "scrubpattern.h"
#include "scrublogger.h"

#define SCRUB_OK            1
#define SCRUB_ERR          -1
#define SCRUB_VERIFY_ERR   -2

#define SCRUB_ENTRY_NUM     20
#define ENTRY_LENGTH        256
#define RAND_ENTRY_LENGTH   100
#define MESSAGE_LEN         10000

//typedef long long off64_t;

using namespace std;

class Scrubber
{
public:
    Scrubber(vector<string>* lstFiles, ScrubPattern* scrbPattern, unsigned int nBufferSize, ScrubLogger* logger = NULL);
    ~Scrubber();

    vector<string>* lstFiles;
    unsigned int nBufferSize;

    ScrubPattern* scrbPattern;
    ScrubLogger* logger;

    int Scrub();

private:
    char sLogBuf[MESSAGE_LEN];
    int  nCurrFile;
    off64_t nTotalWritten;
    off64_t nTotalSize;
    int nCurrPct;

    static const std::string sEntryLetters;

    int ScrubObject(const char* sPath, char* arrBuffer);
    int ScrubData(int fdFile, off64_t nFileSize, char* arrBuffer);
    int ScrubEntry(const char* sPath, char*& sNewFilePath);
    int DeleteEntry(const char* sPath);
    int FillFile(int fdFile, off64_t nFileSize, char* arrData, unsigned int nSize, bool bRandomise = false);
    int VerifyFile(int fdFile, off64_t nFileSize, char* arrData, unsigned int nSize);
    int WriteData(int fdFile, const char* arrData, unsigned int nSize);
    int ReadData(int fdFile, char* arrData, unsigned int nSize);
    unsigned int GetOptBuffSize(unsigned int nBufferSize, unsigned int nPatternSize);
    void FillBuffer(char* arrBuffer, const vector<char>* vec);
    void WriteToLog(ScrubLogger::MSG_TYPE msgType, int nResult, const char* sMsg = NULL);
    void UpdateProgress();
};

#endif // SCRUBBER_H
