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

// fat16 FileSystem in C

// Task 2
// Metadata Struct
typedef struct __attribute__((__packed__))
{
    uint8_t BS_jmpBoot[3];      // x86 instr. to boot code
    uint8_t BS_OEMName[8];      // What created the filesystem
    uint16_t BPB_BytsPerSec;    // Bytes Per Sector
    uint8_t BPB_SecPerClus;     // Sectors per Cluster
    uint16_t BPB_RsvdSecCnt;    // Reserved Sector Count
    uint8_t BPB_NumFATs;        // Number of copies of FAT
    uint16_t BPB_RootEntCnt;    // FAT12/FAT16: Size of root DIR
    uint16_t BPB_TotSec16;      // Sectors, may be 0, see below
    uint8_t BPB_Media;          // Media type
    uint16_t BPB_FATSz16;       // Sectors in FAT (12 or 16)
    uint16_t BPB_SecPerTrk;     // Sectors per track
    uint16_t BPB_NumHeads;      // Number of heads in disk
    uint32_t BPB_HiddSec;       // Hidden Sector Count
    uint32_t BPB_TotSec32;      // Sectors if BPB_ToSec16 == 0
    uint8_t BS_DrvNum;          // 0 = Floppy, 0x80 = Hard Disk
    uint8_t BS_Reservedd1;      //
    uint8_t BS_BootSig;         // Should = 0x29
    uint32_t BS_VolID;          // unique ID for volume
    uint8_t BS_VolLab[11];      // Non Zero terminated string
    uint8_t BS_FilSysType[8];   // e.g. FAT16 (not 0 term)
} BootSector;

// Task 4
typedef struct __attribute__((__packed__))
{
    uint8_t DIR_Name[11];       // Non-zero terminated string
    uint8_t DIR_Attr;           // File Attributes
    uint8_t DIR_NTRes;          // Ignore,
    uint8_t DIR_CrtTimeTenth;   // Tenths of sec. 0.. 199
    uint16_t DIR_CrtTime;       // Creation time in 2s intervals
    uint16_t DIR_CrtDate;       // Date file created
    uint16_t DIR_LstAccDate;    // Date of last read or write
    uint16_t DIR_FstClusHI;     // Top 16 bits file's 1st cluser
    uint16_t DIR_WrtTime;       // Time of last write
    uint16_t DIR_WrtDate;       // Date of last write
    uint16_t DIR_FstClusLO;     // Lower 16 bits file's 1st cluster
    uint32_t DIR_FileSize;      // File size in bytes
} DirectoryStructure;

// Task 6
typedef struct __attribute__((__packed__))
{
    uint8_t LDIR_Ord;           // Order/ position in sequence/ set
    uint8_t LDIR_Name1[10 ];    // First 5 UNICODE characters
    uint8_t LDIR_Attr;          // = ATTR_LONG_NAME (xx001111)
    uint8_t LDIR_Type;          // Should = 0
    uint8_t LDIR_Chksum;        // Checksum of short name
    uint8_t LDIR_Name2[ 12 ];   // Middle 6 UNICODE characters
    uint16_t LDIR_FstClusLO;    // MUST be zero
    uint8_t LDIR_Name3[ 4 ];    // Last 2 UNICODE characters
} LongFileNameEntry;

char * defaultFile = "../src/fat16.img";

// Task 2 List of clusters that make up file
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

