
#include "utility.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <unistd.h>

/*
   Gets the file size, if error occured returns -1
*/
off64_t Utility::GetFileSize(const char* sPath)
{
    off64_t nSize = -1;

    struct stat status;

    if (stat(sPath, &status) == 0)
    {
        nSize = status.st_size;
    }

    // Check if we need to try other method
    if (nSize <= 0)
    {
        int fdFile = open(sPath, O_RDONLY);

        if (fdFile >= 0)
        {
            // Seek
            nSize = lseek(fdFile, 0, SEEK_END);

            if (nSize <= 0)
            {
                // Rewind
                lseek(fdFile, 0, SEEK_SET);

                // Get size
                ioctl(fdFile, BLKGETSIZE64, &nSize);
            }

            close(fdFile);
        }
    }

    return nSize;
}

/*
   Gets the file type using stat(2)
*/
Utility::FILE_TYPE Utility::GetFileType(const char* sPath)
{
    struct stat st;

    FILE_TYPE fType = NOT_EXIST;

    if (stat(sPath, &st) == 0)
    {
        if (S_ISREG(st.st_mode))
            fType = REGULAR;
        else if (S_ISDIR(st.st_mode))
            fType = DIRECTORY;
        else if (S_ISCHR(st.st_mode))
            fType = CHAR;
        else if (S_ISBLK(st.st_mode))
            fType = BLOCK;
        else
            fType = OTHER;
    }

    return fType;
}


/*
   Reads data from file, making sure no partial reads are done
   Returns -1 if errors occured, otherwise returns number of bytes read
*/
int Utility::ReadData(int fdFile, unsigned char* arrData, int nSize)
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
