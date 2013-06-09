#include "scrubber.h"

#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "utility.h"
#include "random.h"
#include "genrand.h"

// Intentionally last due to lib conflicts
#include <stdio.h>

const std::string Scrubber::sEntryLetters = "abcdefghijklmnopqrstuvwxyz"
                                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "1234567890";

Scrubber::Scrubber(vector<string>* lstFiles, ScrubPattern* scrbPattern, unsigned int nBufferSize, ScrubLogger* logger)
{
    this->lstFiles = lstFiles;
    this->scrbPattern = scrbPattern;
    this->nBufferSize = nBufferSize;
    this->logger = logger;

    this->nTotalWritten = 0LL;
    this->nTotalSize = 0LL;
    this->nCurrPct = -1;

    // Calculate files total size (for progress calculation)
    for (unsigned int i = 0; i < lstFiles->size(); i++)
    {
        long long nSize = Utility::GetFileSize(lstFiles->at(i).c_str());

        if (nSize > 0)
            this->nTotalSize += nSize;
    }
}

Scrubber::~Scrubber()
{
    delete lstFiles;
    delete scrbPattern;
    //delete logger;
}

/*
    Initiates scrub operation
    Return: SCRUB_OK - Successful
            SCRUB_ERR - Errors occured
*/
int Scrubber::Scrub()
{
    int nVal, nResult = SCRUB_OK;

    char* arrBuffer = new char[this->nBufferSize];

    // Report scrub start
    WriteToLog(ScrubLogger::SCRUB_START, 0);

    // Initialize RNG
    if (initrand() != 0)
    {
        snprintf(sLogBuf, MESSAGE_LEN, "ERROR: Failed to initrand().\tfunction Scrub() - "
                "Scrub operation canceled, no changes were made!\n");
        WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

        nResult = SCRUB_ERR;
    }
    else if(!arrBuffer)
    {
        snprintf(sLogBuf, MESSAGE_LEN, "ERROR: Memory allocation failed.\tfunction Scrub() - "
                "Scrub operation canceled, no changes were made!\n");
        WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

        nResult = SCRUB_ERR;
    }
    else
    {
        for (this->nCurrFile = 0; this->nCurrFile < (int)lstFiles->size(); this->nCurrFile++)
        {
            const char* sPath = (*lstFiles)[this->nCurrFile].c_str();

            // Report scrubbing of file started
            WriteToLog(ScrubLogger::SCRUB_FILE_START, this->nCurrFile, sPath);

            nVal = ScrubObject(sPath, arrBuffer);

            if (nVal != SCRUB_OK)
                nResult = SCRUB_ERR;

            // Report scrubbing of file ended with result
            WriteToLog(ScrubLogger::SCRUB_FILE_END, nResult);
        }

        delete [] arrBuffer;
    }

    // Report scrub operation ended with result
    WriteToLog(ScrubLogger::SCRUB_END, nResult);

    return nResult;
}

