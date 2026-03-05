// Header Files
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
#include <stdlib.h>

// Fat16 struct definitions
#include "fat16Structures.h"
#include "io.h"
#include "dataHandling.h"

char * defaultFile = "../src/fat16.img";

int main(int argc, char ** argv)
{
    char * filePath;
    uint offset;
    uint readLen;
    uint16_t startingClusterNum;

    // Arg 1 offest, Arg2 Bytes to read, filePath = defaultFile;
    if (argc != 5) {
        filePath = defaultFile;
    }
    else {
        filePath = argv[4];
    }
    offset = atoi(argv[1]);
    readLen = atoi(argv[2]);
    startingClusterNum = atoi(argv[3]);
    printf("Usage: Offset: %d | Read Length: %d | Starting Cluster: %d \n", offset, readLen, startingClusterNum);

    // Task 1
    unsigned char * buffer = (unsigned char *) malloc(readLen);
    readImage(offset, filePath, readLen, buffer);
    printf("------ Task 1 ------\n");
    printBytes(buffer, readLen);
    printf("\n");


    // Task 2
    BootSector * bootSector = (BootSector *) malloc(sizeof(BootSector));
    readImage(0, filePath, sizeof(BootSector), bootSector);
    printf("------ Task 2 ------\n");
    printBootSector(bootSector);
    printf("\n");


    // Task 3
    uint16_t fatStart = bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec;
    uint16_t fatSize = bootSector->BPB_FATSz16 * bootSector->BPB_BytsPerSec;

    uint16_t * fatBuffer = (uint16_t *) malloc(fatSize);

    readImage(fatStart, filePath, fatSize, fatBuffer);
    printf("------ Task 3 ------\n");
    printClusterChain(fatBuffer, startingClusterNum);
    printf("\n");


    // Task 4
    char * lfnBuffer = (char *) malloc(256);
    uint8_t checksum = 0;

    FileEntryNames entries[bootSector->BPB_RootEntCnt];

    uint32_t rootDirBytes = bootSector->BPB_RootEntCnt * 32;

    DirectoryStructure * rootEntries = malloc(rootDirBytes);

    uint32_t rootStart = (bootSector->BPB_RsvdSecCnt + (bootSector->BPB_NumFATs * bootSector->BPB_FATSz16))*bootSector->BPB_BytsPerSec;

    // printf("Root Start: %d \nSize of Root: %d\n", rootStart, rootDirBytes);

    printf("------ Task 4 ------\n");
    readImage(rootStart, filePath, rootDirBytes, rootEntries);

    printTable(&checksum, lfnBuffer, bootSector, entries, rootEntries);

    uint32_t readLength = bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec;
    char * sectorBuffer = (void *) malloc(readLength);


    uint32_t dataStartByte = (bootSector->BPB_RsvdSecCnt + (bootSector->BPB_NumFATs * bootSector->BPB_FATSz16)) * bootSector->BPB_BytsPerSec + rootDirBytes;
    // User Command Line Interface for Task 5
    while (1)
    {
        printf("Enter a command (h for help): ");
        char command[128];
        char input[256];
        int bytesToRead;
        char readFileName[13];

        // Handle User input
        fgets(input, sizeof(input), stdin);

        int cmdCount = sscanf(input, "%s %s %d", command, readFileName, &bytesToRead);

        if (cmdCount == 1 && strcmp(command, "q") == 0)
        {
            printf("Quitting System.\n");
            break;
        }
        else if (cmdCount == 1 && strcmp(command, "h") == 0)
        {
            printCommands();
            continue;
        }
        else if (cmdCount == 2 && strcmp(command, "tc") == 0)
        {
            uint16_t startCluster = atoi(readFileName);

            printf("Following Cluster Chain from %d: ", startCluster);
            printClusterChain(fatBuffer, startCluster);
            continue;
        }
        else if (cmdCount == 1 && strcmp(command, "ls") == 0)
        {
            printTable(&checksum, lfnBuffer, bootSector, entries, rootEntries);
            continue;
        }
        else if (cmdCount == 2 && (strcmp(command, "cat") == 0) && (strcmp(readFileName, "lfn") == 0))
        {
            printf("Enter Long File Name: ");
            char lfn[256];
            fgets(lfn, sizeof(lfn), stdin);

            // Removing stdin \n character as was getting 10 when strcmp
            int endLine = strcspn(lfn, "\n");
            lfn[endLine] = 0;


            for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
            {
                if (checkValidRootFile(rootEntries[i].DIR_Name, rootEntries[i].DIR_Attr) == 1)
                {
                    // printf("File Not Found\n");
                    break;
                }
                if (strcmp(lfn, entries[i].longFN) == 0)
                {
                    if (entries[i].entry.DIR_Attr & 0x10 || entries[i].entry.DIR_Attr & 0x08)
                    {
                        printf("Directory/Volume Entered\n");
                        break;
                    }
                    uint32_t currentcluster = getStartCluster(entries[i].entry);
                    uint32_t bytesRemaining = entries[i].entry.DIR_FileSize;
                    printerClusterData(bootSector, currentcluster, bytesRemaining, dataStartByte,  readLength, sectorBuffer, filePath, fatBuffer);
                }
            }
        }
        else if (cmdCount == 2 && strcmp(command, "cat") == 0)
        {
            for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
            {
                if (checkValidRootFile(rootEntries[i].DIR_Name, rootEntries[i].DIR_Attr) == 1)
                {
                    // printf("File Not Found\n");
                    break;
                }
                if (strcmp(readFileName, entries[i].shortFN) == 0 || strcmp(readFileName, entries[i].longFN) == 0)
                {
                    if (rootEntries[i].DIR_Attr & 0x10 || rootEntries[i].DIR_Attr & 0x08)
                    {
                        printf("Directory/Volume Entered (Did you mean to use cd?)\n");
                        break;
                    }

                    uint32_t currentCluster = getStartCluster(rootEntries[i]);
                    uint32_t bytesRemaining = rootEntries[i].DIR_FileSize;
                    printerClusterData(bootSector, currentCluster, bytesRemaining, dataStartByte,  readLength, sectorBuffer, filePath, fatBuffer);
                }
                continue;
            }
            continue;
        }
        else if (cmdCount == 3 && strcmp(command, "cat") == 0)
        {
            for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
            {
                if (checkValidRootFile(rootEntries[i].DIR_Name, rootEntries[i].DIR_Attr) == 1)
                {
                    // printf("File Not Found\n");
                    break;
                }

                if (strcmp(readFileName, entries[i].shortFN) == 0 || strcmp(readFileName, entries[i].longFN) == 0)
                {
                    if (rootEntries[i].DIR_Attr & 0x10 || rootEntries[i].DIR_Attr & 0x08)
                    {
                        printf("Directory/Volume Entered (Did you mean to use cd?)\n");
                        break;
                    }
                    uint32_t currentcluster = getStartCluster(rootEntries[i]);
                    uint32_t bytesRemaining = bytesToRead;
                    printerClusterData(bootSector, currentcluster, bytesRemaining, dataStartByte,  readLength, sectorBuffer, filePath, fatBuffer);

                    printf("\n");

                }
                continue;
            }
            continue;
        }
        else
        {
            printf("Invalid command.\n");
            continue;
        }
    }

    free(sectorBuffer);
    free(fatBuffer);
    free(buffer);
    free(rootEntries);
    free(bootSector);
    free(lfnBuffer);

    return 0;
}
