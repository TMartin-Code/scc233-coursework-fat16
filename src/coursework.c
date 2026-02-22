// Header Files
#include <linux/limits.h>
#include <setjmp.h>
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
    uint16_t DIR_FstClisHI;     // Top 16 bits file's 1st cluser
    uint16_t DIR_WrtTime;       // Time of last write
    uint16_t DIR_WrtDate;       // Date of last write
    uint16_t DIR_FstClusLO;     // Lower 16 bits file's 1st cluster
    uint32_t DIR_FileSize;      // File size in bytes
} DirectoryStructure;



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

// Task 1 read from fat16.img w/ offset
void readImage(int offset, char * filePath, int toRead, void * buffer)
{
    // Task 1
    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        perror("Error Opening to Read\n");
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

// Task 3
void printfat(uint16_t * fatBuffer, uint16_t start)
{
    uint32_t currentCluster = start;

    while (currentCluster < 0xFFF8)
    {
        printf("0x%04X -> ", currentCluster);
        currentCluster = fatBuffer[currentCluster];
    }

    printf("EOF\n");
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
    if (directory.DIR_Attr & 0x20) {attrFlags[0] = 'A';}
    if (directory.DIR_Attr & 0x10) {attrFlags[1] = 'D';}
    if (directory.DIR_Attr & 0x08) {attrFlags[2] = 'V';}
    if (directory.DIR_Attr & 0x04) {attrFlags[3] = 'S';}
    if (directory.DIR_Attr & 0x02) {attrFlags[4] = 'H';}
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
    uint32_t startCluster = (directory.DIR_FstClisHI << 16) | directory.DIR_FstClusLO;
    printf("| %-8u | %02d:%02d:%02d | %04d/%02d/%02d | %-15s | %-10u | %-13s|\n",
        startCluster,
        writeTime[0], writeTime[1], writeTime[2],
        writeDate[0], writeDate[1], writeDate[2],
        attrFlags,
        directory.DIR_FileSize,
        fileName);
}


// void openFile(Volume *, ShortDirEntry *);
// void seekFile(File *, off_t offset, inr whence);
// void readFile(File *, void * buffer, size_t length);
// void closeFile(File *);




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
    printfat(fatBuffer, startingClusterNum);
    printf("\n");

    // Task 4
    DirectoryStructure * rootDir = (DirectoryStructure *) malloc(sizeof(DirectoryStructure));
    uint32_t sizeOf = (bootSector->BPB_RootEntCnt*sizeof(DirectoryStructure));

    DirectoryStructure rootEntries[bootSector->BPB_RootEntCnt];
    printf("rootEntries: %d\n", sizeof(rootEntries));
    printf("Directorystructure: %d\n", sizeof(DirectoryStructure));
    uint32_t rootStart = (bootSector->BPB_RsvdSecCnt + (bootSector->BPB_NumFATs * bootSector->BPB_FATSz16))*bootSector->BPB_BytsPerSec;
    printf("sizeOf: %d\n", sizeOf);

    printf("------ Task 4 ------\n");
    printf("==================================================================================\n");
    printf("| %-8s | %-8s | %-10s | %-15s | %-10s | %-13s|\n", "Cluster", "Time", "Date", "Attributes", "Size", "Name");
    printf("==================================================================================\n");
    int fd = open(filePath, O_RDONLY);

    if (fd < 0) {
        perror("open");
        return 0;
    }

    int rootEntry = 0;
    lseek(fd, rootStart, SEEK_SET);
    for (int i = 0; i < bootSector->BPB_RootEntCnt; i++)
    {

        if(read(fd, rootDir, sizeof(DirectoryStructure))<0)
        {
            perror("Error reading from file\n");
            return 0;
        }

        // File Names
        if (rootDir->DIR_Name[0] == 0x00)
        {
            break;
        }
        if (rootDir->DIR_Name[0] == 0xE5 || rootDir->DIR_Attr == 0x0F)
        {
            continue;
        }
        rootEntries[rootEntry++] = *rootDir;


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
        printDirectory(*rootDir, fileName, attrFlags, writeDate, writeTime);

    }
    close(fd);
    printf("==================================================================================\n\n");

    for (int i = 0; i < rootEntry; i++)
    {
        uint16_t start = (rootEntries[i].DIR_FstClisHI << 16) | rootEntries[i].DIR_FstClusLO;
        if (rootEntries[i].DIR_Attr & 0x10) {
            printf("%s/", rootEntries[i].DIR_Name);

        }
        else {
            printf("%s\n", rootEntries[i].DIR_Name);
        }
        printfat(fatBuffer,start);
    }
    free(fatBuffer);
    free(buffer);
    free(rootDir);
    free(bootSector);
    return 0;
}