/*
   Scrubs object sPath entry and data if possible.
   Returns: SCRUB_OK  - Successful
            SCRUB_ERR - Errors occured
*/
int Scrubber::ScrubObject(const char* sPath, char* arrBuffer)
{
    int nVal, nResult = SCRUB_OK;
    Utility::FILE_TYPE fType = Utility::GetFileType(sPath);

    // Check if file can be scrubbed
    if(fType != Utility::DIRECTORY)
    {
        int fdFile;
        int flags = O_RDWR;
        off64_t nFileSize;

        // Open file for writing
        fdFile = open(sPath, flags, 0644);

        // Check for open errors
        if(fdFile < 0)
        {
            // Report open error
            snprintf(sLogBuf, MESSAGE_LEN, "ERROR: %s. Occured while opening file %s. Inside function ScrubObject().",
                    strerror(errno), sPath);

            WriteToLog(ScrubLogger::SCRUB_OPEN_ERROR, this->nCurrFile, sLogBuf);

            nResult = SCRUB_ERR;
        }
        else
        {
            nFileSize = Utility::GetFileSize(sPath);

            if (nFileSize == -1)
            {
                nResult = SCRUB_ERR;

                snprintf(sLogBuf, MESSAGE_LEN, "ERROR: Unable to get file size: %s. File skipped.\nInside function ScrubObject().\n",
                         sPath);
                WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);
            }
            else
            {
                // Scrub data
                nVal = ScrubData(fdFile, nFileSize, arrBuffer);

                // Check if scrub was successful
                if(nVal != SCRUB_OK)
                {
                    nResult = SCRUB_ERR;
                }
            }

            // Close file
            if (close(fdFile) < 0)
            {
                snprintf(sLogBuf, MESSAGE_LEN, "ERROR: %s - Occured while closing file %s.\nInside function ScrubObject().\n",
                        strerror(errno), sPath);
                WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

                nResult = SCRUB_ERR;
            }
        }
    }

    // Check if file/directory for entry scrub
    if ((nResult == SCRUB_OK) &&
        (fType == Utility::REGULAR || fType == Utility::DIRECTORY))
    {
        char* sNewPath;

        // Report scrub entry start
        WriteToLog(ScrubLogger::SCRUB_ENTRY_START, this->nCurrFile);

        // Scrub entry
        if(ScrubEntry(sPath, sNewPath) != SCRUB_OK)
            nResult = SCRUB_ERR;

        // Delete file/directory
        if(DeleteEntry(sNewPath) != SCRUB_OK)
            nResult = SCRUB_ERR;

        // Report scrub entry end with result
        WriteToLog(ScrubLogger::SCRUB_ENTRY_END, nResult);

        delete [] sNewPath;
    }


    return nResult;
}


/*
   Scrubs the given file data by overwriting
   Returns: SCRUB_OK - Successful
            SCRUB_VERIFY_ERR - Verification failure
            SCRUB_ERR - Errors occured
*/
int Scrubber::ScrubData(int fdFile, off64_t nFileSize, char* arrBuffer)
{
    int nVal, nResult = SCRUB_OK;

    bool bRandomize;
    unsigned int nOptBuffSize;

    // Loop through pattern passes
    for(unsigned int nCurr = 0; nCurr < scrbPattern->lstPasses.size(); nCurr++)
    {
        // Get current item
        ScrubPass psCurr = scrbPattern->lstPasses[nCurr];

        // Check if random pass
        if (scrbPattern->lstPasses[nCurr].pType == ScrubPass::RANDOM)
        {
            bRandomize = true;
            nOptBuffSize = this->nBufferSize;
        }
        else
        {
           bRandomize = false;
           nOptBuffSize = GetOptBuffSize(this->nBufferSize, psCurr.arr.size());
           FillBuffer(arrBuffer, &psCurr.arr);
        }

        // Report scrub pass start
        WriteToLog(ScrubLogger::SCRUB_PASS_START, nCurr);

        // Fill file and check if successful
        if (FillFile(fdFile, nFileSize, arrBuffer, nOptBuffSize, bRandomize) < 0)
        {
            nResult = SCRUB_ERR;
        }

        // Report scrub pass ended with result
        WriteToLog(ScrubLogger::SCRUB_PASS_END, nResult);

        // Check if we need to verify
        if (scrbPattern->lstPasses[nCurr].pType == ScrubPass::VERIFY)
        {
            // Report verify file start
            WriteToLog(ScrubLogger::SCRUB_VERIFY_START, nCurr);

            // Verify file
            nVal = VerifyFile(fdFile, nFileSize, arrBuffer, nOptBuffSize);

            if (nResult == SCRUB_OK && nVal != SCRUB_OK)
                nResult = nVal;

            // Report verify file end
            WriteToLog(ScrubLogger::SCRUB_VERIFY_END, nVal);
        }
    }

    return nResult;
}