// task 1 Output bytes read
void printBytes(unsigned char * buffer, int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

int openFDR(char * filePath)
{
    int fd = open(filePath, O_RDONLY);
    if (fd < 0)
    {
        printf("Error opening File");
        return -1;
    }
    return fd;
}

// Task 1 read from fat16.img w/ offset
void readImage(int offset, char * filePath, int toRead, void * buffer)
{
    // Task 1
    // int fd = open(filePath, O_RDONLY);
    int fd = openFDR(filePath);
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

// Create a function to create an array containing
// Task 3
// // Could provide a list and print it
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

uint16_t nextCluster(uint16_t * fatBuffer, uint16_t start)
{
    uint32_t currentCluster = start;

    while (currentCluster < 0xFFF8)
    {
        return fatBuffer[currentCluster];
    }

    currentCluster = 0xFFF8;
    return currentCluster;
}

uint32_t getStartCluster(DirectoryStructure directory)
{
    return (directory.DIR_FstClusHI) << 16 | directory.DIR_FstClusLO;
}
// Task 4
void findDate(int date, int * dateArray)
{
    // Year
    dateArray[0] = ((date >> 9) & 0x7F) + 1980;
    // Month
    dateArray[1] = (date >> 5) & 0x0F;
    // Day
    dateArray[2] = (date & 0x1F);
}

// Task 4
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

// Task 4
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

// Task 4
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

void printDirectory(DirectoryStructure directory, char * fileName, char *attrFlags, int *writeDate, int * writeTime)
{
    uint32_t startCluster = getStartCluster(directory);
    printf("| %-8u | %02d:%02d:%02d | %04d/%02d/%02d | %-15s | %-10u | %-13s|\n",
        startCluster,
        writeTime[0], writeTime[1], writeTime[2],
        writeDate[0], writeDate[1], writeDate[2],
        attrFlags,
        directory.DIR_FileSize,
        fileName);
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

int lfnHandler(DirectoryStructure * rootDir, BootSector * bootSector)
{

    return 1;
}


void readRootDir(DirectoryStructure * rootDir, BootSector * bootSector)
{
    // File Names
    int valid = checkValidRootFile(rootDir->DIR_Name, rootDir->DIR_Attr);

    // Flag for whether LFN conditions are appropriate.
    int hasLFN = 0;

    if (valid == 1)
    {
        return;
    }
    else if (valid == 2)
    {
        // Handle LFN
        lfnHandler(rootDir, bootSector);
        printf("LFN\n");
        return;
    }

    // File Attributes
    char attrFlags[7] = "------\0";
    getAttributes(*rootDir, attrFlags);

    // Date format
    int createDate[3];
    int writeDate[3];

    // Create Date
    findDate(rootDir->DIR_CrtDate, createDate);
    // Write Date
    findDate(rootDir->DIR_WrtDate, writeDate);

    // Time format
    int createTime[4];
    int writeTime[3];

    // Create Time
    findTime(rootDir->DIR_CrtTime, createTime, rootDir->DIR_CrtTimeTenth);
    // Write Time
    findTime(rootDir->DIR_WrtTime, writeTime, 0);

    // Filename
    char fileName[13];
    getFileName(*rootDir, fileName);

    // Print Data
    printDirectory(*rootDir, fileName, attrFlags, writeDate, writeTime);
}


// Main Function
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
    uint32_t sizeOf = (bootSector->BPB_RootEntCnt*sizeof(DirectoryStructure));

    DirectoryStructure * rootEntries = malloc(sizeOf);

    uint32_t rootStart = (bootSector->BPB_RsvdSecCnt + (bootSector->BPB_NumFATs * bootSector->BPB_FATSz16))*bootSector->BPB_BytsPerSec;

    printf("Root Start: %d \nSize of Root: %d\n", rootStart, sizeOf);

    printf("------ Task 4 ------\n");
    readImage(rootStart, filePath, sizeOf, rootEntries);

    printf("+================================================================================+\n");
    printf("| %-8s | %-8s | %-10s | %-15s | %-10s | %-13s|\n", "Cluster", "Time", "Date", "Attributes", "Size", "File Name");
    printf("+================================================================================+\n");

    for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
    {
        readRootDir(&rootEntries[i], bootSector);
    }

    printf("+================================================================================+\n\n");

    uint32_t readLength = bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec;
    char * sectorBuffer = (void *) malloc(readLength);

    uint32_t rootDirBytes = bootSector->BPB_RootEntCnt * 32;

    uint32_t dataStartByte = (bootSector->BPB_RsvdSecCnt + (bootSector->BPB_NumFATs * bootSector->BPB_FATSz16)) * bootSector->BPB_BytsPerSec + rootDirBytes;

    // User Command Line Interface for Task 5
    while (1)
    {
        printf("Enter a command (h for help): ");
        char command[128];
        char input[128];
        int bytesToRead;
        char readFileName[13];

        fgets(input, sizeof(input), stdin);

        int cmdCount = sscanf(input, "%s %s %d", command, readFileName, &bytesToRead);

        if (cmdCount == 1 && strcmp(command, "q") == 0)
        {
            printf("Quitting System.\n");
            break;
        }
        else if (cmdCount == 1 && strcmp(command, "h") == 0)
        {
            printf("List of Commands: \n");
            printf("q - Quit System\n");
            printf("h - Help\n");
            printf("tc <int> - Trace Cluster Chain\n");
            printf("ls - List Files\n");
            printf("cat <filename> <bytes> - Read File\n");
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
            printf("List of Files: \n");
            printf("+================================================================================+\n");
            printf("| %-8s | %-8s | %-10s | %-15s | %-10s | %-13s|\n", "Cluster", "Time", "Date", "Attributes", "Size", "Name");
            printf("+================================================================================+\n");
            for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
            {
                readRootDir(&rootEntries[i], bootSector);
            }
            printf("+================================================================================+\n\n");

            continue;
        }
        else if (cmdCount == 2 && strcmp(command, "cat") == 0)
        {
            // printf("Reading %d bytes from File: %s\n", bytesToRead, readFileName);
            for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
            {
                char fileName[13];
                getFileName(rootEntries[i], fileName);

                if (strcmp(readFileName, fileName) == 0)
                {
                    if (rootEntries[i].DIR_Attr & 0x10 || rootEntries[i].DIR_Attr & 0x08)
                    {
                        printf("Directory/Volume Entered\n");
                        continue;
                    }

                    uint32_t currentCluster = getStartCluster(rootEntries[i]);
                    while (currentCluster != 0xFFFF)
                    {
                        uint32_t clusterOffsetByte = (currentCluster - 2) * bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec;

                        uint32_t finalByteOffset = dataStartByte + clusterOffsetByte;
                        uint32_t bytesRemaining = rootEntries[i].DIR_FileSize;

                        uint32_t toWrite = (bytesRemaining < readLength) ? bytesRemaining : readLength;

                        // 5. Read using the Byte Offset
                        readImage(finalByteOffset, filePath, readLength, sectorBuffer);
                        write(1,sectorBuffer, toWrite);

                        currentCluster = nextCluster(fatBuffer, currentCluster);
                    }
                }
                continue;
            }
            continue;
        }
        else if (cmdCount == 3 && strcmp(command, "cat") == 0)
        {
            // printf("Reading %d bytes from File: %s\n", bytesToRead, readFileName);
            for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
            {

                char fileName[13];
                getFileName(rootEntries[i], fileName);

                if (strcmp(readFileName, fileName) == 0)
                {
                    if (rootEntries[i].DIR_Attr & 0x10 || rootEntries[i].DIR_Attr & 0x08)
                    {
                        printf("Directory/Volume Entered\n");
                        break;
                    }
                    uint32_t currentcluster = getStartCluster(rootEntries[i]);
                    while (currentcluster != 0xFFFF)
                    {
                        uint32_t clusterOffsetByte = (currentcluster - 2) * bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec;

                        uint32_t finalByteOffset = dataStartByte + clusterOffsetByte;
                        uint32_t bytesRemaining = bytesToRead;

                        uint32_t toWrite = (bytesRemaining < readLength) ? bytesRemaining : readLength;

                        // 5. Read using the Byte Offset
                        readImage(finalByteOffset, filePath, readLength, sectorBuffer);
                        write(1,sectorBuffer, toWrite);

                        currentcluster = nextCluster(fatBuffer, currentcluster);

                    }

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

    // Freeing up memory
    free(sectorBuffer);
    free(fatBuffer);
    free(buffer);
    free(rootEntries);
    free(bootSector);
    return 0;
}
