#ifndef DISK_DRIVE_H
#define DISK_DRIVE_H

#define DRIVE_MAX_LENGTH 10
#define MAX_PARTITION_COUNT 10

struct partition
{
    char PartitionLabel[DRIVE_MAX_LENGTH];
    long PartitionSizeInBytes;
};

struct disk_drive
{
    char DriveLabel[DRIVE_MAX_LENGTH];
    char DriveCapacity[DRIVE_MAX_LENGTH];
    int  DriveHeads;
    int  DriveSectorsPerTrack;
    int  DriveCylinders;
    long DriveSizeInBytes;
    int  DrivePartitionsCount;
    struct partition DrivePartitions[MAX_PARTITION_COUNT];
};


#endif