/*
   Scrubs file\directory entry by renaming it numerous times.
   sNewFilePath will contain the new file path name. It is allocated and must be freed after use
   Returns: SCRUB_OK - Successful
            SCRUB_ERR - Renaming error
*/
int Scrubber::ScrubEntry(const char *sPath, char*& sNewFilePath)
{
    int nResult = SCRUB_OK;
    char* sNewPath;
    char* sOldPath;

    int nPathLen = strlen(sPath);
    int nNameIndex = 0;


    // Find name index (skip first char for directories)
    for (int i = 1; i < nPathLen; i++ )
    {
        if (sPath[nPathLen - i - 1] == '/')
        {
            nNameIndex = nPathLen - i;
            break;
        }
    }

    // Allocate space for entry buffer
    sOldPath = new char[nNameIndex + ENTRY_LENGTH + 1];
    sNewPath = new char[nNameIndex + ENTRY_LENGTH + 1];

    // Copy path
    strncpy(sOldPath, sPath, nNameIndex + ENTRY_LENGTH);

    // Copy up to file name start
    strncpy(sNewPath, sOldPath, nNameIndex);

    // null terminators
    sOldPath[nNameIndex + ENTRY_LENGTH] = 0;
    sNewPath[nNameIndex + RAND_ENTRY_LENGTH] = 0;

    for (int i = 0; i < SCRUB_ENTRY_NUM; i++)
    {
        // Get new file name
        Random::GetInstance()->FillRandString(&sNewPath[nNameIndex], RAND_ENTRY_LENGTH, &Scrubber::sEntryLetters);



        // Attempt to rename and check for errors
        if(rename(sOldPath, sNewPath) != 0)
        {
            // Write the error to log file
            snprintf(sLogBuf, MESSAGE_LEN, "ERROR: %s. Occured while renaming file %s. Inside function ScrubEntry(), entry was not fully scrubbed.\n",
                    strerror(errno), sPath);
            WriteToLog(ScrubLogger::SCRUB_RENAME_ERROR, this->nCurrFile, sLogBuf);

            nResult = SCRUB_ERR;

            break;
        }
        else
        {
            // Sync file/directory
            int fd = open(sNewPath, O_RDONLY);
            if (fd > 0)
            {
                fsync(fd);
                close(fd);
            }

            // Copy new path to old
            strncpy(&sOldPath[nNameIndex], &sNewPath[nNameIndex], ENTRY_LENGTH);
        }
    }

    // Set new file path (equal to old since new path is always copied to old)
    sNewFilePath = sOldPath;
    delete [] sNewPath;

    return nResult;
}

int Scrubber::DeleteEntry(const char* sPath)
{    
    int nResult = SCRUB_OK;

    if (remove(sPath) != 0)
    {
        nResult = SCRUB_ERR;
    }

    return nResult;
}

/*
   Fills file with given data
   Returns SCRUB_ERR in case of error, otherwise returns SCRUB_OK
*/
int Scrubber::FillFile(int fdFile, off64_t nFileSize, char* arrData, unsigned int nSize, bool bRandomize)
{
    //       Add progress update
    off64_t nWritten = 0LL;

    int nBytes = 0;

    lseek(fdFile, 0, SEEK_SET);

    if (bRandomize)
    {
        if (churnrand() != 0)
        {
            snprintf(sLogBuf, MESSAGE_LEN, "ERROR: Occured in churnrand(). Inside function FillFile().\n");
            WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

            return SCRUB_ERR;
        }
    }

    while (nWritten < nFileSize)
    {
        // Randomise buffer if needed
        if (bRandomize)
        {
            genrand(arrData, nSize);
            //Random::GetInstance()->FillRand(arrData, nSize, 0, 255);
        }

        // Make sure we won't write out of file bounds
        if (nWritten + nSize > nFileSize)
        {
            nSize = nFileSize - nWritten;
        }

        // Write Data
        nBytes = WriteData(fdFile, arrData, nSize);

        if (nBytes < 0)
        {
            snprintf(sLogBuf, MESSAGE_LEN, "ERROR: %s. Occured while writing data to file %s. Inside function FillFile().\n",
                    strerror(errno), this->lstFiles->at(this->nCurrFile).c_str());
            WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

            this->nTotalWritten += nFileSize - nWritten;

            return SCRUB_ERR;
        }

        nWritten += nBytes;
        this->nTotalWritten += nBytes;

        // Report progress
        UpdateProgress();
    }

    return SCRUB_OK;
}

