// Header file that contains all IO functions
#ifndef IO_H
#define IO_H

#include "fat16Structures.h"
#include <stdint.h>

/**
 * @brief Reads from given filepath, given an offset and read length, into a buffer.
 ** @param offset - The integer to offset where the file is read from.
 * @param filePath - The String containing the file path to the file you want to read from.
 * @param toRead - The number of bytes, as an Integer to read from the file.
 * @param buffer - The buffer to store data from file, updated via Pointer.
 */
void readImage(int offset, char * filePath, int toRead, void * buffer);

/**
 * @brief Outputs the individual directory entry information
 * @param directory - The directory structure entry to output information from.
 * @param fileName - A string containing current entrys short file name
 * @param attrFlags - A string containing current entrys formatted attribute flags
 * @param writeDate - Pointer to an int containing current entrys most recent write date
 * @param writeTime - Pointer to an int containing current entrys most recent write time
 * @param longFileName - A string containing current entrys long file name
 */
void printDirectory(DirectoryStructure directory, char * fileName, char *attrFlags, int *writeDate, int * writeTime, char * longFileName);

/**
 * @brief Outputs the list of entries in the directory, updates short and long file name per entry.
 * @param checksum - Pointer to uint8_t checksum value used to check long file name entries.
 * @param lfnBuffer - Pointer to char buffer that stores current long file name.
 * @param bootSector - Pointer to buffer where bootSector data is stored.
 * @param entries - Pointer to list of FileEntryName structs that track entries in directory.
 * @param rootEntries - Pointer to buffer where root directory data is stored.
 */
void printTable(uint8_t * checksum, char * lfnBuffer, BootSector * bootSector, FileEntryNames * entries, DirectoryStructure * rootEntries);

/**
 * @brief Outputs a formatted list of commands for the user interface
 */
void printCommands();

/**
 * @brief Will follow the cluster chain from the provided start cluster to EOF, and output each cluster visited
 * @param fatBuffer - Pointer to FAT entry to read clusters from.
 * @param start - Starting cluster number
 */
void printClusterChain(uint16_t * fatBuffer, uint16_t start);

/**
 * @brief Will output a length of bytes from the provided buffer.
 * @param buffer - The buffer to retrieve bytes from.
 * @param length - An int that provides how many bytes to read.
 */
void printBytes(unsigned char * buffer, int length);

/**
 * @brief Will output the data in the boot sector buffer
 * @param bootSector - A buffer containing the boot sector information.
 */
void printBootSector(BootSector * bootSector);

/**
 * @brief Will output the data stored in the cluster chain of the given starting cluster.
 * @param bootSector - Pointer to buffer containing boot sector informaion
 * @param startCluster - uint32_t value of the cluster to start at
 * @param byteRemaining - uint32_t value stating how many bytes are left to read
 * @param dataStartbyte - uint32_t value of where data starts in the fat image
 * @param readlength - uint32_t value stating starting value of bytes to read
 * @param sectorBuffer - Pointer to buffer to read data into
 * @param fatBuffer - Pointer to FAT entry containing cluster chains
 * @param filePath - Pointer to a string that contains file to read from
 */
void printerClusterData(BootSector * bootSector, uint32_t startCluster, uint32_t bytesRemaining, uint32_t dataStartByte, uint32_t readLength, void * sectorBuffer,  char * filePath, void * fatBuffer);


#endif
