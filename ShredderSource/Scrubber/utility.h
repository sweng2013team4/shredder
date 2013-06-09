#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <sys/types.h>

class Utility
{
public:

    enum FILE_TYPE
    {
        NOT_EXIST,
        REGULAR,
        DIRECTORY,
        CHAR,
        BLOCK,
        OTHER
    };

    static off64_t GetFileSize(const char* sPath);

    static FILE_TYPE GetFileType(const char* sPath);
    static void SizeToString(char *str, int len, off64_t size);
    static int ReadData(int fdFile, unsigned char* arrData, int nSize);
};

#endif
