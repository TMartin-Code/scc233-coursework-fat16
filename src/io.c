#include "io.h"
#include <stdio.h>
#include <endian.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/inotify.h>
#include <limits.h>
#include <string.h>
#include "dataHandling.h"



void readImage(int offset, char * filePath, int toRead, void * buffer)
{
    // Task 1
    // int fd = open(filePath, O_RDONLY);
    int fd = open(filePath, O_RDONLY);
    if (fd < 0)
    {
        printf("Error opening File");
        return;
    }
    if (fd == -1)
    {
        return;
    }
    lseek(fd, offset, SEEK_SET);

    if (read(fd, buffer, toRead) < 0)
    {
        perror("Error reading from file\n");
        return;
    }

    close(fd);
    return;
}

void printClusterChain(uint16_t * fatBuffer, uint16_t start)
{
    uint32_t currentCluster = start;

    while (currentCluster < 0xFFF8)
    {
        printf("0x%02X -> ", currentCluster);
        currentCluster = fatBuffer[currentCluster];
    }

    printf("EOF\n");
}

void printBootSector(BootSector * bootSector)
{
    printf("------Boot Sector Information------\n");
    printf("BS_VolLab:      %.11s\n", bootSector->BS_VolLab);
    printf("BPB_BytsPerSec: 0x%04x \n", bootSector->BPB_BytsPerSec); // This * RsvdSecCnt = offset for first FAT
    printf("BPB_SecPerClus: 0x%02x \n", bootSector->BPB_SecPerClus);
    printf("BPB_RsvdSecCnt: 0x%02x \n", bootSector->BPB_RsvdSecCnt);// Num reserved Sectors
    printf("BPB_NumFATs:    0x%02x \n", bootSector->BPB_NumFATs);
    printf("BPB_RootEntCnt: %03u \n", bootSector->BPB_RootEntCnt);
    printf("BPB_TotSec16:   %05u \n", bootSector->BPB_TotSec16);
    printf("BPB_FATSz16:    %02u \n", bootSector->BPB_FATSz16); // Size of each FAT in FS
    printf("BPB_TotSec32:   %02u \n", bootSector->BPB_TotSec32);
}

void printBytes(unsigned char * buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

void printDirectory(DirectoryStructure directory, char * fileName, char *attrFlags, int *writeDate, int * writeTime, char * longFileName)
{
    uint32_t startCluster = getStartCluster(directory);
    printf("| %-8u | %02d:%02d:%02d | %04d/%02d/%02d | %-15s | %-10u | %-36s|\n",
        startCluster,
        writeTime[0], writeTime[1], writeTime[2],
        writeDate[0], writeDate[1], writeDate[2],
        attrFlags,
        directory.DIR_FileSize,
        fileName);
    if(longFileName[0] != '\0')
    {
        // int count = 0;
        printf("|- ");
        printf("%-100s", longFileName);
        printf(" |\n");

    }
}

void printTable(uint8_t * checksum, char * lfnBuffer, BootSector * bootSector, FileEntryNames * entries, DirectoryStructure * rootEntries)
{
    printf("+=======================================================================================================+\n");
    printf("| %-8s | %-8s | %-10s | %-15s | %-10s | %-36s|\n", "Cluster", "Time", "Date", "Attributes", "Size", "File Name");
    printf("+=======================================================================================================+\n");

    memset(lfnBuffer, 0, 256);
    for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
    {
        entries[i].entry = rootEntries[i];
        readRootDir(&entries[i], lfnBuffer, checksum);

    }
    printf("+=======================================================================================================+\n");

    return;
}

void printCommands()
{
    printf("+=======================================================================================================+\n");
    printf("| %-101s |\n","List of Commands:");
    printf("+=======================================================================================================+\n");
    printf("| q - %-97s |\n", "Quit System");
    printf("| h - %-97s |\n","Help");
    printf("| tc <int> - %-90s |\n","Trace Cluster Chain");
    printf("| ls - %-96s |\n","List Files");
    printf("| cat <filename> <bytes> - %-76s |\n","Read File (If bytes empty, will read whole file)");
    printf("| cat lfn - %-91s |\n","Read From Long File Names With Spaces");
    printf("+=======================================================================================================+\n");

    return;
}

void printerClusterData(BootSector * bootSector, uint32_t startCluster, uint32_t bytesRemaining, uint32_t dataStartByte, uint32_t readLength, void * sectorBuffer,  char * filePath, void * fatBuffer)
{
    uint32_t currentcluster = startCluster;

    while (currentcluster != 0xFFFF)
    {
        uint32_t clusterOffsetByte = (currentcluster - 2) * bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec;

        uint32_t finalByteOffset = dataStartByte + clusterOffsetByte;
        uint32_t toWrite;
        if (bytesRemaining < readLength)
        {
            toWrite = bytesRemaining;
        }
        else
        {
            toWrite = readLength;
        }

        readImage(finalByteOffset, filePath, readLength, sectorBuffer);
        write(1,sectorBuffer, toWrite);


        bytesRemaining -= toWrite;

        currentcluster = nextCluster(fatBuffer, currentcluster);
    }
}
