// Header file that containes data handling functions
#ifndef  DATAHANDLING_H
#define DATAHANDLING_H

#include "fat16Structures.h"
#include <stdint.h>

void readRootDir(FileEntryNames * entry, BootSector * bootSector, char * lfnBuffer, uint8_t * checksum);
void getLFNChars(LongFileNameEntry * lFN, char * buffer);
int checkValidRootFile(uint8_t * name, uint8_t attributes);
void getFileName(DirectoryStructure directory, char *fileName);
void getAttributes(DirectoryStructure directory, char *attrFlags);
void findTime(int time, int * timeArray, int createTrue);
void findDate(int date, int * dateArray);
uint32_t getStartCluster(DirectoryStructure directory);
uint16_t nextCluster(uint16_t * fatBuffer, uint16_t start);


#endif
