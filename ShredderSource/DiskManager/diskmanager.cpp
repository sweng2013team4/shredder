#include "diskmanager.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

using namespace std;

DiskManager::DiskManager()
{
    Refresh();

    this->SetBootByName(GetBootDevice());
}

// Gets disks/partitions info
void DiskManager::Refresh()
{
    int nSuccess, nDiskIndex = 0, nPartitionIndex, nTemp;
    char strTemp[DISK_BUFFER];
    FILE *fDiskData;

    // Clearing the disks list.
    memset(this->drives, '\0', sizeof(struct disk_drive) * MAX_DRIVE_DEVICES);

    // Running the command.
    nSuccess = system(GET_DRIVES_CMD);

    if (!nSuccess)
    {
       // Trying to read from the temp file.
        fDiskData = fopen(DRIVES_TMP_FILE, "r");
        if (fDiskData != NULL)
        {
            while (fgets(strTemp, DISK_BUFFER, fDiskData) != NULL && nDiskIndex < MAX_DRIVE_DEVICES)
            {
                // Setting the current drive's label.
                strTemp[strlen(strTemp) - 1] = '\0';
                strcpy(this->drives[nDiskIndex].DriveLabel, &strTemp[5]);

                // Clearing the temporary string.
                memset(strTemp, '\0', sizeof(char) * DISK_BUFFER);

                this->drives[nDiskIndex].DrivePartitionsCount = 0;
                nPartitionIndex = 0;
                fgets(strTemp, DISK_BUFFER, fDiskData);
                while (strTemp[0] == this->drives[nDiskIndex].DriveLabel[0])
                {
                    strTemp[strlen(strTemp) - 1] = '\0';
                    strTemp[4] = '\0';
                    strcpy(this->drives[nDiskIndex].DrivePartitions[nPartitionIndex].PartitionLabel, strTemp);
                    this->drives[nDiskIndex].DrivePartitions[nPartitionIndex].PartitionSizeInBytes = atol(&strTemp[5]);
                    ++nPartitionIndex;
                    ++this->drives[nDiskIndex].DrivePartitionsCount;
                    memset(strTemp, '\0', sizeof(char) * DISK_BUFFER);
                    fgets(strTemp, DISK_BUFFER, fDiskData);
                }

                // Reading the current drive's capacity & writing it to the drive details.
                strTemp[strlen(strTemp) - 1] = '\0';
                strcpy(this->drives[nDiskIndex].DriveCapacity, strTemp);

                // Reading the current drive's heads & writing it to the drive details.
                fscanf(fDiskData, "%d", &nTemp);
                fgets(strTemp, DISK_BUFFER, fDiskData);
                this->drives[nDiskIndex].DriveHeads = nTemp;

                // Reading the current drive's sectors/track & writing it to the drive details.
                fscanf(fDiskData, "%d", &nTemp);
                fgets(strTemp, DISK_BUFFER, fDiskData);
                this->drives[nDiskIndex].DriveSectorsPerTrack = nTemp;

                // Reading the current drive's cylinders & writing it to the drive details.
                fscanf(fDiskData, "%d", &nTemp);
                fgets(strTemp, DISK_BUFFER, fDiskData);
                this->drives[nDiskIndex].DriveCylinders = nTemp;
/*
                // Reading the current drive's size in bytes & writing it to the drive details.
                fscanf(fDiskData, "%l", &lTemp);
                printf ("shit is %l", lTemp);
                fgets(strTemp, DISK_BUFFER, fDiskData);
                this->drives[nDiskIndex].DriveSizeInBytes = lTemp;
*/                           
                ++nDiskIndex;
            }
        }
    }

    this->DrivesCount = nDiskIndex;
}

string DiskManager::GetBootDevice()
{
    int nFailed;
    ifstream ifBoot;
    string strBoot;

    // Run the script
    nFailed = system(GET_BOOT_CMD);

    if (!nFailed)
    {
        // Open result file
        ifBoot.open(BOOT_TMP_FILE);

        // Check if file was opened
        if(!ifBoot)
        {
            std::cerr << "Error opening file." << endl;
            return "";
        }

        // Read the first line
        ifBoot >> strBoot;
        if (ifBoot.eof())
        {
            return "";
        }

        // Close file
        ifBoot.close();
    }

    int nStart = strBoot.find_last_of('/');

    return strBoot.substr(nStart+1 , strBoot.length() - nStart);
}

void DiskManager::SetBootByName(string strBootPart)
{
    if(strBootPart != "")
    {
        for(int i = 0; i < this->DrivesCount; i++)
        {
            for(int j = 0; j < this->drives[i].DrivePartitionsCount; j++)
            {
                if(strBootPart == this->drives[i].DrivePartitions[j].PartitionLabel)
                {
                   this->nBootDrive = i;
                   this->nBootPart  = j;

                   return;
                }
            }
        }
    }

    this->nBootDrive = -1;
    this->nBootPart  = -1;
}

char* DiskManager::GetBootPart()
{
    if(nBootDrive == -1)
        return NO_BOOT_DRIVE;

    return this->drives[nBootDrive].DrivePartitions[nBootPart].PartitionLabel;
}

char* DiskManager::GetBootDrive()
{
    if(nBootDrive == -1)
        return NO_BOOT_DRIVE;

    return this->drives[nBootDrive].DriveLabel;
}