/*
   Verifies the file matches the pattern in arrData
   Returns: SCRUB_OK - Successful
            SCRUB_VERIFY_ERR - Verification failed
            SCRUB_ERR - Errors occured
*/
int Scrubber::VerifyFile(int fdFile, off64_t nFileSize, char* arrData, unsigned int nSize)
{
    off64_t nRead = 0LL;

    char* arrBuffer = new char[nSize];

    if(!arrBuffer)
    {
        snprintf(sLogBuf, MESSAGE_LEN, "ERROR: Memory allocation failed. Occurred while handling file %s."
                " Inside function VerifyFile(), file was not verified!\n",
                this->lstFiles->at(this->nCurrFile).c_str());
        WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

        return SCRUB_ERR;
    }

    int nBytes = 0;

    while (nRead < nFileSize)
    {
        // Make sure we won't read out of file bounds
        if (nRead + nSize > nFileSize)
        {
            nSize = nFileSize - nRead;
        }

        // Read Data
        nBytes = ReadData(fdFile, arrBuffer, nSize);

        if (nBytes < 0)
        {
            snprintf(sLogBuf, MESSAGE_LEN, "ERROR: %s. Occured while reading from file %s.Inside function VerifyFile().\n",
                    strerror(errno), this->lstFiles->at(this->nCurrFile).c_str());
            WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

            return SCRUB_ERR;
        }

        // Compare data read with pattern
        if(memcmp(arrBuffer, arrData, nSize) != 0)
        {
            snprintf(sLogBuf, MESSAGE_LEN, "ERROR: Verification failed, file does not match pattern. "
                    "Occured while verifying data of file %s.Inside function VerifyFile().\n",
                    this->lstFiles->at(this->nCurrFile).c_str());
            WriteToLog(ScrubLogger::SCRUB_ERROR, this->nCurrFile, sLogBuf);

            // Exit - file does not match pattern
            return SCRUB_VERIFY_ERR;
        }

        nRead += nSize;
    }

    delete [] arrBuffer;

    return SCRUB_OK;
}

/*
   Writes data to file, making sure no partial writes are done
   Returns -1 if errors occured, otherwise returns number of bytes written
*/
int Scrubber::WriteData(int fdFile, const char* arrData, unsigned int nSize)
{
    int nBytes;

    do
    {
        nBytes = write(fdFile, arrData, nSize);

        // Check for partial write
        if (nBytes > 0)
        {
            nSize -= nBytes;
            arrData += nBytes;
        }
    }
    while (nBytes > 0 && nSize > 0);

    // Make sure data block was written to disk
    //fsync(fdFile);

    return nBytes;
}

/*
   Reads data from file, making sure no partial reads are done
   Returns -1 if errors occured, otherwise returns number of bytes read
*/
int Scrubber::ReadData(int fdFile, char* arrData, unsigned int nSize)
{
    int nBytes;

    do
    {
        nBytes = read(fdFile, arrData, nSize);

        // Check for partial read
        if (nBytes > 0)
        {
            nSize -= nBytes;
            arrData += nBytes;
        }
    }
    while (nBytes > 0 && nSize > 0);

    return nBytes;
}

unsigned int Scrubber::GetOptBuffSize(unsigned int nBufferSize, unsigned int nPatternSize)
{
    return (nBufferSize - (nBufferSize % nPatternSize));
}

/*
   Fills given buffer with vector
*/
void Scrubber::FillBuffer(char* arrBuffer, const vector<char>* vec)
{
    int i, j;

    int nVecSize = vec->size();

    for (i = 0, j = 0; i < (int)this->nBufferSize; i++)
    {
        arrBuffer[i] = vec->at(j++);

        if(j == nVecSize)
           j = 0;
    }
}

/*
   Writes messages to log
*/
void Scrubber::WriteToLog(ScrubLogger::MSG_TYPE msgType, int nResult, const char* sMsg)
{
    if (this->logger)
    {
        logger->WriteToLog(msgType, nResult, sMsg);
    }
}

void Scrubber::UpdateProgress()
{
    int nPct;
    float fPct = ((this->nTotalWritten)/((long double)nTotalSize) * 100);

    // Divide by number of passes
    // nTotalSize isn't multiplied by number of passes in effort to not exceed off64_t bounds
    fPct /= (float)this->scrbPattern->lstPasses.size();

    nPct = (int)fPct;

    // Round up if >= 0.5
    if ((fPct - (float)nPct) >= 0.5f)
        nPct++;

    if (this->nCurrPct < nPct)
    {
        this->nCurrPct = nPct;
        WriteToLog(ScrubLogger::SCRUB_PROGRESS, nPct);
    }
}




