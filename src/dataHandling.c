#include "dataHandling.h"
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


uint16_t nextCluster(uint16_t * fatBuffer, uint16_t start)
{
    uint32_t currentCluster = start;

    if (currentCluster < 0xFFF8)
    {
        return fatBuffer[currentCluster];
    }

    currentCluster = 0xFFF8;
    return currentCluster;
}

void findDate(int date, int * dateArray)
{
    // Year
    dateArray[0] = ((date >> 9) & 0x7F) + 1980;
    // Month
    dateArray[1] = (date >> 5) & 0x0F;
    // Day
    dateArray[2] = (date & 0x1F);
}

uint32_t getStartCluster(DirectoryStructure directory)
{
    return (directory.DIR_FstClusHI) << 16 | directory.DIR_FstClusLO;
}

void findTime(int time, int * timeArray, int createTrue)
{
    // Hour
    timeArray[0] = ((time >> 11) & 0x1F);
    // Minute
    timeArray[1] = (time >> 5) & 0x3F;
    // Second
    timeArray[2] = (time & 0x1F) * 2;
    // Tenths of Second
    if (createTrue)
    {
        timeArray[2] = timeArray[2] + (createTrue/100);
        timeArray[3] = (createTrue % 100) * 10;
    }
}

void getAttributes(DirectoryStructure directory, char *attrFlags)
{
    // Archive
    if (directory.DIR_Attr & 0x20) {attrFlags[0] = 'A';}
    // Directory
    if (directory.DIR_Attr & 0x10) {attrFlags[1] = 'D';}
    // Volume
    if (directory.DIR_Attr & 0x08) {attrFlags[2] = 'V';}
    // System
    if (directory.DIR_Attr & 0x04) {attrFlags[3] = 'S';}
    // Hidden
    if (directory.DIR_Attr & 0x02) {attrFlags[4] = 'H';}
    // Read Only
    if (directory.DIR_Attr & 0x01) {attrFlags[5] = 'R';}
}

void getFileName(DirectoryStructure directory, char *fileName)
{
    int count = 0;
    for (int i = 0; i < 8; i++)
    {
        if(directory.DIR_Name[i] != 0x20)
        {
        fileName[count++] = directory.DIR_Name[i];
        }
    }
    if (directory.DIR_Name[8] != 0x20)
    {
        fileName[count++] = '.';
        for (int i = 8; i < 11; i++)
        {
            if (directory.DIR_Name[i] != 0x20)
            {
                fileName[count++] = directory.DIR_Name[i];
            }
        }
    }
    fileName[count] = '\0';
}
int checkValidRootFile(uint8_t * name, uint8_t attributes)
{
    if (name[0] == 0x00 || *name == 0xE5)
    {
        return 1;
    }

    // 0x0F checks if LFN
    if (attributes == 0x0F)
    {
        return 2;
    }

    return 0;
}

void getLFNChars(LongFileNameEntry * lFN, char * buffer)
{
    int order = lFN->LDIR_Ord & 0x1F;
    int index = (order-1) * 13;
    int count = 0;
    char temp[256];

    for (int i = 0; i < 10; i+=2)
    {
        if (lFN->LDIR_Name1[i] == 0x00)
        {
            goto writeToBuffer;
        }
        temp[count++] = lFN->LDIR_Name1[i];
    }
    for (int i = 0; i < 12; i+=2)
    {
        if (lFN->LDIR_Name2[i] == 0x00)
        {
            goto writeToBuffer;
        }
        temp[count++] = lFN->LDIR_Name2[i];
    }
    for (int i = 0; i < 4; i+=2)
    {
        if (lFN->LDIR_Name3[i] == 0x00)
        {
            goto writeToBuffer;
        }
        temp[count++] = lFN->LDIR_Name3[i];
    }

    writeToBuffer:
    temp[count] = '\0';
    memcpy(buffer + index, temp, count);
    return;
}

void readRootDir(FileEntryNames * entry, char * lfnBuffer, uint8_t * checksum)
{
    DirectoryStructure * rootDir = &entry->entry;

    int valid = checkValidRootFile(rootDir->DIR_Name, rootDir->DIR_Attr);

    if (valid == 1)
    {
        return;
    }
    if (valid == 2)
    {
        LongFileNameEntry * longFileName = (LongFileNameEntry *) &entry->entry;
        if (longFileName->LDIR_Ord & 0x40)
        {
            memset(lfnBuffer, 0, 256);
            *checksum = longFileName->LDIR_Chksum;
        }
        else if(longFileName->LDIR_Chksum != *checksum)
        {
            printf("Checksum Error\n");
            memset(lfnBuffer, 0, 256);
            return;
        }
        getLFNChars(longFileName, lfnBuffer);
        return;
    }

    char attrFlags[7] = "------";
    getAttributes(*rootDir, attrFlags);

    int createDate[3];
    int writeDate[3];

    findDate(rootDir->DIR_CrtDate, createDate);
    findDate(rootDir->DIR_WrtDate, writeDate);

    int createTime[4];
    int writeTime[3];

    findTime(rootDir->DIR_CrtTime, createTime, rootDir->DIR_CrtTimeTenth);
    findTime(rootDir->DIR_WrtTime, writeTime, 0);

    getFileName(*rootDir, entry->shortFN);
    strcpy(entry->longFN, lfnBuffer);

    printDirectory(*rootDir, entry->shortFN, attrFlags, writeDate, writeTime, entry->longFN);

}
