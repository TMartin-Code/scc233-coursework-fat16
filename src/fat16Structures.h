#ifndef FAT16STRUCTURES_H
#define FAT16STRUCTURES_H

#include <stdint.h>

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
    uint8_t LDIR_Name1[10];    // First 5 UNICODE characters
    uint8_t LDIR_Attr;          // = ATTR_LONG_NAME (xx001111)
    uint8_t LDIR_Type;          // Should = 0
    uint8_t LDIR_Chksum;        // Checksum of short name
    uint8_t LDIR_Name2[12];   // Middle 6 UNICODE characters
    uint16_t LDIR_FstClusLO;    // MUST be zero
    uint8_t LDIR_Name3[4];    // Last 2 UNICODE characters
} LongFileNameEntry;

typedef struct __attribute__((__packed__))
{
    DirectoryStructure entry;
    char shortFN[13];
    char longFN[256];

} FileEntryNames;

#endif
