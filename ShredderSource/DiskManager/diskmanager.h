#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>

#include "disk_drive.h"

#define DISK_BUFFER 100
#define MAX_DRIVE_DEVICES 15
#define DRIVES_TMP_FILE "/tmp/hd.tmp"
#define GET_DRIVES_CMD  "sudo Scripts/get_drives.sh " DRIVES_TMP_FILE
#define BOOT_TMP_FILE   "/tmp/boot_dev.tmp"
#define GET_BOOT_CMD    "sudo Scripts/get_boot_drive.sh " BOOT_TMP_FILE
#define NO_BOOT_DRIVE   "NO_BOOT"

class DiskManager
{
public:
    DiskManager();
    void Refresh();
    int DrivesCount;
    int nBootDrive;
    int nBootPart;
    disk_drive GetDrive(int i) { return this->drives[i]; }

    char* GetBootDrive();
    char* GetBootPart();

private:
    std::string GetBootDevice();
    void SetBootByName(std::string strBootPart);
    disk_drive drives[MAX_DRIVE_DEVICES];
};

#endif
